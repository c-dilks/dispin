#include <cstdlib>
#include <iostream>
#include <map>

// ROOT
#include "TFile.h"
#include "TString.h"
#include "TMath.h"
#include "TSystem.h"
#include "TROOT.h"
#include "TRegexp.h"
#include "TH1.h"
#include "TTree.h"

// RooFit
#include <RooGlobalFunc.h>
#include <RooRealVar.h>
#include <RooAbsReal.h>
#include <RooDataSet.h>
#include <RooDataHist.h>
#include <RooGaussian.h>
#include <RooPolynomial.h>
#include <RooChebychev.h>
#include <RooGenericPdf.h>
#include <RooAddPdf.h>
#include <RooFitResult.h>
#include <RooPlot.h>
#include <RooStats/SPlot.h>

// dispin
#include <Binning.h>
#include <Tools.h>


using std::cout;
using std::cerr;
using std::endl;
using std::map;
using std::pair;

TString infileN, outfileN;
TFile *infile, *outfile;
TTree *tr;
Int_t ivType;
Int_t numBin[3];

Binning *BS;
Long64_t ENT;


// class for histograms, fit results, etc., for one bin of Binning scheme
class FitBin {
  public:
    TH1D *Mdist;
    Int_t binnum;
    RooFitResult *modelFit;
    RooPlot *plotFrame;
    TCanvas *fitCanv;

    // constructor -------------------------------------
    FitBin(Int_t binn) {
      binnum = binn;
      Mdist = new TH1D(
        Form("Mdist_%d",binnum),Form("Mdist_%d",binnum),
        100,0.04,0.45);
    };

    // fit algorithm -----------------------------------
    void Fit() {
      for(int i=0; i<3; i++) Tools::PrintSeparator(60,"=");
      cout << "[+++++] call fit on bin " << binnum << endl;

      RooRealVar mass("Mgg","M_{#gamma#gamma}",0,3,"GeV");
      TString massdistN = "roo_" + TString(Mdist->GetName());
      RooDataHist massdist(massdistN.Data(),massdistN.Data(),
          mass,RooFit::Import(*Mdist));

      // pi0 signal
      Double_t nmax = (Double_t)ENT;
      RooRealVar pi0n("pi0n","pi0n", nmax/2.0, 0, nmax);
      RooRealVar pi0mu("pi0mu","pi0mu", 0.135, 0, 2);
      RooRealVar pi0sigma("pi0sigma","pi0sigma", 0.02, 0.001, 0.1);
      RooGaussian pi0model("pi0model","pi0model",mass,pi0mu,pi0sigma);

      // background
      RooRealVar bgN("bgN","bgN", 0, nmax);
      RooRealVar bgP0("bgP0","bgP0", -1, 1);
      RooRealVar bgP1("bgP1","bgP1", -1, 1);
      RooRealVar bgP2("bgP2","bgP2", -1, 1);
      RooRealVar bgP3("bgP3","bgP3", -1, 1);
      RooRealVar bgP4("bgP4","bgP4", -1, 1);
      RooPolynomial bgModel("bgModel","bgModel",mass,RooArgSet(bgP0,bgP1,bgP2));
      //RooChebychev bgModel("bgModel","bgModel",mass,RooArgSet(bgP0,bgP1,bgP2,bgP3));
      //RooChebychev bgModel("bgModel","bgModel",mass,RooArgSet(bgP0,bgP1,bgP2,bgP3,bgP4));

      // signal+bg
      RooAddPdf model("model","model",
          RooArgList(pi0model,bgModel),
          RooArgList(pi0n,bgN)
          );

      // fit range
      mass.setRange("fitRange", 0.08, 0.4);

      // perform fit
      modelFit = model.fitTo(
          massdist,
          RooFit::Range("fitRange"),
          RooFit::Extended(kTRUE),
          RooFit::Save(kTRUE)
          );

      // plot
      plotFrame = mass.frame(
          RooFit::Title("M_{#gamma#gamma} distribution")
          );
      massdist.plotOn(plotFrame,
          RooFit::Name("massdistPlot"),
          RooFit::LineWidth(0),
          RooFit::LineColor(kWhite), /* invisible (to set a frame) */
          RooFit::XErrorSize(0)
          );
      model.plotOn(plotFrame,
          RooFit::Name("modelPlot"),
          RooFit::Range("fitRange"),
          RooFit::LineColor(kBlack),
          RooFit::LineWidth(4)
          );
      model.plotOn(plotFrame,
          RooFit::Name("pi0modelPlot"),
          RooFit::Components("pi0model"),
          RooFit::Range("fitRange"),
          RooFit::LineColor(kCyan+1),
          RooFit::LineWidth(4)
          );
      model.plotOn(plotFrame,
          RooFit::Name("bgModelPlot"),
          RooFit::Components("bgModel"),
          RooFit::Range("fitRange"),
          RooFit::LineColor(kGray+1),
          RooFit::LineWidth(4),
          RooFit::LineStyle(9)
          );
      fitCanv = new TCanvas(
          Form("fitCanv_%d",binnum),Form("fitCanv_%d",binnum),
          1200,800);
      plotFrame->Draw();
      Mdist->SetMarkerStyle(kFullCircle);
      Mdist->SetMarkerSize(1);
      Mdist->SetMarkerColor(kBlack);
      Mdist->Draw("EPSAME");
    };

};


////////////////////////////////////////
////////////////////////////////////////
////////////////////////////////////////
////////////////////////////////////////


int main(int argc, char** argv) {

  // ARGUMENTS
  if(argc<=2) {
    cerr << "USAGE: " << argv[0] << " [tree file] [IV type] [num bins]" << endl;
    BS = new Binning();
    cout << " - IV types:" << endl
         << "    1, 2, or 3 digit number specifying binning scheme;" << endl
         << "    available vars:" << endl;
    for(int i=0; i<Binning::nIV; i++)
      cout << "     " << i+1 << " = " << BS->IVtitle[i] << endl;
    cout << " - num bins: number of bins, listed for each IV," << endl
         << "   separated by spaces (default is single bin)" << endl;
    return 1;
  };
  infileN = TString(argv[1]);
  ivType = (Int_t)strtof(argv[2],NULL);
  for(int i=0; i<3; i++) numBin[i]=-1;
  if(argc>3) {
    int ab=0;
    for(int ar=3; ar<argc; ar++)
      numBin[ab++] = (Int_t)strtof(argv[ar],NULL);
  };
  cout << "infileN = " << infileN << endl
       << "ivType = " << ivType << endl
       << "numBin = [" << numBin[0] << ","
                       << numBin[1] << ","
                       << numBin[2] << "]" << endl;


  // set binning scheme
  BS = new Binning();
  BS->SetScheme(ivType,numBin[0],numBin[1],numBin[2]);


  // open TFiles
  infile = new TFile(infileN,"READ");
  outfileN = infileN;
  outfileN(TRegexp("cat__")) = "fit__";
  outfile = new TFile(outfileN,"RECREATE");


  // open tree
  Double_t ivVal[3] = {0,0,0};
  Double_t Mgg;
  tr = (TTree*)infile->Get("diphTr");
  for(int d=0; d<BS->dimensions; d++) {
    cout << "IV " << d << " is " << BS->GetIVname(d) << endl;
    tr->SetBranchAddress(BS->GetIVname(d),&ivVal[d]);
  };
  tr->SetBranchAddress("diphM",&Mgg);


  // create FitBin object for each bin
  map<Int_t,FitBin*> FitBinList;
  for(Int_t bn : BS->binVec) {
    FitBinList.insert(pair<Int_t,FitBin*>(bn,new FitBin(bn)));
  };


  // loop through tree
  FitBin *fb;
  ENT = tr->GetEntries();
  for(Long64_t e=0; e<ENT; e++) {
    if(e%10000==0) printf("[+] %.2f%%\n",100*(float)e/((float)ENT));
    tr->GetEntry(e);

    // find bin, and set FitBin pointer
    try {
      fb = FitBinList.at(BS->FindBin(ivVal[0],ivVal[1],ivVal[2]));
    } catch (const std::out_of_range &ex) {
      cerr << "ERROR: cannot find bin" << endl;
      continue;
    };

    // fill Mgg dist
    fb->Mdist->Fill(Mgg);
  };

  // fit histograms
  for(Int_t bn : BS->binVec) FitBinList.at(bn)->Fit();

  // write
  outfile->cd();
  for(Int_t bn : BS->binVec) {
    FitBinList.at(bn)->Mdist->Write();
    FitBinList.at(bn)->fitCanv->Write();
  };


  // cleanup
  outfile->Close();
  infile->Close(); 
  return 0;
};
