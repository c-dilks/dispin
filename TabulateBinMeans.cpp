#include <cstdlib>
#include <iostream>
#include <vector>
#include <map>

// ROOT
#include "TFile.h"
#include "TString.h"
#include "TMath.h"
#include "TRegexp.h"
#include "TH1.h"
#include "TH2.h"
#include "TF1.h"
#include "TSystem.h"
#include "TObjArray.h"

// dispin
#include "Constants.h"
#include "Binning.h"
#include "EventTree.h"

TString inputData;
Int_t pairType;
Int_t ivType;
int Ntmp;
int b;
Binning * BS;
EventTree * ev;

int PrintUsage();
void SetDefaultArgs();
void PrintMeans(TString title, TH1D ** dist);

int main(int argc, char** argv) {

  // read options
  SetDefaultArgs();
  int opt;
  enum inputType_enum {iFile,iDir};
  Int_t inputType = -1;
  while( (opt=getopt(argc,argv,"f:d:p:i:")) != -1 ) {
    switch(opt) {
      case 'f': /* input file */
        if(inputType>=0) return PrintUsage();
        inputData = optarg;
        inputType = iFile;
        break;
      case 'd': /* input directory */
        if(inputType>=0) return PrintUsage();
        inputData = optarg;
        inputType = iDir;
        break;
      case 'p': /* pair type (hexadecimal number) */
        pairType = (Int_t) strtof(optarg,NULL);
        break;
      case 'i': /* independent variables */
        ivType = (Int_t) strtof(optarg,NULL);
        break;
      default: return PrintUsage();
    };
  };
  if(inputType!=iFile && inputType!=iDir) {
    fprintf(stderr,"ERROR: must specify input file or directory\n");
    return PrintUsage();
  };
  if(pairType==0x99) {
    fprintf(stderr,"ERROR: pi0 pi0 channel is under construction\n");
    return 0;
  };

  // print arguments' values
  Tools::PrintSeparator(40,"=");
  printf("inputData = %s\n",inputData.Data());
  printf("pairType = 0x%x\n",pairType);
  printf("ivType = %d\n",ivType);

  // set binning scheme
  BS = new Binning(pairType);
  Bool_t schemeSuccess = BS->SetScheme(ivType);
  if(!schemeSuccess) {
    fprintf(stderr,"ERROR: Binning::SetScheme failed\n");
    return 0;
  };

  // instantiate EventTree 
  // (use 1 file if inputType==iFile, 
  //  or all root files in inputData if inputType==iDir)
  ev = new EventTree(inputData+(inputType==iDir?"/*.root":""),pairType);

  
  // define histograms
  TFile * outfile = new TFile("tables.root","RECREATE");
  Ntmp = BS->GetNbins(0);
  const int N = Ntmp;
  TH1D * distMh[N];
  TH1D * distX[N];
  TH1D * distZ[N];
  TH1D * distQ2[N];
  TH1D * distY[N];
  TH1D * distPhPerp[N];
  TH1D * distDepolA[N];
  TH1D * distDepolC[N];
  TH1D * distDepolW[N];
  TH1D * distDepolCA[N];
  TH1D * distDepolWA[N];
  TString bStr;
  for(b=0; b<N; b++) {
    bStr = Form("Bin%d",b);
    distMh[b] = new TH1D(TString("distMh"+bStr),TString("Mh for "+bStr),100,0,3);
    distX[b] = new TH1D(TString("distX"+bStr),TString("X for "+ bStr),100,0,1);
    distZ[b] = new TH1D(TString("distZ"+bStr),TString("Z for "+ bStr),100,0,1);
    distQ2[b] = new TH1D(TString("distQ2"+bStr),TString("Q2 for "+bStr),100,0,12);
    distY[b] = new TH1D(TString("distY"+bStr),TString("Y for "+ bStr),100,0,1);
    distPhPerp[b] = new TH1D(TString("distPhPerp"+bStr),TString("PhPerp for "+bStr),100,0,5);
    distDepolA[b] = new TH1D(TString("distDepolA"+bStr),TString("DepolA for "+bStr),1000,-1,2.5);
    distDepolC[b] = new TH1D(TString("distDepolC"+bStr),TString("DepolC for "+bStr),1000,-1,2.5);
    distDepolW[b] = new TH1D(TString("distDepolW"+bStr),TString("DepolW for "+bStr),1000,-1,2.5);
    distDepolCA[b] = new TH1D(TString("distDepolCA"+bStr),TString("DepolCA for "+bStr),1000,-1,2.5);
    distDepolWA[b] = new TH1D(TString("distDepolWA"+bStr),TString("DepolWA for "+bStr),1000,-1,2.5);
  };




  //-----------------------------------------------------
  // EVENT LOOP  
  //-----------------------------------------------------
  Int_t bn;
  Float_t iv;
  printf("begin loop through %lld events...\n",ev->ENT);
  for(int i=0; i<ev->ENT; i++) {
    ev->GetEvent(i);
    if(ev->Valid()) {

      // set iv variable
      switch(BS->ivVar[0]) {
        case Binning::vM: iv = ev->Mh; break;
        case Binning::vX: iv = ev->x; break;
        case Binning::vZ: iv = ev->Zpair; break;
        case Binning::vPt: iv = ev->PhPerp; break;
        case Binning::vPh: iv = ev->Ph; break;
        case Binning::vQ: iv = ev->Q2; break;
        case Binning::vXF: iv = ev->xF; break;
        default: 
         fprintf(stderr,"ERROR: bad iv\n");
         return false;
      };

      // get bin number
      bn = BS->GetSchemeBin(iv);

      // fill histograms
      distMh[bn]->Fill(ev->Mh);
      distX[bn]->Fill(ev->x);
      distZ[bn]->Fill(ev->Zpair);
      distQ2[bn]->Fill(ev->Q2);
      distY[bn]->Fill(ev->y);
      distPhPerp[bn]->Fill(ev->PhPerp);
      distDepolA[bn]->Fill(ev->GetDepolarizationFactor('A'));
      distDepolC[bn]->Fill(ev->GetDepolarizationFactor('C'));
      distDepolW[bn]->Fill(ev->GetDepolarizationFactor('W'));
      distDepolCA[bn]->Fill(ev->GetDepolarizationFactor('C')/ev->GetDepolarizationFactor('A'));
      distDepolWA[bn]->Fill(ev->GetDepolarizationFactor('W')/ev->GetDepolarizationFactor('A'));
    };
  };

  // print means for cross check
  PrintMeans("x",distX);
  PrintMeans("z",distZ);
  PrintMeans("Mpipi",distMh);
  PrintMeans("Q2",distQ2);
  PrintMeans("y",distY);
  PrintMeans("pHT",distPhPerp);
  PrintMeans("A(y,eps)",distDepolA);
  PrintMeans("C(y,eps)",distDepolC);
  PrintMeans("W(y,eps)",distDepolW);
  PrintMeans("C/A(y,eps)",distDepolCA);
  PrintMeans("W/A(y,eps)",distDepolWA);

    
  // write histograms
  for(b=0; b<N; b++) {
    distMh[b]->Write();
    distX[b]->Write();
    distZ[b]->Write();
    distQ2[b]->Write();
    distY[b]->Write();
    distPhPerp[b]->Write();
    distDepolA[b]->Write();
    distDepolC[b]->Write();
    distDepolW[b]->Write();
    distDepolCA[b]->Write();
    distDepolWA[b]->Write();
  };

};


///////////////////////////////////////////

void PrintMeans(TString title, TH1D ** dist) {
  TString printStr = title + ": {";
  for(b=0; b<Ntmp; b++) {
    printStr = Form("%s%s%.3f",printStr.Data(),b>0?", ":"",dist[b]->GetMean());
  };
  printStr += "}";
  printf("%s\n",printStr.Data());
};

// set default arguments
void SetDefaultArgs() {
  inputData = "";
  pairType = EncodePairType(kPip,kPim);
  ivType = Binning::vM + 1;
};


// help printout
int PrintUsage() {

  SetDefaultArgs();
  BS = new Binning(EncodePairType(kPip,kPim));
  fprintf(stderr,"\nUSAGE: buildSpinroot.exe [-f or -d input_data ] [options...]\n\n");

  printf("INPUT DATA:\n");
  printf(" -f\tsingle ROOT file\n");
  printf(" -d\tdirectory of ROOT files\n");
  printf(" NOTE: specify input with either -f or -d, but not both\n");
  printf("\n");

  printf("OPTIONS:\n");

  printf(" -p\tpair type, specified as a hexadecimal number\n");
  printf("   \trun PrintEnumerators.C for notation\n");
  printf("   \tdefault = 0x%x (%s)\n\n",pairType,PairTitle(pairType).Data());

  printf(" -i\tindependent variable specifier: 1, 2, or 3-digit number which\n");
  printf("   \tspecifies the independent variables that asymmetries will be\n");
  printf("   \tplotted against. The number of digits will be the number of\n");
  printf("   \tdimensions in the multi-dimensional binning\n");
  printf("   \t* the allowed digits are:\n");
  BS = new Binning(pairType);
  for(int i=0; i<Binning::nIV; i++) {
    printf("   \t  %d = %s\n",i+1,(BS->IVtitle[i]).Data());
  };
  printf("   \tdefault = %d\n\n",ivType);

  return 0;
};
