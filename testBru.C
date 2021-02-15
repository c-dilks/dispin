R__LOAD_LIBRARY(DiSpin)
#include "BruAsymmetry.h"
void testBru(TString infileN = "spinroot/cat.root") {

  // RooDataSet name: expects single 1D x-bin `cat.root` file;
  // use `loopBuildSpinroot1Bin.sh` to produce this
  TString roodataN = "A_X0/stream_cat_rfData_X0";
  
  // obtain RooDataSets for data and MC
  TFile * infile = new TFile(infileN,"READ");
  RooDataSet * rooData = (RooDataSet*) infile->Get(roodataN);
  TFile * mcfile = new TFile("catMC.root","READ");
  RooDataSet * rooMC = (RooDataSet*) mcfile->Get(roodataN);

  // load macros needed for PROOF
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
  B->LoadDataSets(rooData,rooMC);

  // MCMC settings
  B->MCMC_iter = 100;
  B->MCMC_burnin = 30;
  B->MCMC_norm= 1.0 / 0.01;

  // perform fit
  B->Fit();
};
