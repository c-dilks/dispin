// useful to compare specific asymmetry plots, especially if
// you want to compare two different modulations' amplitudes

R__LOAD_LIBRARY(DiSpin)
#include "Binning.h"

void CompareAsymsSpecific(
  TString infile0name="spinroot_pw_2/asym_9_mlm.root",
  TString infile1name="spinroot_final_2/asym_42_mlm.root"
) {

  Float_t diffMax = 0.02; // scale for difference plots

  // open asym*.root files
  TFile * infile[2];
  infile[0] = new TFile(infile0name,"READ");
  infile[1] = new TFile(infile1name,"READ");

  TGraphAsymmErrors * gr[2];
  //gr[0] = (TGraphAsymmErrors*) infile[0]->Get("kindepMA_A3_M"); // pw
  //gr[1] = (TGraphAsymmErrors*) infile[1]->Get("kindepMA_A2_M"); // final
  gr[0] = (TGraphAsymmErrors*) infile[0]->Get("kindepMA_A2_M"); // pw
  gr[1] = (TGraphAsymmErrors*) infile[1]->Get("kindepMA_A1_M"); // final

  TH1D * diffDist = new TH1D("diffDist","difference distribution",100,-diffMax,diffMax);
  TGraphAsymmErrors * diffGr = new TGraphAsymmErrors();
  TGraphAsymmErrors * ratGr = new TGraphAsymmErrors();
  diffGr->SetTitle("difference");
  diffGr->GetYaxis()->SetRangeUser(-diffMax,diffMax);
  diffGr->SetMarkerStyle(kFullCircle);
  diffGr->SetLineWidth(2);
  diffGr->SetMarkerColor(kBlack);
  diffGr->SetLineColor(kBlack);
  ratGr->SetTitle("ratio");
  ratGr->SetMarkerStyle(kFullCircle);
  ratGr->SetLineWidth(2);
  ratGr->SetMarkerColor(kBlack);
  ratGr->SetLineColor(kBlack);


  Double_t x[2];
  Double_t y[2];
  Double_t ex[2];
  Double_t ey[2];
  Double_t ay[2];
  Double_t xx,exx;
  Double_t xdif,exdif;
  Double_t xrat,exrat;

  for(int i=0; i<gr[0]->GetN(); i++) {
    for(int g=0; g<2; g++) {
      gr[g]->GetPoint(i,x[g],y[g]);
      ex[g] = gr[g]->GetErrorX(i); // (parabolic error)
      ey[g] = gr[g]->GetErrorY(i); // (parabolic error)
      ay[g] = fabs(y[g]);
    };
    gr[1]->SetPoint(i,x[1]+0.01,y[1]); // horizontal offset

    // compute difference
    xx = x[0];
    xdif = y[0]-y[1];
    xrat = ay[1]>0 ? ay[0]/ay[1] : 1;

    // correlated error (assumes dataset in infile[0] is a subset or
    // equal to the dataset in infile[1], or vice versa)
    exdif = TMath::Sqrt(TMath::Abs(ey[0]*ey[0]-ey[1]*ey[1]));
    exrat = ey[0]>0 && ey[1]>0 ?
            (ay[0]/ay[1])*TMath::Sqrt(
              ey[0]*ey[0]/(ay[0]*ay[0])
             +ey[1]*ey[1]/(ay[1]*ay[1])
             -2*ey[0]*ey[0]/(ay[0]*ay[1])
            ) : 0;
    exx = ex[0]; // (not used)

    diffDist->Fill(xdif);

    diffGr->SetPoint(i,xx,xdif);
    diffGr->SetPointError(i,exx,exx,exdif,exdif);
    ratGr->SetPoint(i,xx,xrat);
    ratGr->SetPointError(i,exx,exx,exrat,exrat);
  };

  TMultiGraph * mgr = new TMultiGraph();
  for(int g=0; g<2; g++) mgr->Add(gr[g]);
  gr[0]->SetMarkerColor(kGreen+2);
  gr[0]->SetLineColor(kGreen+2);
  gr[1]->SetMarkerColor(kRed+2);
  gr[1]->SetLineColor(kRed+2);
  mgr->SetTitle(gr[0]->GetTitle());
  mgr->GetXaxis()->SetTitle(gr[0]->GetXaxis()->GetTitle());
  mgr->GetYaxis()->SetTitle(gr[0]->GetYaxis()->GetTitle());
  new TCanvas(); diffDist->Draw();
  new TCanvas(); diffGr->Draw("APE");
  new TCanvas(); ratGr->Draw("APE");
  new TCanvas(); mgr->Draw("APE");
};
