R__LOAD_LIBRARY(DiSpin)
void testBruBinRead() {
  TFile *infile = new TFile("bruspin.inj0/asym_minuit_BL0.root","READ");
  TObjArray *arr = (TObjArray*) infile->Get("BruBinList");
  BruBin *bb = (BruBin*) arr->At(0);
  bb->PrintInfo();
  infile->Close();
}

