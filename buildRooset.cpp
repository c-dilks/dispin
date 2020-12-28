// build RooDataSet with the a minimal set of variables for the asymmetry fit;
// event selection criteria are applied

#include <cstdlib>
#include <iostream>
#include <vector>
#include <map>

// ROOT
#include "TFile.h"
#include "TString.h"
#include "TRegexp.h"

// RooFit
#include <RooGlobalFunc.h>
#include <RooGenericPdf.h>
#include <RooExtendPdf.h>
#include <RooAbsReal.h>
#include <RooArgSet.h>
#include <RooDataSet.h>
#include <RooRealVar.h>
#include <RooStats/SPlot.h>

// Dispin
#include "Constants.h"
#include "EventTree.h"


TString infileN;
Int_t whichPair;


//////////////////////////////////////


int main(int argc, char** argv) {

  // ARGUMENTS
  infileN = "outroot/skim4_005052.hipo.root";
  whichPair = EncodePairType(kPip,kPim);
  if(argc>1) infileN = TString(argv[1]);
  if(argc>2) whichPair = (Int_t)strtof(argv[2],NULL);

  // define event tree
  printf("infileN = %s\n",infileN.Data());
  printf("whichPair = 0x%x\n",whichPair);
  EventTree * ev = new EventTree(infileN,whichPair);

  // define output rooset file
  TString roofileN = infileN;
  roofileN(TRegexp("^.*/")) = "rooset/roo.";
  printf("roofile = %s\n",roofileN.Data());
  TFile * roofile = new TFile(roofileN,"RECREATE");


  // define RooDataSet (ensure var names match those in Asymmetry.cxx)
  RooRealVar rfPhiH("rfPhiH","#phi_{h}",-PIe,PIe);
  RooRealVar rfPhiR("rfPhiR","#phi_{R}",-PIe,PIe);
  RooRealVar rfPhiD("rfPhiD","#Delta#phi",-PIe,PIe);
  RooRealVar rfTheta("rfTheta","#theta",-0.1,PIe);
  RooRealVar rfMh("rfMh","M_{h}",0,3);
  RooRealVar rfx("rfx","x",0,1);
  RooRealVar rfz("rfz","z",0,1);
  RooRealVar rfyhA("rfyhA","y_{h}^{A}",-4,4);
  RooRealVar rfyhB("rfyhB","y_{h}^{B}",-4,4);
  RooRealVar rfPhPerp("rfPhPerp","P_{h}^{perp}",0,2);
  RooRealVar rfPol("rfPol","P",0,1);
  RooRealVar rfSpin("rfSpin","s",0,1);
  RooArgSet * rooVars = new RooArgSet();

  rooVars->add(rfPhiH);
  rooVars->add(rfPhiR);
  rooVars->add(rfPhiD);
  rooVars->add(rfTheta);
  rooVars->add(rfMh);
  rooVars->add(rfx);
  rooVars->add(rfz);
  rooVars->add(rfyhA);
  rooVars->add(rfyhB);
  rooVars->add(rfPhPerp);
  rooVars->add(rfPol);
  rooVars->add(rfSpin);

  RooDataSet * rooData = new RooDataSet("rooData","rooData",*rooVars);
    


  //-----------------------------------------------------
  // EVENT LOOP  
  //-----------------------------------------------------
  Double_t evCount = 0;
  printf("begin loop through %lld events...\n",ev->ENT);
  for(int i=0; i<ev->ENT; i++) {
    ev->GetEvent(i);
    if(ev->Valid()) {

      rfPhiH.setVal(ev->PhiH);
      rfPhiR.setVal(ev->PhiR);
      rfPhiD.setVal(ev->PhiD);
      rfTheta.setVal(ev->theta);
      rfMh.setVal(ev->Mh);
      rfx.setVal(ev->x);
      rfz.setVal(ev->Zpair);
      rfyhA.setVal(ev->hadYH[qA]);
      rfyhB.setVal(ev->hadYH[qB]);
      rfPhPerp.setVal(ev->PhPerp);
      rfPol.setVal(ev->Polarization());
      rfSpin.setVal(ev->SpinState());

      rooData->add(*rooVars);

      evCount+=1;
    };
  }; // end event loop

  
  // append splot weights (if PDF MhFit file is found)
  Bool_t pdfExists = ! gSystem->AccessPathName("mfit.root");
  //pdfExists = false; // override
  RooStats::SPlot * rooSplot;
  RooAbsPdf * rooPdf;
  TFile * mfitFile;
  RooRealVar * resRhoN;
  RooRealVar * resF0N;
  RooRealVar * resF2N;
  RooRealVar * bgN;
  if(pdfExists) {
    mfitFile = new TFile("mfit.root","READ");
    rooPdf = (RooAbsPdf*) mfitFile->Get("mfitPdf");
    resRhoN = (RooRealVar*) mfitFile->Get("resRhoN");
    resF0N = (RooRealVar*) mfitFile->Get("resF0N");
    resF2N = (RooRealVar*) mfitFile->Get("resF2N");
    bgN = (RooRealVar*) mfitFile->Get("bgN");
    roofile->cd();
    printf("BEGIN SPLOT\n");
    rooSplot = new RooStats::SPlot(
      "rooSplot","rooSplot",
      *rooData,
      rooPdf,
      RooArgList(*resRhoN,*resF0N,*resF2N,*bgN)
    );
    printf("DONE SPLOT\n");
  };


  // write output
  rooData->Write("rooData");
  rooData->write("roo.txt"); // dump to text file
  if(pdfExists) {
    rooSplot->Write("rooSplot");
    rooSplot->GetSDataSet()->write("rooSplot.txt");
  };
  printf("\n%s written\n\n",roofileN.Data());
  printf("evCount = %.0f\n",evCount);
  roofile->Close();
  if(pdfExists) mfitFile->Close();
};
