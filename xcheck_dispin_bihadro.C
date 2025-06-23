// cross check old code (dispin) with new code (bihadro)
void xcheck_dispin_bihadro(TString infileN = "outroot.test_zone/nSidis_005032.hipo.root") {

  // open ROOT file
  auto infile = new TFile(infileN, "READ");
  auto tr = (TTree*) infile->Get("tree");

  // cut for consistency with bihadro
  TCut xcheck_cut = "eleFiduCut>0&&hadFiduCut[0]>0&&hadFiduCut[1]>0";
  cout << tr->GetEntries(xcheck_cut) << endl;

  // write the filtered tree to a new file, and open it
  TString outfile_name = "tmp_dispin.root";
  TFile* outfile = new TFile(outfile_name, "RECREATE");
  TTree* ctr = tr->CopyTree(xcheck_cut);
  ctr->Write("ctr");
  infile->Close();
  outfile->Close();
  TFile* readfile = new TFile(outfile_name, "READ");
  new TBrowser();
}
