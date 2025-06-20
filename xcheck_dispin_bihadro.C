void xcheck_dispin_bihadro(TString infileN = "test_zone/nSidis_005032.hipo.root") {
  auto infile = new TFile(infileN, "READ");
  auto tr = (TTree*) infile->Get("tree");
  cout << tr->GetEntries("eleFiduCut>0&&hadFiduCut[0]>0&&hadFiduCut[1]>0") << endl;
}
