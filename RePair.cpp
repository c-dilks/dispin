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

  Ensemble *ens = new Ensemble(infile);

  Long64_t hi;
  Int_t qh;
  while(ens->NextEvent()) {
     
    // loop through diphotons
    cout << "event " << ens->GetEvnum() << endl;
    for(Long64_t di : ens->GetDiphotonList()) {
      cout << "  di = " << di << endl;
    };

    // loop through dihadrons
    cout << "  nhad = " << ens->GetHadronList().size() << endl;
    for(auto hadUID : ens->GetHadronList()) {
      hi = hadUID.first;
      qh = hadUID.second;
      cout << "  had = " << hi << ", " << qh << endl;
    };
  };


  return 0;
};


