R__LOAD_LIBRARY(DiSpin)
#include "BruAsymmetry.h"
void testBru(TString infileN = "spinroot/cat.root") {

  TString roodataN = "A_M1/stream_cat_rfData_M1";
  TFile * infile = new TFile(infileN,"READ");
  RooDataSet * rooData = (RooDataSet*) infile->Get(roodataN);
  TFile * mcfile = new TFile("catMC.root","READ");
  RooDataSet * rooMC = (RooDataSet*) mcfile->Get(roodataN);

  // macros needed for PROOF
  ///*
  gSystem->Load("libRooStats");
  gSystem->Load("libProof");
  gSystem->Load("libMathMore");
  TString BRUCODE=gSystem->Getenv("BRUFIT");
  TString macpath=BRUCODE+"/macros";
  gROOT->SetMacroPath(
    Form("%s:%s",gROOT->GetMacroPath(),(macpath).Data()));
  //*/
   
  // instantiate
  BruAsymmetry * B = new BruAsymmetry("testbrudir");

  // build modulations
  B->AddNumerMod(new Modulation(3,0,0));
  B->AddNumerMod(new Modulation(2,1,1));
  B->AddNumerMod(new Modulation(3,1,1));
  B->AddNumerMod(new Modulation(3,1,-1));
  B->AddNumerMod(new Modulation(2,2,2));
  B->AddNumerMod(new Modulation(3,2,2));
  B->AddNumerMod(new Modulation(3,2,-2));

  // build full PDF
  B->BuildPDF();

  // binning
  Double_t thetaBins[3] = { 0, TMath::Pi()/2.0, TMath::Pi() };
  Double_t phiDbins[5] = { -TMath::Pi(), -TMath::Pi()/2.0, 0.0, TMath::Pi()/2.0, TMath::Pi() };
  B->FM->Bins().LoadBinVar("Theta",2,thetaBins);
  B->FM->Bins().LoadBinVar("PhiD",4,phiDbins);

  // code to cross check dispin Binning class with Brufit's
  /*
  Int_t nDim = B->FM->Bins().GetBins().GetNAxis();
  Int_t nBins;
  Int_t nBinsTotal = 1;
  TString axisName;
  for(int ia=0; ia<nDim; ia++) {
    nBins = B->FM->Bins().GetBins().GetVarAxis()[ia].GetNbins();
    axisName = B->FM->Bins().GetBins().GetVarAxis()[ia].GetName();
    nBinsTotal *= nBins;
    printf("%s axis has %d bins\n",axisName.Data(),nBins);
    for(int ib=1; ib<=nBins; ib++) {
      printf("  bin %d:  %f to %f\n",
        ib,
        B->FM->Bins().GetBins().GetVarAxis()[ia].GetBinLowEdge(ib),
        B->FM->Bins().GetBins().GetVarAxis()[ia].GetBinUpEdge(ib)
      );
    };
  }
  printf("there are %d bins in total\n",nBinsTotal);
  //printf("\nPrintAxis():\n");
  //B->FM->Bins().GetBins().PrintAxis();
  printf("BruAsymmetry methods:\n nBins=%d\n nDim=%d\n",
    B->GetNbins(),B->GetNdim());
  */

  // load data and MC trees
  B->LoadDataSets(rooData,rooMC);

  // MCMC settings
  B->MCMC_iter = 100;
  B->MCMC_burnin = 30;
  B->MCMC_norm= 1.0 / 0.01;

  // perform fit
  B->Fit();
};



