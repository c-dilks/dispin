R__LOAD_LIBRARY(DiSpin)
#include "Tools.h"

void DrawCatTreeDists(TString infileN="catTreeData.XFgt0.root", TString outputPrefix="") {
  TFile * infile = new TFile(infileN,"READ");
  TTree * tree = (TTree*) infile->Get("tree");

  // define histograms
  TObjArray * histArr = new TObjArray();
  TH2D * ZvsMh = new TH2D(
    "ZvsMh",
    "z vs. M_{h};M_{h} [GeV];z",
    100,0,2.5,100,0.2,1);
    histArr->AddLast(ZvsMh);
  TH2D * PtVsMh = new TH2D(
    "PtVsMh",
    "p_{T} vs. M_{h};M_{h} [GeV];p_{T} [GeV]",
    100,0,2.5,100,0,2.5);
    histArr->AddLast(PtVsMh);
  TH2D * PtVsZ = new TH2D(
    "PtVsZ",
    "p_{T} vs. z;z;p_{T} [GeV]",
    100,0.2,1,100,0,2.5);
    histArr->AddLast(PtVsZ);
  TH1D * MhDist = new TH1D(
    "MhDist",
    "M_{h} distribution;M_{h} [GeV]",
    200,0,2);
    histArr->AddLast(MhDist);
  // // - diphoton Mass correlations
  // Double_t diphMmin = 0.07; // diphM plot range
  // Double_t diphMmax = 0.2;
  // TH2D * diphMvsMh = new TH2D(
  //   "diphMvsMh",
  //   "M_{#gamma#gamma} vs. M_{h};M_{h} [GeV];M_{#gamma#gamma} [GeV]",
  //   100,0,2,100,diphMmin,diphMmax);
  //   histArr->AddLast(diphMvsMh);
  // TH2D * diphMvsX = new TH2D(
  //   "diphMvsX",
  //   "M_{#gamma#gamma} vs. x;x;M_{#gamma#gamma} [GeV]",
  //   100,0,1,100,diphMmin,diphMmax);
  //   histArr->AddLast(diphMvsX);
  // TH2D * diphMvsZ = new TH2D(
  //   "diphMvsZ",
  //   "M_{#gamma#gamma} vs. z;z;M_{#gamma#gamma} [GeV]",
  //   100,0,1,100,diphMmin,diphMmax);
  //   histArr->AddLast(diphMvsZ);
  // TH2D * diphMvsPt = new TH2D(
  //   "diphMvsPt",
  //   "M_{#gamma#gamma} vs. p_{T};p_{T} [GeV];M_{#gamma#gamma} [GeV]",
  //   100,0,3,100,diphMmin,diphMmax);
  //   histArr->AddLast(diphMvsPt);
  // TH2D * diphMvsPhiH = new TH2D(
  //   "diphMvsPhiH",
  //   "M_{#gamma#gamma} vs. #phi_{h};#phi_{h};M_{#gamma#gamma} [GeV]",
  //   100,-TMath::Pi(),TMath::Pi(),100,diphMmin,diphMmax);
  //   histArr->AddLast(diphMvsPhiH);
  // TH2D * diphMvsPhiR = new TH2D(
  //   "diphMvsPhiR",
  //   "M_{#gamma#gamma} vs. #phi_{R};#phi_{R};M_{#gamma#gamma} [GeV]",
  //   100,-TMath::Pi(),TMath::Pi(),100,diphMmin,diphMmax);
  //   histArr->AddLast(diphMvsPhiR);
  // TH2D * diphMvsTheta = new TH2D(
  //   "diphMvsTheta",
  //   "M_{#gamma#gamma} vs. #theta;#theta;M_{#gamma#gamma} [GeV]",
  //   100,0,TMath::Pi(),100,diphMmin,diphMmax);
  //   histArr->AddLast(diphMvsTheta);

  // fill histograms
  tree->Project("ZvsMh","Z:Mh","Weight");
  tree->Project("PtVsMh","PhPerp:Mh","Weight");
  tree->Project("PtVsZ","PhPerp:Z","Weight");
  tree->Project("MhDist","Mh","Weight");
  // tree->Project("diphMvsMh","diphM:Mh","Weight");
  // tree->Project("diphMvsX","diphM:X","Weight");
  // tree->Project("diphMvsZ","diphM:Z","Weight");
  // tree->Project("diphMvsPt","diphM:PhPerp","Weight");
  // tree->Project("diphMvsPhiH","diphM:PhiH","Weight");
  // tree->Project("diphMvsPhiR","diphM:PhiR","Weight");
  // tree->Project("diphMvsTheta","diphM:Theta","Weight");

  // format histograms
  Float_t textSize=0.04;
  TObjArrayIter nextHist(histArr);
  while(TH1 *hist = (TH1*) nextHist()) {
    hist->GetXaxis()->SetTitleSize(textSize);
    hist->GetXaxis()->SetLabelSize(textSize);
    hist->GetYaxis()->SetTitleSize(textSize);
    hist->GetYaxis()->SetLabelSize(textSize);
    // correlations with diphM
    if(TString(hist->GetName()).Contains("diphM")) {
      TString ht = Form("%s -- corr=%.2f",
          hist->GetTitle(),
          ((TH2D*)hist)->GetCorrelationFactor()
          );
      hist->SetTitle(ht);
    };
  };
  MhDist->SetFillColor(kRed);
  MhDist->SetLineColor(kBlack);
  MhDist->SetLineWidth(3);
  gStyle->SetOptStat(0);

  // draw
  TCanvas * canv;
  /* // -- all canvases
  nextHist.Reset();
  while(TH1 *hist = (TH1*) nextHist()) {
    canv = new TCanvas();
    canv->SetGrid(1,1);
    canv->SetLogz();
    canv->SetLeftMargin(0.15);
    hist->Draw("COLZ");
    if(TString(hist->GetName()).Contains("diphM")) {
      Tools::ApplyProfile((TH2D*)hist,1);
      //hist->GetYaxis()->SetRangeUser(0.131,0.137);
    };
  };
  */
  ///* // -- specific canvases
  TLine * line;
  canv = new TCanvas("canv","canv",800,600);
  canv->SetRightMargin(0.15);
  canv->SetLeftMargin(0.1);
  canv->SetGrid(1,1);
  ZvsMh->Draw("COLZ");
  line = new TLine(0.63,ZvsMh->GetYaxis()->GetXmin(),0.63,ZvsMh->GetYaxis()->GetXmax());
  line->SetLineWidth(3); line->Draw();
  canv->SaveAs(outputPrefix+"ZvsMh.png");
  PtVsMh->Draw("COLZ");
  line = new TLine(0.63,PtVsMh->GetYaxis()->GetXmin(),0.63,PtVsMh->GetYaxis()->GetXmax());
  line->SetLineWidth(3); line->Draw();
  canv->SaveAs(outputPrefix+"PtVsMh.png");
  PtVsZ->Draw("COLZ");
  canv->SaveAs(outputPrefix+"PtVsZ.png");
  //*/
};
