#include <cstdlib>
#include <iostream>

// ROOT
#include "TFile.h"
#include "TString.h"
#include "TMath.h"
#include "TSystem.h"
#include "TRegexp.h"
#include "TH1.h"
#include "TCanvas.h"
#include "TGraph.h"

// Dispin
#include "Constants.h"
#include "Tools.h"
#include "Ensemble.h"

using std::cout;
using std::cerr;
using std::endl;
using std::pair;

TString infile;

int main(int argc, char** argv) {

  // ARGUMENTS
  if(argc>1) infile = TString(argv[1]);
  else {
    fprintf(stderr,"USAGE: pi0analyzer.cpp outrootFile.root\n");
    return 1;
  };

  // instantiate tree reading classes
  Ensemble *ens = new Ensemble(infile);
  EventTree *ev = new EventTree(infile);


  // histograms ///////////////////////////////////
  TH1D * Mdist = new TH1D("Mdist",
    "M_{#gamma#gamma} distribution;M_{#gamma#gamma} [GeV]",100,0,1);


  // ensemble loop
  Int_t hadRowEns[2];
  while(ens->NextEvent()) {
     
    // loop through diphotons
    cout << "event " << ens->GetEvnum() << endl;
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



    };

  };


  return 0;
};

