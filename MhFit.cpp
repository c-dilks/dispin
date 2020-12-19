#include <cstdlib>
#include <iostream>

// ROOT
#include <TFile.h>
#include <TString.h>
#include <TMath.h>
#include <TSystem.h>
#include <TRegexp.h>
#include <TH1.h>
#include <TCanvas.h>

// RooFit
#include <RooGlobalFunc.h>
#include <RooRealVar.h>
#include <RooAbsReal.h>
#include <RooDataSet.h>
#include <RooDataHist.h>
#include <RooGaussian.h>
#include <RooBreitWigner.h>
#include <RooPolynomial.h>
#include <RooChebychev.h>
#include <RooGenericPdf.h>
#include <RooAddPdf.h>
#include <RooFitResult.h>
#include <RooPlot.h>


TString infileN;
using namespace RooFit;

int main(int argc, char** argv) {

  // ARGUMENTS
  infileN = "plots.inbending.noXFcut.root";
  if(argc>1) infileN = TString(argv[1]);

  // open mass distribution
  TFile * infile = new TFile(infileN,"READ");
  TH1D * MhDist = (TH1D*) infile->Get("MhDist");
  RooRealVar mass("mass","M_{h}",0,3,"GeV");
  RooDataHist massdist("massdist","M_{h} distribution",mass,Import(*MhDist));

  // resonances
  // -- rho
  RooRealVar resRhoN("resRhoN","N_{#rho}",100e3,0,1e6,"");
  RooRealVar resRhoMean("resRhoMean","M_{#rho}",0.77,"GeV");
  RooRealVar resRhoWidth("resRhoWidth","#Gamma_{#rho}", 0.3, 0.01, 2, "GeV");
  RooBreitWigner resRho("resRho","#rho_{0} resonance",mass,resRhoMean,resRhoWidth);
  // -- f0
  RooRealVar resF0N("resF0N","N_{f0}",100e3,0,1e6,"");
  RooRealVar resF0Mean("resF0Mean","M_{f0}",0.98,"GeV");
  RooRealVar resF0Width("resF0Width","#Gamma_{f0}", 0.3, 0.001, 0.2, "GeV");
  RooBreitWigner resF0("resF0","f_{0} resonance",mass,resF0Mean,resF0Width);
  // -- f2
  RooRealVar resF2N("resF2N","N_{f2}",100e3,0,5e5,"");
  RooRealVar resF2Mean("resF2Mean","M_{f2}",1.27,"GeV");
  RooRealVar resF2Width("resF2Width","#Gamma_{f2}", 0.3, 0.001, 0.2, "GeV");
  RooBreitWigner resF2("resF2","f_{2} resonance",mass,resF2Mean,resF2Width);

  // background
  RooRealVar bgN("bgN","N_{bg}",5e3,0,1e6,"");
  RooRealVar bgP0("bgP0","p_0",-0.1,0.1,"");
  RooRealVar bgP1("bgP1","p_1",-0.1,0.1,"");
  RooRealVar bgP2("bgP2","p_2",-0.1,0.1,"");
  RooPolynomial bg("bg","background",mass,RooArgSet(bgP0,bgP1,bgP2));

  // composite
  RooAddPdf model("model","model",
    RooArgList(resRho,resF0,resF2,bg),
    RooArgList(resRhoN,resF0N,resF2N,bgN)
  );

  // set fit range
  mass.setRange("fit_range",0.6,1.5);

  // perform fit
  RooFitResult * modelFit = model.fitTo(
    massdist,
    Range("fit_range"),
    Extended(kTRUE),
    Save(kTRUE)
  );

  // plot
  RooPlot * plotframe = mass.frame(Title("M_{h} distribution"));
  massdist.plotOn( plotframe, Name("massdistPlot"), LineWidth(0), LineColor(kWhite), XErrorSize(0) ); // invisible (to set a frame)
  model.plotOn( plotframe, Name("modelPlot"), Range("Full"), LineColor(kBlack), LineWidth(4));
  model.plotOn( plotframe, Name("resRhoPlot"), Components("resRho"), Range("Full"), LineColor(kMagenta), LineWidth(4));
  model.plotOn( plotframe, Name("resF0Plot"), Components("resF0"), Range("Full"), LineColor(kOrange), LineWidth(4));
  model.plotOn( plotframe, Name("resF2Plot"), Components("resF2"), Range("Full"), LineColor(kRed), LineWidth(4));
  model.plotOn( plotframe, Name("bgPlot"), Components("bg"), Range("Full"), LineColor(kBlue), LineWidth(4), LineStyle(9));

  TCanvas * canv = new TCanvas("canv","canv",1200,800);
  plotframe->Draw();
  MhDist->SetMarkerStyle(kFullCircle);
  MhDist->SetMarkerSize(1);
  MhDist->SetMarkerColor(kBlack);
  MhDist->Draw("PSAME");
  canv->Print("canv.png","png");



};

