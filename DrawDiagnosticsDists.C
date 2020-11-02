// draw kinematic distributions from diagnostics plots.root
// - useful for presentations

TFile * infile;
TCanvas * canv;
Float_t textSize=0.06;

void DrawDist(TString distname) {
  TH1D * dist = (TH1D*) infile->Get(distname);
  dist->SetLineWidth(6);
  dist->SetLineColor(kBlue);
  dist->GetXaxis()->SetTitleSize(textSize);
  dist->GetXaxis()->SetLabelSize(textSize);
  dist->GetYaxis()->SetTitleSize(textSize);
  dist->GetYaxis()->SetLabelSize(textSize);
  canv = new TCanvas(TString(distname+"_canv"),TString(distname+"_canv"),1200,800);
  dist->Draw();
};

void DrawDist2D(TString distname) {
  TH2D * dist = (TH2D*) infile->Get(distname);
  dist->GetXaxis()->SetTitleSize(textSize);
  dist->GetXaxis()->SetLabelSize(textSize);
  dist->GetYaxis()->SetTitleSize(textSize);
  dist->GetYaxis()->SetLabelSize(textSize);
  canv = new TCanvas(TString(distname+"_canv"),TString(distname+"_canv"),1200,800);
  dist->Draw("colz");
};
  
void DrawDiagnosticsDists(TString infileN="plots.root") {
  infile = new TFile(infileN,"READ");
  gStyle->SetOptStat(0);
  DrawDist("XDist");
  DrawDist("MhDist");
  DrawDist("ZpairDist");
  DrawDist2D("Q2vsX");
  DrawDist2D("PhiHvsPhiR");
  DrawDist("thetaDist");
};
