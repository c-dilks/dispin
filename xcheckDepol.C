// draw depol plots, transparently
void xcheckDepol(TString infileN="tables.root") {
  TFile *infile = new TFile(infileN,"READ");
  TCanvas *canv = new TCanvas("canv","canv",900,600);
  canv->SetGrid(1,1);
  TPad *pad = new TPad("pad","",0,0,1,1);
  TGraphErrors *gr = (TGraphErrors*) infile->Get("aveW_aveA_vs_aveX");
  // TGraphErrors *gr = (TGraphErrors*) infile->Get("aveWA_vs_aveX");
  gr->GetXaxis()->SetRangeUser(0.1,0.4);
  gr->GetYaxis()->SetRangeUser(0.3,0.7);
  gr->SetMarkerColor(kRed);
  gr->SetLineColor(kRed);
  gr->Draw("APE");
  canv->SaveAs("xcheck.png");
}
