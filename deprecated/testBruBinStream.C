R__LOAD_LIBRARY(DiSpin)
#include "Constants.h"
void testBruBinStream() {
  // TString bruDir = "bruspin.inj0"; // 1D
  TString bruDir = "bruspin.testBruBin.zm"; // 2D
  TFile *testFile = new TFile("testStream.root","RECREATE");
  TFile *binFile = new TFile(bruDir+"/DataBinsConfig.root","READ");
  HS::FIT::Bins * HSbins = (HS::FIT::Bins*) binFile->Get("HSBins");
  // BruBin *bb = new BruBin(bruDir,HSbins,1); // 1D
  BruBin *bb = new BruBin(bruDir,HSbins,1,1); // 2D
  bb->CalculateStats();
  // bb->OpenResultFile(mkMinuit); // 1D
  bb->OpenResultFile(mkMCMC); // 2D
  testFile->cd();
  bb->Write("bb");
  binFile->Close();
  testFile->Close();
};

