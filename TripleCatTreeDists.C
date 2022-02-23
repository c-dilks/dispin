// TripleCatTreeDists ------------------------------
// draw kinematic distributions from 3 catTree*.root files
R__LOAD_LIBRARY(DiSpin)
#include "Tools.h"
#include "Constants.h"

TFile * infile[3];
TString outdir;
TTree * tr[3];
TCanvas * canv;
Float_t textSize=0.04;
int f;
Double_t normalizer[3];
const Int_t NBINS = 100;
TCut extraCut;

void CompareDist(TString varname, TString vartitle, Double_t forceMin=UNDEF, Double_t forceMax=UNDEF);

void TripleCatTreeDists(
  TString infile0N="catTreeData.rga.inbending.all.idx.root", // light red uptriangles
  TString infile1N="catTreeData.rgb.inbending.all.idx.root", // dark blue downtriangles
  TString infile2N="catTreeMC.mc.inbending.all.idx.root", // green circles
  TString outdir_="cattreetriple", // output directory
  TCut extraCut_ = "" // additional cuts
) {
  infile[0] = new TFile(infile0N,"READ");
  infile[1] = new TFile(infile1N,"READ");
  infile[2] = new TFile(infile2N,"READ");
  for(f=0;f<3;f++) tr[f] = (TTree*)infile[f]->Get("tree");
  outdir = outdir_;
  extraCut = extraCut_;

  gStyle->SetOptStat(0);
  gStyle->SetPalette(kGreenPink);
  gROOT->ProcessLine(Form(".! mkdir -p %s",outdir.Data()));

  // set normalization
  for(f=0;f<3;f++) {
    //normalizer[f] = ((TH1D*)infile[f]->Get("dihadronCntDist"))->GetEntries();// electron yield
    normalizer[f] = (Double_t)tr[f]->GetEntries(extraCut);
    //normalizer[f] = 1;
  };

  CompareDist("Mh",     "M_{h} [GeV]",     0.0,  2.5  );
  CompareDist("X",      "x",               0.0,  1.0  );
  CompareDist("Z",      "z",               0.0,  1.0  );
  CompareDist("Q2",     "Q^{2} [GeV^{2}]", 0.0,  11.0 );
  CompareDist("PhPerp", "p_{T} [GeV]",     0.0,  2.5  );
  CompareDist("PhiH",   "#phi_{h}",        -PI,  PI   );
  CompareDist("PhiR",   "#phi_{R}",        -PI,  PI   );
  CompareDist("PhiD",   "#Delta#phi",      -PI,  PI   );
  CompareDist("Theta",  "#theta",          0.0,  PI   );
  CompareDist("Depol2", "K_{2}",           0.2,  1.2  );
  CompareDist("Depol3", "K_{3}",           0.3,  0.8  );
  CompareDist("Mmiss",  "M_{X}",           1.3,  3.3  );
  CompareDist("XF",     "x_{F}",           -0.1, 0.9  );
  CompareDist("DYsgn",  "#Delta Y_{h}",    -3.0, 3.0  );
  // CompareDist("diphM",  "M_{#gamma#gamma} [GeV]", 0.0,  2.0  );
};


void CompareDist(TString varname, TString vartitle, Double_t forceMin, Double_t forceMax) {
  TH1D * dist[3];
  TString distN[3];
  Double_t varmin[3];
  Double_t varmax[3];
  printf("compare %s\n",varname.Data());

  // {min,max}ima
  Double_t distmin,distmax;
  if(forceMin<=UNDEF && forceMax<=UNDEF) { // determine {min,max}ima automatically
    for(f=0;f<3;f++) {
      varmin[f] = tr[f]->GetMinimum(varname);
      varmax[f] = tr[f]->GetMaximum(varname);
    };
    distmin = TMath::Min(
        0.0,
        TMath::Min(TMath::Min(varmin[0],varmin[1]),varmin[2])
        );
    distmax = TMath::Max(TMath::Max(varmax[0],varmax[1]),varmax[2]);
  } else { // use preferred values
    distmin = forceMin;
    distmax = forceMax;
  };
  Int_t numBins = NBINS;
  if(varname=="diphM") { // overrides -- make sure `extraCut_` matches
    numBins*=2;
    //distmin = 0.05; // avoid low diphM
    //distmax = 0.05; // low diphM
    //distmin = 0.05; distmax = 0.4; // pi0
    //distmin = 0.35; distmax = 0.7; // eta
  };

  // formatting
  Color_t color[3] = {
    kRed-7,
    kBlue+3,
    kGreen+1
  };
  Style_t style[3] = {
    kFullTriangleUp,
    kFullTriangleDown,
    kFullCircle
  };

  // distributions
  TString distT = vartitle;
  distT(TRegexp(" \\[.*\\]")) = "";
  distT += " distributions;"+vartitle;
  for(f=0;f<3;f++) {
    distN[f] = Form("%sDist%d",varname.Data(),f);
    dist[f] = new TH1D(distN[f],distT,numBins,distmin,distmax);
    tr[f]->Project(distN[f],varname,extraCut);
    dist[f]->Scale(1/normalizer[f]); // normalization
    //dist[f]->Sumw2(); // (already done, redundant)
    dist[f]->SetMarkerStyle(style[f]);
    dist[f]->SetMarkerSize(1.0);
    dist[f]->SetMarkerColor(color[f]);
    dist[f]->SetLineColor(color[f]);
    dist[f]->SetLineWidth(2);
    dist[f]->GetXaxis()->SetTitleSize(textSize);
    dist[f]->GetXaxis()->SetLabelSize(textSize);
    dist[f]->GetYaxis()->SetTitleSize(textSize);
    dist[f]->GetYaxis()->SetLabelSize(textSize);
  };

  // draw canvas
  canv = new TCanvas( TString(varname+"_canv"), TString(varname+"_canv"), 1000, 800);
  canv->SetGrid(1,1);
  canv->SetLeftMargin(0.15);
  canv->SetBottomMargin(0.15);
  for(f=0;f<3;f++) dist[f]->Draw(f==0?"P":"PSAME");
  Tools::UnzoomVertical(canv,"",0.0);
  canv->Print(Form("%s/%s.png",outdir.Data(),varname.Data()));

  // print
  cout << "integrals:"
       << " " << dist[0]->Integral()
       << " " << dist[1]->Integral()
       << endl;

};
