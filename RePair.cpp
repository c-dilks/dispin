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
#include "EventTree.h"
#include "DIS.h"
#include "Dihadron.h"
#include "Diphoton.h"

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

  Ensemble *ens = new Ensemble(infileN);
  EventTree *ev = new EventTree(infileN);
  Diphoton *diphot = new Diphoton();
  Dihadron *dihad = new Dihadron();
  DIS *disEv = new DIS();

  Long64_t hi;
  Int_t qh;
  while(ens->NextEvent()) {

    // get DIS event, and calculate kinematics
    ev->GetTrajectories(ens->GetEnum());
    disEv->CalculateKinematics(
      ev->GetElectronTraj(),
      ev->runnum
    );


    // loop over dihadrons
    for(Long64_t di : ens->GetDihadronList()) {

      // calculate dihadron kinematics
      ev->GetTrajectories(di);
      dihad->CalculateKinematics(
        ev->GetHadronTraj(qA),
        ev->GetHadronTraj(qB),
        disEv
      );

      /* FILL */
    }; // end dihadron loop
     

    // loop over diphotons, pairing each with each hadron
    cout << "event " << ens->GetEvnum() << endl;
    for(Long64_t di : ens->GetDiphotonList()) {
      cout << "  di = " << di << endl;

      // calculate diphoton kinematics
      ev->GetTrajectories(di);
      diphot->CalculateKinematics(
        ev->GetHadronTraj(qA),
        ev->GetHadronTraj(qB),
        disEv
      );
      
      // loop over hadrons
      cout << "  nhad = " << ens->GetHadronList().size() << endl;
      for(auto hadUID : ens->GetHadronList()) {
        hi = hadUID.first;
        qh = hadUID.second;
        cout << "  had = " << hi << ", " << qh << endl;

        // pair hadron with diphoton
        ev->GetTrajectories(hi);
        if(CorrectOrder(
          ev->GetHadronTraj(qh)->Idx,
          diphot->GetDiphotonTraj()->Idx
        )) {
          dihad->CalculateKinematics(
            ev->GetHadronTraj(qh),
            diphot->GetDiphotonTraj(),
            disEv
          );
        } else {
          dihad->CalculateKinematics(
            diphot->GetDiphotonTraj(),
            ev->GetHadronTraj(qh),
            disEv
          );
        };

        /* FILL */

      }; // end hadron loop
    }; // end diphoton loop

  };


  return 0;
};


