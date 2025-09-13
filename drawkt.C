void drawkt(TString fn="diskim.sss/test.root") { // draw the kT distribution from a stringspinner file
  auto* f = new TFile(fn,"READ");
  auto* t = (TTree*) f->Get("ditr");
  new TCanvas();
  auto m = TMath::Power(t->GetMaximum("SS_kT"), 2);
  auto d = new TH1D("kT", "kT", 100, 0, m);
  t->Project("kT", "TMath::Power(SS_kT,2)", "SS_kT>-5000");
  d->Fit("gaus", "", "", 0, m);
}
