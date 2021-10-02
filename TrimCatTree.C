// filter catTree, in particular, cut `diphM`, the diphoton invariant
// mass, in preparation of an sFit
//
void TrimCatTree(TString infileN, TString treeName="tree") {

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
  TTree * outtree = intree->CopyTree("0.08<diphM && diphM<0.2"); // must match sPlotBru.C

  // write and close
  outtree->Write(treeName);
  outfile->Close();
  infile->Close();
};
