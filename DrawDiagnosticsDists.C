// draw kinematic distributions from diagnostics plots.root
// - useful for presentations

TFile * infile;
TCanvas * canv;
Float_t textSize=0.04;
TString outDir;

void DrawDist(TString distname,Bool_t logy=false) {
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
  if(logy) canv->SetLogy(1);
  dist->Draw();
  canv->Print(outDir+"/"+TString(distname)+".png");
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
  canv->Print(outDir+"/"+TString(distname)+".png");
};

void DrawCanv(TString canvname, Bool_t logy=false) {
  canv = (TCanvas*) infile->Get(canvname);
  if(canv->GetListOfPrimitives()->GetEntries()>1) {
    for(int i=0; i<canv->GetListOfPrimitives()->GetEntries(); i++) canv->GetPad(i+1)->SetGrid(1,1);
    if(logy) canv->GetPad(1)->SetLogy();
  } else {
    canv->SetGrid(1,1);
    if(logy) canv->SetLogy();
  };
  canv->Draw();
  canv->SetWindowSize(1200,1200);
  canv->Show();
  canv->Print(outDir+"/"+TString(canvname)+".png");
};
  
void DrawDiagnosticsDists(TString infileN="plots.root", TString outDir_="diagplots") {
  infile = new TFile(infileN,"READ");
  outDir = outDir_;
  gStyle->SetOptStat(0);
  gROOT->ProcessLine(Form(".! mkdir -p %s",outDir.Data()));
  DrawDist("XDist");
  DrawDist("MhDist");
  DrawDist("ZpairDist");
  DrawDist2D("Q2vsX");
  DrawDist("thetaDist");
  DrawDist("PhPerpDist");
  DrawDist("eleVzDist",true);
  DrawCanv("hadEleVzDiffCanv",true);
  // DrawDist2D("YHvsXF_piPlus");
  // DrawDist2D("YHvsXF_piMinus");
  // DrawDist2D("hadPperpVsYH_piPlus");
  // DrawDist2D("PhiHvsPhiR");
  // DrawDist2D("hadPperpVsYH_piMinus");
  // DrawDist2D("DeltaPhiVsPhiR");
  DrawCanv("hadPperpCanv");
  DrawCanv("hadXFCanv");
  DrawCanv("hadYHCanv");
  DrawCanv("hadPhiHCanv");
};
