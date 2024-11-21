void drawkt(TString fn="diskim.sss/test.root") { // draw the kT distribution from a stringspinner file
  auto* f = new TFile(fn,"READ");
  auto* t = (TTree*) f->Get("ditr");
  new TCanvas();
  t->Draw("SS_kT","SS_kT>-5000");
}
