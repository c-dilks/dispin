#include <cstdlib>
#include <iostream>

// ROOT
#include "TFile.h"
#include "TString.h"
#include "TMath.h"
#include "TSystem.h"
#include "TROOT.h"
#include "TRegexp.h"
#include "TH1.h"
#include "TCanvas.h"
#include "TGraph.h"

// Dispin
#include "Constants.h"
#include "Tools.h"
#include "Ensemble.h"
#include "EventTree.h"

using std::cout;
using std::cerr;
using std::endl;
using std::pair;

TString infileN;

int main(int argc, char** argv) {

  // ARGUMENTS
  if(argc>1) infileN = TString(argv[1]);
  else {
    fprintf(stderr,"USAGE: pi0analyzer.cpp outrootFile.root\n");
    return 1;
  };

  // instantiate tree reading classes
  Ensemble *ens = new Ensemble(infileN);
  EventTree *ev = new EventTree(
    infileN,EncodePairType(kPhoton,kPhoton)
  );


  // output file
  TString outfileN = infileN;
  outfileN(TRegexp("^.*/")) = "";
  outfileN = "pi0diag/" + outfileN;
  cout << "outfileN = " << outfileN << endl;
  gROOT->ProcessLine(".! mkdir -p pi0diag");
  TFile *outfile = new TFile(outfileN,"RECREATE");


  // histograms ///////////////////////////////////
  TH1D * Mdist = new TH1D("Mdist",
    "M_{#gamma#gamma} distribution;M_{#gamma#gamma} [GeV]",300,0,1);


  // ensemble loop
  Int_t hadRowEns[2];
  Long_t nEns = 0;
  while(ens->NextEvent()) {

    // status and limiiter
    if(++nEns%1000==0)
      cout << nEns << " ensembles analyzed" << endl;
    if(nEns>10000) break; // limiter
     
    // loop through diphotons
    for(Long64_t di : ens->GetDiphotonList()) {

      // get diphoton kinematics
      ev->GetEvent(di);

      // sanity cross check
      ens->GetHadRow(di,hadRowEns[qA],hadRowEns[qB]);
      if( ev->evnum!=ens->GetEvnum() ||
          ev->hadRow[qA]!=hadRowEns[qA] ||
          ev->hadRow[qB]!=hadRowEns[qB] ) {
        fprintf(stderr,"ERROR: sanity check failed\n");
        return 1;
      };

      // fill histograms
      Mdist->Fill(ev->Mh);

    };

  };

  // write to outfile
  Mdist->Write();
  
  outfile->Close();



  return 0;
};

