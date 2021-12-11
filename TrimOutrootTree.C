// filter outroot tree
//
void TrimOutrootTree(TString infileN="TESTMC.root", TString treeName="tree") {

  // open input tree
  TFile *infile = new TFile(infileN,"READ");
  TTree *intree = (TTree*) infile->Get(treeName);

  // open output file
  TString outfileN = infileN;
  outfileN(TRegexp("\\.root$")) = ".trimmed.root";
  if(infileN==outfileN) {
    cerr << "ERROR: output file name not properly formatted; quitting" << endl;
    return;
  };
  cout << "trimmed tree will be written to: " << outfileN << endl;
  TFile * outfile = new TFile(outfileN,"RECREATE");

  // clone input tree
  TCut trimCut = "pairType==0x34";
  TTree * outtree = intree->CopyTree(trimCut);

  // write and close
  outtree->Write(treeName);
  outfile->Close();
  infile->Close();
};
