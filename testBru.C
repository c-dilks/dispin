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
  const Int_t nBins = 12;
  Double_t MhBins[nBins+1] = {
    0.0,
    0.381,
    0.462,
    0.531,
    0.606,
    0.675,
    0.737,
    0.781,
    0.831,
    0.900,
    0.981,
    1.125,
    3.0
  };
  B->FM->Bins().LoadBinVar("Mh",nBins,MhBins);
  //B->FM->Bins().LoadBinVar("Z",4,ZBins);
  B->PrintBinScheme();


  // load data and MC trees
  B->LoadDataSets("spinroot/catTree.root","catTreeMC.root");

  // MCMC settings
  B->MCMC_iter = 3000; // number of samples
  B->MCMC_burnin = 300; // number of initial samples to drop
  B->MCMC_norm = 1.0 / 0.03; // 1/stepsize

  // perform fit
  B->Fit();
};
