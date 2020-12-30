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


  // define RooDataSet
  RooRealVar runnum("runnum","runnum",0);
  RooRealVar evnum("evnum","evnum",0);
  RooRealVar PhiH("PhiH","#phi_{h}",-PIe,PIe);
  RooRealVar PhiR("PhiR","#phi_{R}",-PIe,PIe);
  RooRealVar PhiD("PhiD","#Delta#phi",-PIe,PIe);
  RooRealVar Theta("Theta","#theta",-0.1,PIe);
  RooRealVar Mh("Mh","M_{h}",0,3);
  RooRealVar x("x","x",0,1);
  RooRealVar z("z","z",0,1);
  RooRealVar yhA("yhA","y_{h}^{A}",-4,4);
  RooRealVar yhB("yhB","y_{h}^{B}",-4,4);
  RooRealVar PhPerp("PhPerp","P_{h}^{perp}",0,2);
  RooRealVar Pol("Pol","P",0,1);
  RooRealVar Spin("Spin","s",0,1);
  RooArgSet * rooVars = new RooArgSet();

  rooVars->add(runnum);
  rooVars->add(evnum);
  rooVars->add(PhiH);
  rooVars->add(PhiR);
  rooVars->add(PhiD);
  rooVars->add(Theta);
  rooVars->add(Mh);
  rooVars->add(x);
  rooVars->add(z);
  rooVars->add(yhA);
  rooVars->add(yhB);
  rooVars->add(PhPerp);
  rooVars->add(Pol);
  rooVars->add(Spin);

  RooDataSet * rooData = new RooDataSet("rooData","rooData",*rooVars);
    


  //-----------------------------------------------------
  // EVENT LOOP  
  //-----------------------------------------------------
  Double_t evCount = 0;
  printf("begin loop through %lld events...\n",ev->ENT);
  for(int i=0; i<ev->ENT; i++) {
    ev->GetEvent(i);
    if(ev->Valid()) {

      runnum.setVal(ev->runnum);
      evnum.setVal(ev->evnum);
      PhiH.setVal(ev->PhiH);
      PhiR.setVal(ev->PhiR);
      PhiD.setVal(ev->PhiD);
      Theta.setVal(ev->theta);
      Mh.setVal(ev->Mh);
      x.setVal(ev->x);
      z.setVal(ev->Zpair);
      yhA.setVal(ev->hadYH[qA]);
      yhB.setVal(ev->hadYH[qB]);
      PhPerp.setVal(ev->PhPerp);
      Pol.setVal(ev->Polarization());
      Spin.setVal(ev->SpinState());

      rooData->add(*rooVars);

      evCount+=1;
    };
  }; // end event loop


  // write output
  rooData->Write("rooData");
  //rooData->write("roo.txt"); // dump to text file
  printf("\n%s written\n\n",roofileN.Data());
  printf("evCount = %.0f\n",evCount);
  roofile->Close();
};
