/****************************************************************
 * Cosa fa la main:                                             *
 * 1) crea un vettore con le string dei file della cartella     *
 * 2) apre un cilclo sulle altezze scansionate                  *
 * 3) apre un cilco sull'angolo (ad aletezza fissata)           *
 * 4) trova il file corrispondente all'altezza e a r            *
 * 5) apre il file e ne crea un istogramma                      *
 * 6) calcola il segnale                                        *
 * 7) crea l'sitogramma 2D (h,r)                                *
 * 8) scrive un file root contente l'isto2D                     *
 * 9) scrive un file con tutti gli isto1D e fit                 *
 * 10)scrive un file con xpos, rad e segnale                    *
 *                                                              *
 * NOTA: MODIFICARE l'histo2D e i parametri del fit             *
 * **************************************************************/

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <dirent.h>

#include "scan_functions.h"                                                     //qui sono definite le funzioni

#include "TSpectrum.h"
#include "TNtuple.h"
#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TH1D.h"
#include "TF1.h"
#include "TH2D.h"
#include "TLine.h"
using namespace std;



int main ( int argc, char* argv[] ) {
    
//----  PARAMETRI DI SCANSIONE --------------------------------------------------------------------------------------------------
    
    int r_min=0, r_max=0, h_min=0, h_max=0, r_step=0, h_step=0;
    cout <<"Inserire rmin: ";
    cin>>r_min;
    cout<<"Inserire rmax: ";
    cin>>r_max;
    cout<<"Inserie i r step: ";
    cin>>r_step;
    cout<<"Inserie h min [0.1*mm]: ";
    cin>>h_min;
    cout<<"Inserie h max [0.1*mm]: ";
    cin>>h_max;
    cout<<"Inserie h step: ";
    cin>>h_step;
    
//---------------------------------------------------------------------------------------------------------------------------------        
    
    int dr = (r_max - r_min)/r_step;                                                            //inizializzo l'isto2d 
    int dh = (h_max - h_min)/h_step;                                                            //LE H SONO NEGATIVE COSI' DA RISPETTARE
    TH2D* top_surface = new TH2D("top", "top surface",dr+1,r_min,r_max+r_step,dh+1,-h_max,-h_min+r_step);         //LA POSIZIONE FISICA DEL RIVELATORE NEL PLOT (VEDERE IL FILL DI TH2D)
    //IL CENTRO DEL RIVELATORE E' h_max, MENTRE IL BORDO E' h_min
    
//---- INPUT E OUTPUT PATH ---------------------------------------------------------------------------------------------------------
    
    string PATH_IN;									    //path dei file da analizzare
    string PATH_OUT;									    //path dei file da scrivere

    if( argc != 3 )									    //la main prende come argomento il path della cartella dove sono i dati
    {
	cout<< "not enough arguments"<<endl;						    //se non do i file allora faccio eseguire il programma nella directory
        PATH_IN = ".";
        PATH_OUT = ".";
    }
    
    else {

    PATH_IN = argv[1];								    //PATH è il path inserito da terminale
    PATH_OUT = argv[2];

    }

//-----------------------------------------------------------------------------------------------------------------------------------

    vector<string> files = make_data_name(PATH_IN);                                          //creo il vector con i file
    
//---- OPEN OUTPUT FILES -------------------------------------------------------------------------------------------------------------    
    string outname = PATH_OUT + "/";
    outname += "histo1D_top.root";
    TFile * out_h1d = new TFile(outname.c_str(), "RECREATE");                        //creo il file con gli isto1d eliminando una versione prec
    
    string txtname = PATH_OUT + "/top_signal.txt";
    ofstream out_signal(txtname.c_str());
    out_signal<<"#XPOS   RAD ----> SIGNAL\n\n";
    
//---- CICLO SULLE XPOS E RAD --------------------------------------------------------------------------------------------------------    
    
    for(int h=h_min; h<=h_max; h+=h_step) {
        
        for(int r=r_min; r<=r_max; r+=r_step) {
            
            int flag=0;                                                                     //flag controlla se nel vector c'è il file a h e r corrispondente
            
//---- LETTURA DEI FILE ALLA XPOS E RAD CORRISPONDENTE AL CICLO -----------------------------------------------------------------------------
            
            string h_str = to_string(h);                                                    //trasformo le h da int in string
            //if(h<)
            
            string r_str;                                                                   //trasformo le r da int in string
            if(r<10) r_str = "000";                                                         //condizioni di inizializzazione
            else if (r>=10 && r<100) r_str = "0"+to_string(r);
            else if (r>=100) r_str = to_string(r);

           
            for (string data_name : files) {
    
                size_t posx = data_name.find(h_str);                                        //trovo la posizione del xops nella stringa
                size_t posr = data_name.find(r_str, 20);                                        //gli dico di partire da 20 (Ex: x=3050---r=050)
                                                                                            //ambiguità con le x
                
                if ( (posx>100) || (posr !=26) ) continue;                                  //seleziona i file con alla posizione e angolo corrispondenti al ciclo              

               /*****************************************************************
                *  quando nel file non trova la sub string r_str posr è         *
                *  un numero molto alto oppure diverso da 26                    * 
                *  cout<<data_name<<endl;                                       *
                *  cout<<h<<"--"<<r<<"---->"<<posx<<"******"<<posr<<endl<<endl; *
                *****************************************************************/
                
                flag=1;                                                                     //poichè il flusso non si è interrotto il file è stato trovato
                                                                                            //quindi flag assume valore 1
               
//---- ANALISI DATI -------------------------------------------------------------------------------------------------------------------------

		string filename = PATH_IN; filename += "/";						  //aggiungo il path al dataname
		filename += data_name;

                TH1D* histo=make_histo(filename,to_string(h),to_string(r),0,0);                            //creo l'istogramma del file .Spe
            
                double segnale = comp_signal(histo, out_h1d);                                               //computo il segnale
            
                top_surface -> Fill(r,-h,segnale);                                                         //riempio l'istogramma 2D
                //PER LE H NEGATIVE VEDERE LE PRIME RIGHE DOVE INIZIALIZZO top_surface
                
//-------------------------------------------------------------------------------------------------------------------------------------------
        
                out_signal<<" "<<h<<"   "<<r<<" ----> " <<segnale<<endl;                                         //scrivo i segnali nel file txt
                
            }                                                                                               //chiudo il ciclo sui files
        
        if (flag==0) cout<<"File a -----> xpos=" <<h <<" e r=" <<r <<" <-----NON TROVATO!"<<endl<<endl;
            
            
        }                                                                                                   //chiudo il ciclo sui gradi
    
    out_signal<<endl;                                                                                   //divido le varie h nel file.txt
    
    }                                                                                                       //chiudo il ciclo sulle altezze

    
    out_h1d -> Close();                                                                                     //chiudo il dile histo.h
    cout<<"File 'histo1D_top.root' creato!\n\n";
    
    out_signal.close();                                                                                     //chiudo il file txt con i segnali
    cout<<"File 'top_signal.txt' creato!\n\n";
    
    //FILE CON ISTO2D
    string h2d_name = PATH_OUT + "/histo2D_top.root";
    TFile* out_h2d = new TFile(h2d_name.c_str(), "RECREATE");                                         //crea il file root con TH2D-
    
    TCanvas* c = new TCanvas();

    top_surface -> GetXaxis()->SetTitle("Angle [Degree]");
    top_surface -> GetYaxis()->SetTitle("Position [0.1*mm]");
    top_surface -> GetZaxis()->SetTitle("Signal");
    top_surface -> Draw("SURF1 POL");  

    c -> Write();
    out_h2d -> Close();
    
    cout<<"File 'histo2D_top.root'  creato!"<<endl;

    
    return 0;
}




