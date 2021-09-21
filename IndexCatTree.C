// add index branch to catTree (basically the same as $BRUFIT/macros/AddIDBranch.C)
void IndexCatTree(TString infileN, TString treeName="tree") {

  // open input tree
  TFile *infile = new TFile(infileN,"READ");
  TTree *intree = (TTree*) infile->Get(treeName);

  // check if index branch exists
  if(intree->GetBranch("Idx")){
    cout<<"Branch Idx already exists, exiting "<<endl;
    return;
  };

  // open output file
  TString outfileN = infileN;
  outfileN(TRegexp("\\.root$")) = ".idx.root";
  if(infileN==outfileN) {
    cerr << "ERROR: output file name not properly formatted; quitting" << endl;
    return;
  };
  cout << "indexed tree will be written to: " << outfileN << endl;
  TFile * outfile = new TFile(outfileN,"RECREATE");

  // clone input tree
  TTree * outtree = intree->CloneTree();

  // add unique ID branch "Idx" to output tree
  Double_t Idx=0;
  auto IdxBr = outtree->Branch("Idx",&Idx,"Idx/D");

  // tree loop
  Long64_t ENT = outtree->GetEntries();
  cout << "Indexing tree..." << endl;
  for(Long64_t i=0; i<ENT; i++) { IdxBr->Fill(); Idx+=1; };
  cout << "DONE Indexing tree." << endl;
  cout << "Indexed tree: " << outfileN << endl;

  // write and close
  outtree->Write(treeName);
  outfile->Close();
  infile->Close();
};
