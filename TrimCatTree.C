// filter catTree, in particular, cut `diphM`, the diphoton invariant
// mass, in preparation of an sFit
//
void TrimCatTree(TString infileN="catTreeMC.mc.PRL.0x3b.idx.root", TString treeName="tree") {

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

  // set cut
  TCut trimCut;
  if(infileN.Contains("catTreeMC")) { // MC file
    trimCut = "diphMCmatchDist<0.02 && diphIsMCpi0 && diphM>0.108 && diphM<0.160"; // MC pi0 decays
    //trimCut = "diphMCmatchDist<0.02 && diphIsMCpi0"; // MC pi0 decays, no diphM cut
    //trimCut = "diphIsMCpi0"; // MC pi0 decays, no diphM cut
  } else { // data file
    trimCut = "0.08<diphM && diphM<0.2"; // match sPlotBru.C fit range
  };
  cout << "trimCut = " << trimCut << endl;

  // clone input tree
  TTree * outtree = intree->CopyTree(trimCut);

  // write and close
  outtree->Write(treeName);
  outfile->Close();
  infile->Close();
};
