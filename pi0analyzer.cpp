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

TString infiles;

int main(int argc, char** argv) {

  // ARGUMENTS
  infiles = "outroot/*.root";
  if(argc>1) infiles = TString(argv[1]);

  Ensemble *ens = new Ensemble(infiles);

  while(ens->NextEvent()) {
     
    // loop through diphotons
    cout << "event " << ens->evnum << endl;
    for(Long64_t di : ens->GetDiphotList()) {
      cout << "  di = " << di << endl;
    };

  };


  return 0;
};

