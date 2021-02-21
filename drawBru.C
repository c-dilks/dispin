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
Int_t nDim, nBins;
TString bruDir;
HS::FIT::Bins * HSbins;

/////////////////////////////

class BruBin : public TObject {
  public:
    Int_t idx;
    TString name,var;
    Double_t center,mean,ub,lb;
    TH1D * hist;
    // -constructor
    BruBin(TAxis axis, Int_t binnum, TVectorD coord) {
      idx = HSbins->FindBin(coord); // bin index
      name = HSbins->GetBinName(idx); // bin name (brufit syntax)
      var = axis.GetName(); // bin variable
      // get bin center, lower bound, upper bound
      center = axis.GetBinCenter(binnum);
      lb = axis.GetBinLowEdge(binnum);
      ub = axis.GetBinUpEdge(binnum);
      // get bin mean (requires opening the bin's tree)
      hist = new TH1D( Form("ivHist%d",idx),Form("ivHist%d",idx),
        100,lb-0.05,ub+0.05);
      TFile * binTreeFile;
      TTree * binTree;
      Double_t iv;
      binTreeFile = new TFile(bruDir+"/"+name+"/TreeData.root","READ");
      binTree = (TTree*) binTreeFile->Get("tree");
      binTree->SetBranchAddress(var,&iv);
      for(Long64_t e=0; e<binTree->GetEntries(); e++) {
        binTree->GetEntry(e);
        hist->Fill(iv);
      };
      mean = hist->GetMean();
      binTreeFile->Close();
    };
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

void drawBru(TString bruDir_="bruspin") {

  // get nDim
  bruDir = bruDir_;
  TFile * binFile = new TFile(bruDir+"/DataBinsConfig.root","READ");
  HSbins = (HS::FIT::Bins*) binFile->Get("HSBins");
  nDim = HSbins->GetNAxis();
  printf("nDim = %d\n",nDim);

  // get Nbins
  nBins = HSbins->GetN();
  printf("nBins = %d\n",nBins);

  // build array of BruBin objects
  BruBins = new TObjArray();
  TVectorD binCoord(nDim);
  if(nDim==1) {
    for(TAxis axis0 : HSbins->GetVarAxis()) {
      for(int bn=1; bn<=axis0.GetNbins(); bn++) {
        binCoord[0] = axis0.GetBinCenter(bn);
        BruBins->AddLast(new BruBin(axis0,bn,binCoord));
      };
    };
  };


  // print bins
  BruBin * BB;
  TObjArrayIter nextBin(BruBins);
  while((BB = (BruBin*) nextBin())) {
    BB->PrintInfo();
  };
  nextBin.Reset();




  // cleanup
  binFile->Close();

};
