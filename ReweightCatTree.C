// change value of `Weight` in catTree
void ReweightCatTree(TString catTreeFileN, Double_t newWeight) {

  cout << endl << "reweighting " << catTreeFileN << endl;
  TFile *inFile = new TFile(catTreeFileN,"READ");
  TString outFileN = catTreeFileN;
  outFileN.ReplaceAll(".root",".REWEIGHTED.root");
  cout << "reweighted tree will be in: " << outFileN << endl;

  TTree *inTr = (TTree*)inFile->Get("tree");
  Double_t weight;
  inTr->SetBranchAddress("Weight",&weight);

  TFile *outFile = new TFile(outFileN,"RECREATE");
  TTree *outTr = inTr->CloneTree(0);

  cout << "reweighting..." << endl;
  for(Long64_t e=0; e<inTr->GetEntries(); e++) {
    inTr->GetEntry(e);
    weight = newWeight;
    outTr->Fill();
  }
  outTr->Write();
  outFile->Close();
  inFile->Close();
  cout << "...done" << endl;
};
