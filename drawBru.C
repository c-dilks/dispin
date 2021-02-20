/*
#include <cstdlib>
#include <iostream>
#include <vector>
#include <map>
#include <TFile.h>
#include <TString.h>
#include <TCanvas.h>
#include <TGraphErrors.h>
#include "BruAsymmetry.h"
*/
R__LOAD_LIBRARY(DiSpin)
#include "BruAsymmetry.h"

/////////////////////////////

TObjArray * BruBins;
HS::FIT::Bins * HSbins;
class BruBin : public TObject {
  public:
    Int_t idx;
    TString name,var;
    Float_t center,mean,ub,lb;
    // -constructor
    BruBin() {};
    // -print out
    void PrintInfo() {
      printf("BIN %d\n",idx);
      printf("  name   = %s\n",name.Data());
      printf("  var    = %s\n",var.Data());
      printf("  center = %.2f\n",center);
      printf("  mean   = %.2f\n",mean);
      printf("  range  = %.2f to %.2f\n",lb,ub);
    };
};

/////////////////////////////

void drawBru(TString bruDir="bruspin") {

  // get nDim
  TFile * binFile = new TFile(bruDir+"/DataBinsConfig.root","READ");
  HSbins = (HS::FIT::Bins*) binFile->Get("HSBins");
  Int_t nDim = HSbins->GetNAxis();
  printf("nDim = %d\n",nDim);

  // get Nbins
  Int_t nBins = HSbins->GetN();
  printf("nBins = %d\n",nBins);

  // build array of BruBin objects
  BruBins = new TObjArray();
  TVectorD binCoord(nDim);
  BruBin * BB;
  if(nDim==1) {
    for(auto axis0 : HSbins->GetVarAxis()) {
      for(int b=1; b<=axis0.GetNbins(); b++) {
        BB = new BruBin();
        binCoord[0] = axis0.GetBinCenter(b);
        BB->idx = HSbins->FindBin(binCoord);
        BB->name = HSbins->GetBinName(BB->idx);
        BB->var = axis0.GetName();
        BB->center = axis0.GetBinCenter(b);
        BB->mean = 0; // (updated later)
        BB->lb = axis0.GetBinLowEdge(b);
        BB->ub = axis0.GetBinUpEdge(b);
        BruBins->AddLast(BB);
      };
    };
  }
  else { fprintf(stderr,"ERROR: nDim too large\n"); return 1; };

  // get bin means
  TObjArrayIter nextBin(BruBins);
  while((BB = (BruBin*) nextBin())) {
    
  };
  nextBin.Reset();


  // print bins
  while((BB = (BruBin*) nextBin())) {
    BB->PrintInfo();
  };
  nextBin.Reset();




  // cleanup
  binFile->Close();

};
