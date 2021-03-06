// draw kinematic distributions from diagnostics plots.root
// - useful for presentations

TFile * infile;
TCanvas * canv;
Float_t textSize=0.04;

void DrawDist(TString distname) {
  TH1D * dist = (TH1D*) infile->Get(distname);
  if(distname=="PhPerpDist") dist->SetTitle("p_{T} distribution;p_{T}");
  dist->SetLineWidth(3);
  dist->SetLineColor(kBlack);
  dist->GetXaxis()->SetTitleSize(textSize);
  dist->GetXaxis()->SetLabelSize(textSize);
  dist->GetYaxis()->SetTitleSize(textSize);
  dist->GetYaxis()->SetLabelSize(textSize);
  canv = new TCanvas(TString(distname+"_canv"),TString(distname+"_canv"),800,700);
  canv->SetGrid(1,1);
  dist->Draw();
  canv->Print("diagplots/"+TString(distname)+".png");
};

void DrawDist2D(TString distname) {
  TH2D * dist = (TH2D*) infile->Get(distname);
  dist->GetXaxis()->SetTitleSize(textSize);
  dist->GetXaxis()->SetLabelSize(textSize);
  dist->GetYaxis()->SetTitleSize(textSize);
  dist->GetYaxis()->SetLabelSize(textSize);
  canv = new TCanvas(TString(distname+"_canv"),TString(distname+"_canv"),800,700);
  canv->SetGrid(1,1);
  dist->Draw("colz");
  canv->Print("diagplots/"+TString(distname)+".png");
};

void DrawCanv(TString distname) {
  canv = (TCanvas*) infile->Get(distname);
  canv->SetGrid(1,1);
  canv->Draw();
  canv->SetWindowSize(1200,1200);
  canv->Show();
  canv->Print("diagplots/"+TString(distname)+".png");
};
  
void DrawDiagnosticsDists(TString infileN="plots.root") {
  infile = new TFile(infileN,"READ");
  gStyle->SetOptStat(0);
  gROOT->ProcessLine(".! mkdir -p diagplots");
  DrawDist("XDist");
  DrawDist("MhDist");
  DrawDist("ZpairDist");
  DrawDist2D("Q2vsX");
  DrawDist("thetaDist");
  DrawDist("PhPerpDist");
  DrawDist2D("YHvsXF_piPlus");
  //DrawDist2D("YHvsXF_piMinus");
  DrawDist2D("hadPperpVsYH_piPlus");
  DrawDist2D("PhiHvsPhiR");
  //DrawDist2D("hadPperpVsYH_piMinus");
  //DrawDist2D("DeltaPhiVsPhiR");
  DrawCanv("hadPperpCanv");
  DrawCanv("hadXFCanv");
  DrawCanv("hadYHCanv");
  DrawCanv("hadPhiHCanv");
};
