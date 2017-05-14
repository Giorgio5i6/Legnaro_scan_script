#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <dirent.h>

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




vector<string> make_data_name (string path);                                         //crea un vector con i file

TH1D* make_histo (string fileSpe, string Xpos, string Rad, int bin_min, int bin_max);       //crea un histo con lo spettro,

void write_histo1D_fit (TH1D* h1d, TF1* fit_func);                                            //scrive su file il plot dell'isto e il fit

double comp_signal (TH1D* h, TFile* f1d);                                                   //calcola il segnale

//---- MAKE_DATA_NAME_1 ---------------------------------------------------------------------------------------------------------

/********************************************************************
 * LA FUNZIONE RITORNA IL VETTORE CONTENETE I FILE NELLA CARTELLA.  *
 * PRENDE COME ARGOMENTO IL PATH DELLA CARTELLA.                    *
 * ******************************************************************/

vector<string> make_data_name (string path) {
    
    vector<string> fileList_temp;                                                           //vettore contenente i file
    
    DIR* d;                                                                                 //oggetto DIR che apre la cartella
    d=opendir(path.c_str());
    
    struct dirent *dir;                                                                     //oggetto struct dirent che la legge dalla cartella
    dir=readdir(d);
    
    if(!d) cout<<"DIRECTORY NOT FOUND!"<<endl;
    
    else {
        
        while ((dir = readdir(d)) != NULL)    fileList_temp.push_back(dir->d_name);         //d_name è la stringa contenenuta nella struct
        
    }
    
    closedir(d);
    
    return fileList_temp;
    
}

//---- MAKE_HISTO ----------------------------------------------------------------------------------------------------------------------

/**************************************************************************
 * LA FUNZIONE CREA L'ISTOGRAMMA DAL FILE .SPE                            *
 * SE VENGONO FORNITI bin_max/min LA FUNZIONE CREA UN ISTOGRAMMA TAGLIATO *
 * PRENDE COME ARGOMENTO IL FILE, LA POSIZIONE E RAD, bin_max/bin_min     *
 * ************************************************************************/

TH1D* make_histo (string fileSpe, string Xpos, string Rad, int bin_min = 0, int bin_max = 0) {
    
    string line;
    const int maxbins=4096*4;
    float *binc = new float[maxbins];
    int nbins=0;

    ifstream infile (fileSpe, ios::in);                                     //apro il file
            
            if (!infile.is_open()) {                                         //se non trova il file continua ad aprire gli altri
                
                cout<<endl<<endl;
                cout << "file "  << fileSpe <<" not found! " <<endl;
                return NULL;
    
            }
            
    while ( getline(infile,line) ) {                                        //Legge le righe del file, salta i commenti e mette i rate nel vettore binc
        
        if (line[0] == '#') continue;                                       // skip comments
        
        binc[nbins++] = atof(line.c_str());                                 // legge 1 float dalla riga
    
    }
    
    
    string histotitle="Am241_xpos"+Xpos+"_r"+Rad;
    
    
    if (bin_min==0 || bin_max==0) {                                                         //SE HA I PARAMETRI DI DEFAULT CREA TUTTO L'ISTOGRAMMA
        
        TH1D* h = new TH1D(fileSpe.c_str(), histotitle.c_str(), nbins, 0, nbins);           //c_str() trasforma una stringa in una C-string (char* mystring)
    
        for (int i=1; i<=nbins; i++)  h->SetBinContent(i,binc[i-1]);
    
        h->SetEntries(h->Integral());
        
        return h;
        
    } else {                                                                                 //SE VENGONO FORNITI IL MIN E MAX CREA L'ISTO TAGLAIATO  
        
        int sub_nbins = bin_max-bin_min;
        
        TH1D* h = new TH1D(fileSpe.c_str(), histotitle.c_str(), sub_nbins , bin_min, bin_max);   //c_str() trasforma una stringa in una C-string (char* mystring)
    
        for (int i=1; i<=sub_nbins; i++)  h->SetBinContent(i,binc[i+bin_min-1]);
        
        h->SetEntries(h->Integral());
        
        return h;
    }
        
    
}

//---- WRITE_HISTO1D_FIT -------------------------------------------------------------------------------------------------------------------------------

void write_histo1D_fit (TH1D* h1d, TF1* fit_func) {

    //setto i nomi dei parametri della funzione fit_func----->NON FUNZIONA
    /*fit_func -> SetParName(0,"Amp_gauss");
    fit_func -> SetParName(1,"Mean");
    fit_func -> SetParName(2,"Sigma");
    fit_func -> SetParName(3,"Background");
    fit_func -> SetParName(4,"Amp_erfc");*/

    string cname = "c"; cname += h1d->GetName();                            //nome degli histo nella canvas
    
    double m = fit_func->GetParameter(1);                                   //dalla funzione fit mi ricavo i parametri mean e sigma del fit
    double s = fit_func->GetParameter(2);
    
    TCanvas * c = new TCanvas();                                            //creo la canvas su cui disegnare l'histo e i fit
    
    h1d -> Draw("histo");                                                   //disegno l'isto sulla canvas
    h1d -> GetXaxis() -> SetRangeUser(m - 10*s,m + 10*s);
    
    fit_func -> SetLineColor(kRed);
    fit_func -> Draw("same");                                               //disegno il fit nella canvas con l'istogramma
    
    TLine * line1 = new TLine( m - 9*s, 0, m - 9*s, 70 );                 //linee per vedere le zone dove ho calcolato il segnale
    TLine * line2 = new TLine( m - 3*s, 0, m - 3*s, 70 );
    TLine * line3 = new TLine( m + 3*s, 0, m + 3*s, 70 );
    TLine * line4 = new TLine( m + 9*s, 0, m + 9*s, 70 );

    line1->Draw("same");                                                    //disegno le linee nel plot dell'isto
    line2->Draw("same");
    line3->Draw("same");
    line4->Draw("same");

    //PLOTTO LA ERROR FUNCTION SEPARATAMENTE
    TF1* erf = new TF1("erf","([2]-[3])/2.*ROOT::Math::erfc( ([0]-x)/(sqrt(2.)*[1]) )",1120.,1185.);
    erf -> SetParameter(3, fit_func -> GetParameter(3));
    erf -> SetParameter(2, fit_func -> GetParameter(4));
    erf -> SetParameter(1, fit_func -> GetParameter(1));
    erf -> SetParameter(0, fit_func -> GetParameter(2));
    erf -> SetLineColor(kGreen);
    erf -> Draw("same");

    c -> Write(cname.c_str());                                              //scrivo tutto nel file

}

//---- COMP_SIGNAL ------------------------------------------------------------------------------------------------------------

/************************************************************************************************************
 * IL SEGNALE È' STATO CALCOLATO FACENDO L'INTEGRALE DELL'ISTOGRAMMA A +-3SIGMA DAL PICCO.                  *
 * E' STATO SOTTRATTA LA MEDIA DEGLI INTEGRALI DELLE CODE PRIMA E DOPO IL PICCO COME STIMA DEL BACKGROUND.  *
 * PER TUTTI GLI INTEGRALI HO USATO nbins=6*sigma.                                                          *
 *													    *
 * NOTA: LA FUNZIONE Integral() PRENDE I BIN COME ARGOMENTO, QUI È GIUSTO PERCHÉ I BIN E LE X COINCIDONO    *
 *                                                                                                          *
 *													    *
 * NOTA ERROR FUNCTION (erfc):										    *
 * 	la scelta dei parametri segue la seguente logica:						    *
 *      [4]=altezza dello step       									    *
 *      [1]=è stato posto lo stesso parametro di mean poichè inidica la traslazione rispetto all'origine    *
 *	[2]=è stato posto lo stesso parametro di sigma poichè indica la larghezza dello step che coincide   *
 *          con la larghezza del picco                                                                      *
 ************************************************************************************************************/

double comp_signal (TH1D* h, TFile* f1d) {
    
    //FACCIO RICAVARE LA SIGMA E MEAN DAL FIT DEI PICCHI
    
    TF1* fit = new TF1 ("fitgaus", "gaus(0) + pol0(3) + ([4]-[3])/2.*ROOT::Math::erfc( ([1]-x)/(sqrt(2.)*[2]) )", 1100., 1200.);             //fitto i picchi
    
    fit->SetParameters(100.,1160.,3.5,10.);                                       //i parametri sono settati secondo un file di rifermiento (xpos2810,r000)
    fit->SetParLimits(1,1140.,1170.);                                               //setto il range dei parametri
    fit->SetParLimits(2,1.,5.);
    fit->SetParLimits(4,0.,40.);
    
    h->Fit("fitgaus", "R" "Q");                                                     //"R"--->fitta nel range, "Q"---> non stampa i parametri del fit su schermo
   
    double k= fit->GetParameter(0);                                                 //prendo i parametri del fit
    double mean= fit->GetParameter(1);
    double sigma= fit->GetParameter(2);
    
   
    double peack = 0., backg1 = 0., backg2 = 0., signal = 0.;
    
    peack = h -> Integral(mean-(3*sigma), mean+(3*sigma));                  //calcolo l'integrale del picco a +- 3 sigma dal picco
    backg1 = h-> Integral(mean-(9*sigma),mean-(3*sigma));                   //calcolo il background prima del segnale
    backg2 = h-> Integral(mean+(3*sigma),mean+(9*sigma));                   //calcolo il background dopo il segnale
    
    signal = peack - (0.5*(backg1+backg2));                                 //calcolo il segnale sottrando il picco alla media del fondo

    write_histo1D_fit(h, fit);
    
    return signal;
    
    
     //FORNISCO LA SIGMA E FACCIO TROVARE IL PICCO CON TSpectrum<--------DA RIVEDERE NON FUNZIONA BENE
  /*//************************************************************************************************************
    double sigma=3.5;                                                   //fornisco la sigma
    
    TSpectrum* s = new TSpectrum(10);                                    //creo l'oggetto TSpectrum con il massimo numero di picchi
    int npecks = s->Search(h,sigma, "nodraw");                          //cerca i picchi e mi restituisce quanti picchi ha trovato 
    double* means = s->GetPositionX();                                //crea un array con la xposizione dei picchi
    double* ymean = s->GetPositionY();
    for (int p=0; p<npecks; p++)
    cout<<means[p]<<endl;
    cout <<endl<<endl;
    double mean=means[0]; 
    //************************************************************************************************************/
}

