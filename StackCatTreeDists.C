// StackCatTreeDists ------------------------------
// draw kinematic distributions from a set of catTree*.root files
R__LOAD_LIBRARY(DiSpin)
#include "Tools.h"
#include "Constants.h"

const Int_t NSTACK_MAX = 4;
Int_t NSTACK;
TFile * infile[NSTACK_MAX];
TString outdir;
TTree * tr[NSTACK_MAX];
TCanvas * canv;
Float_t textSize=0.04;
int f;
Double_t normalizer[NSTACK_MAX];
const Int_t NBINS = 100;
TCut extraCut;

void CompareDist(TString varname, TString vartitle, Double_t forceMin=UNDEF, Double_t forceMax=UNDEF);

void StackCatTreeDists(
  TString infile0N="catTreeData.rga.bibending.all.idx.root", // light red closed circles
  TString infile1N="catTreeData.rgb.bibending.all.idx.root", // dark blue closed squares
  TString infile2N="catTreeMC.mca.bibending.all.idx.root",   // light red or black open circles
  TString infile3N="catTreeMC.mcb.bibending.all.idx.root",   // dark blue open squares
  TString outdir_="cattreestacks", // output directory
  TCut extraCut_ = "" // additional cuts, or weight
) {
  NSTACK = 0;
  if(infile0N!="") infile[NSTACK++] = new TFile(infile0N,"READ");
  if(infile1N!="") infile[NSTACK++] = new TFile(infile1N,"READ");
  if(infile2N!="") infile[NSTACK++] = new TFile(infile2N,"READ");
  if(infile3N!="") infile[NSTACK++] = new TFile(infile3N,"READ");
  for(f=0;f<NSTACK;f++) tr[f] = (TTree*)infile[f]->Get("tree");
  outdir = outdir_;
  extraCut = extraCut_;

  gStyle->SetOptStat(0);
  gStyle->SetPalette(kGreenPink);
  gROOT->ProcessLine(Form(".! mkdir -p %s",outdir.Data()));

  // set normalization
  for(f=0;f<NSTACK;f++) {
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
  TH1D * dist[NSTACK_MAX];
  TString distN[NSTACK_MAX];
  Double_t varmin[NSTACK_MAX];
  Double_t varmax[NSTACK_MAX];
  printf("compare %s\n",varname.Data());

  // {min,max}ima
  Double_t distmin,distmax;
  if(forceMin<=UNDEF && forceMax<=UNDEF) { // determine {min,max}ima automatically
    for(f=0;f<NSTACK;f++) {
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
  Color_t color[NSTACK_MAX] = {
    kRed-7,
    kBlue+3,
    kRed-7,
    kBlue+3
  };
  if(NSTACK==3) color[2] = kBlack;
  Style_t style[NSTACK_MAX] = {
    kFullCircle,
    kFullSquare,
    kOpenCircle,
    kOpenSquare
  };

  // distributions
  /*
  TString distT = vartitle;
  distT(TRegexp(" \\[.*\\]")) = "";
  distT += " distributions;"+vartitle;
  *///*
  TString distT = ";"+vartitle; // x-axis only
  //*/
  for(f=0;f<NSTACK;f++) {
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
  canv->SetLeftMargin(0.10);
  canv->SetBottomMargin(0.10);
  canv->SetTopMargin(0.03);
  canv->SetRightMargin(0.03);
  for(f=0;f<NSTACK;f++) dist[f]->Draw(f==0?"P":"PSAME");
  Tools::UnzoomVertical(canv,"",0.0);
  canv->Print(Form("%s/%s.png",outdir.Data(),varname.Data()));

  // print
  cout << "integrals:"
       << " " << dist[0]->Integral()
       << " " << dist[1]->Integral()
       << endl;

};
