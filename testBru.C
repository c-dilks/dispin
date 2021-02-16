R__LOAD_LIBRARY(DiSpin)
#include "BruAsymmetry.h"
void testBru() {

  // load macros needed for PROOF
  TString BRUCODE=gSystem->Getenv("BRUFIT");
  TString macpath=BRUCODE+"/macros";
  gROOT->SetMacroPath(
    Form("%s:%s",gROOT->GetMacroPath(),(macpath).Data()));
   
  // instantiate
  BruAsymmetry * B = new BruAsymmetry("bruspin");

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
  Double_t MhBins[5] = { 0.0, 0.4, 0.6, 0.9, 3.0 };
  B->FM->Bins().LoadBinVar("Mh",4,MhBins);
  //B->FM->Bins().LoadBinVar("Z",4,ZBins);
  B->PrintBinScheme();


  // load data and MC trees
  B->LoadDataSets("spinroot/catTree.root","catTreeMC.root");

  // MCMC settings
  B->MCMC_iter = 100;
  B->MCMC_burnin = 30;
  B->MCMC_norm= 1.0 / 0.01;

  // perform fit
  B->Fit();
};
