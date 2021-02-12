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
  /*
  Double_t thetaBins[3] = { 0, TMath::Pi()/2.0, TMath::Pi() };
  B->FM->Bins().LoadBinVar("Theta",2,thetaBins);
  */

  // load data and MC trees
  B->LoadDataSets(rooData,rooMC);

  // MCMC settings
  B->MCMC_iter = 5000;
  B->MCMC_burnin = 300;
  B->MCMC_norm= 20;

  // perform fit
  B->Fit();
};



