// CompareDiagnosticsDists ------------------------------
// draw kinematic distributions from diagnostics plots.root
// - useful for presentations

TFile * infile[2];
TCanvas * canv;
Float_t textSize=0.04;
int f;
Double_t electronCnt[2];

void CompareDist(TString distname) {
  TH1D * dist[2];
  TH1D * rat;
  printf("compare %s\n",distname.Data());

  // distributions
  for(f=0;f<2;f++) {
    dist[f] = (TH1D*) infile[f]->Get(distname);
    if(dist[f]==NULL) { printf("...not found...\n"); return; };
    dist[f]->Scale(1/electronCnt[f]); // normalize by electron yield
    //dist[f]->Sumw2(); // (already done, redundant)
    dist[f]->SetMarkerStyle(f==0?kFullTriangleUp:kFullTriangleDown);
    dist[f]->SetMarkerSize(2.0);
    dist[f]->SetMarkerColor(f==0?kGreen+1:kViolet+2);
    dist[f]->SetLineColor(f==0?kGreen+1:kViolet+2);
    dist[f]->SetLineWidth(2);
    dist[f]->GetXaxis()->SetTitleSize(textSize);
    dist[f]->GetXaxis()->SetLabelSize(textSize);
    dist[f]->GetYaxis()->SetTitleSize(textSize);
    dist[f]->GetYaxis()->SetLabelSize(textSize);
  };

  // ratio
  rat = (TH1D*) dist[0]->Clone();
  rat->Divide(dist[1]); // dist[0] / dist[1]
  rat->SetLineColor(kBlack);
  rat->SetMarkerColor(kBlack);
  rat->SetMarkerStyle(kFullCircle);
  rat->SetMarkerSize(0.5);
  rat->GetYaxis()->SetRangeUser(0.5,2);

  // draw canvas
  canv = new TCanvas(
    TString(distname+"_canv"),TString(distname+"_canv"),1600,800);
  canv->Divide(2,1);
  canv->cd(1); canv->GetPad(1)->SetGrid(1,1);
  dist[0]->Draw("P"); dist[1]->Draw("PSAME");
  canv->cd(2); canv->GetPad(2)->SetGrid(1,1);
  rat->Draw("E");
  canv->Print(TString(distname)+".png");
};


void CompareDist2D(TString distname) {
  TH2D * dist[2];
  TH2D * rat;
  printf("compare %s\n",distname.Data());

  // distributions
  for(f=0;f<2;f++) {
    dist[f] = (TH2D*) infile[f]->Get(distname);
    if(dist[f]==NULL) { printf("...not found...\n"); return; };
    dist[f]->Scale(1/electronCnt[f]); // normalize by electron yield
    //dist[f]->Sumw2(); // (already done, redundant)
    dist[f]->GetXaxis()->SetTitleSize(textSize);
    dist[f]->GetXaxis()->SetLabelSize(textSize);
    dist[f]->GetYaxis()->SetTitleSize(textSize);
    dist[f]->GetYaxis()->SetLabelSize(textSize);
  };

  // ratio
  rat = (TH2D*) dist[0]->Clone();
  rat->Divide(dist[1]); // dist[0] / dist[1]
  rat->GetZaxis()->SetRangeUser(0.1,2.1);

  // draw canvas
  canv = new TCanvas(
    TString(distname+"_canv"),TString(distname+"_canv"),2400,800);
  canv->Divide(3,1);
  canv->cd(1); canv->GetPad(1)->SetGrid(1,1); dist[0]->Draw("COLZ");
  canv->cd(2); canv->GetPad(2)->SetGrid(1,1); dist[1]->Draw("COLZ");
  canv->cd(3); canv->GetPad(3)->SetGrid(1,1); rat->Draw("COLZ");
  canv->Print(TString(distname)+".png");
};

/*
void DrawDist2D(TString distname) {
  TH2D * dist = (TH2D*) infile->Get(distname);
  dist->GetXaxis()->SetTitleSize(textSize);
  dist->GetXaxis()->SetLabelSize(textSize);
  dist->GetYaxis()->SetTitleSize(textSize);
  dist->GetYaxis()->SetLabelSize(textSize);
  canv = new TCanvas(TString(distname+"_canv"),TString(distname+"_canv"),1200,800);
  canv->SetGrid(1,1);
  dist->Draw("colz");
  canv->Print(TString(distname)+".png");
};

void DrawCanv(TString distname) {
  canv = (TCanvas*) infile->Get(distname);
  canv->SetGrid(1,1);
  canv->Draw();
  canv->SetWindowSize(1200,1200);
  canv->Show();
  canv->Print(TString(distname)+".png");
};
*/

///////////////////////////////////////////////////
///////////////////////////////////////////////////
///////////////////////////////////////////////////
  
void CompareDiagnosticsDists(
  TString infile0N="plots.inbending.root",
  TString infile1N="plots.rga_spring19.root"
) {
  infile[0] = new TFile(infile0N,"READ");
  infile[1] = new TFile(infile1N,"READ");
  gStyle->SetOptStat(0);
  gStyle->SetPalette(kGreenPink);


  // get electron yields, for normalizations
  for(f=0;f<2;f++) {
    electronCnt[f] = ((TH1D*)infile[f]->Get("dihadronCntDist"))->GetEntries();
  };

  CompareDist("Q2Dist");
  CompareDist("XDist");
  CompareDist("WDist");

  CompareDist("MhDist");
  CompareDist("ZpairDist");
  CompareDist("PhPerpDist");
  CompareDist("thetaDist");
  CompareDist("MmissDist");

  CompareDist("PhiHDist");
  CompareDist("PhiRDist");

  CompareDist("eleVzDist");
  CompareDist("vzDiffHadhad");
  CompareDist("piPlushadPhiHDist");
  CompareDist("piMinushadPhiHDist");

  CompareDist2D("Q2vsX");
  CompareDist2D("eleVxyDist");
  CompareDist2D("eleEvsPhi");
  CompareDist2D("vzDiffEleHad");
  CompareDist2D("PhiHvsPhiR");

};
