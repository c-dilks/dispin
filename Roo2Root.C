// convert RooDataSet `roodataN` to TTree on the fly
void Roo2Root(
  TString infileN = "spinroot/cat.root",
  TString roodataN = "A_M1/stream_cat_rfData_M1"
) {
  TFile * infile = new TFile(infileN,"READ");
  RooDataSet * roodata = (RooDataSet*) infile->Get(roodataN);
  TFile * tmpfile = new TFile("tmp.root","RECREATE");
  roodata->convertToTreeStore();
  TTree * tree = (TTree*) roodata->tree();
  tree->Write("tree");
  tree->Print();
};

