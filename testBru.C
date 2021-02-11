R__LOAD_LIBRARY(DiSpin)
#include "BruAsymmetry.h"
void testBru(TString infileN = "spinroot/cat.root") {

  TString roodataN = "A_M1/stream_cat_rfData_M1";
  TFile * infile = new TFile(infileN,"READ");
  RooDataSet * rooData = (RooDataSet*) infile->Get(roodataN);
  RooDataSet * rooMC = (RooDataSet*) rooData->Clone();

  // macros needed for PROOF
  /*
  TString BRUCODE=gSystem->Getenv("BRUFIT");
  TString macpath=BRUCODE+"/macros";
  gROOT->SetMacroPath(
    Form("%s:%s",gROOT->GetMacroPath(),(macpath).Data()));
  */
   
  BruAsymmetry * B = new BruAsymmetry("testbrudir");

  B->AddNumerMod(new Modulation(3,0,0));
  B->AddNumerMod(new Modulation(2,1,1));
  B->AddNumerMod(new Modulation(3,1,1));
  B->AddNumerMod(new Modulation(3,1,-1));
  B->AddNumerMod(new Modulation(2,2,2));
  B->AddNumerMod(new Modulation(3,2,2));
  B->AddNumerMod(new Modulation(3,2,-2));

  B->BuildPDF();

  B->LoadDataSets(rooData,rooMC);


  B->MCMC_iter = 10000;
  B->MCMC_burnin = 300;
  B->MCMC_norm= 20;

  B->Fit();
};



