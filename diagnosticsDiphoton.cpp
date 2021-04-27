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
#include "Diphoton.h"
#include "DIS.h"

using std::cout;
using std::cerr;
using std::endl;
using std::pair;

TString infileN;
Int_t dataState;
EventTree *ev;
Ensemble *ens;
Diphoton *diphot;
DIS *disEv;
const Int_t nbins = 300;


// histograms class
class Histos {
  public:
    TH1D *MDist;
    TH1D *EDist;
    TH1D *PtDist;
    TH1D *EtaDist;
    TH1D *PhiDist;
    TH1D *ZEDist;
    TH1D *VtxDiffDist;

    Histos() {
      MDist = new TH1D("MDist",
        "M_{#gamma#gamma} Distribution;M_{#gamma#gamma} [GeV]",
        nbins,0,1);
      EDist = new TH1D("EDist",
        "E_{#gamma#gamma} Distribution;E_{#gamma#gamma} [GeV]",
        nbins,0,12);
      PtDist = new TH1D("PtDist",
        "p_{T,#gamma#gamma}^{lab} Distribution;p_{T,#gamma#gamma}^{lab} [GeV]",
        nbins,0,6);
      EtaDist = new TH1D("EtaDist",
        "#eta_{#gamma#gamma} Distribution;#eta_{#gamma#gamma}",
        nbins,0,6);
      PhiDist = new TH1D("PhiDist",
        "#phi_{#gamma#gamma} Distribution;#phi_{#gamma#gamma}",
        nbins,-PIe,PIe);
      ZEDist = new TH1D("ZEDist",
        "Z_{E,#gamma#gamma} Distribution;Z_{E,#gamma#gamma}",
        nbins,0,1);
      VtxDiffDist = new TH1D("VtxDiffDist",
        "|Vtx(#gamma_1)-Vtx(#gamma2)| Distribution;|Vtx(#gamma_1)-Vtx(#gamma2)|",
        nbins,-10,10);
    };

    void FillHistograms() {
      MDist->Fill(diphot->M);
      EDist->Fill(diphot->E);
      PtDist->Fill(diphot->Pt);
      EtaDist->Fill(diphot->Eta);
      PhiDist->Fill(diphot->Phi);
      ZEDist->Fill(diphot->ZE);
      VtxDiffDist->Fill(diphot->VtxDiff);
    };

    void WriteHistograms(TFile *tf) {
      tf->cd();
      MDist->Write();
      EDist->Write();
      PtDist->Write();
      EtaDist->Write();
      PhiDist->Write();
      ZEDist->Write();
      VtxDiffDist->Write();
    };
      
};



////////////////////////////////////////
int main(int argc, char** argv) {

  // ARGUMENTS
  infileN = "outroot.root";
  dataState = 0; // 0=notRePaired, 1=RePaired
  if(argc==1) {
    fprintf(stderr,"USAGE: pi0analyzer.cpp [outrootFile.root] [dataState]\n");
    fprintf(stderr,"       -dataState=0: not RePaired; one histogram entry = one unique diphoton\n");
    fprintf(stderr,"       -dataState=1: RePaired; one histogram entry = a diphoton from a unique pair(hadron,diphoton)\n");
    return 1;
  };
  if(argc>1) infileN = TString(argv[1]);
  if(argc>2) dataState = (Int_t)strtof(argv[2],NULL);


  // instantiations
  ens = new Ensemble(infileN);
  ev = new EventTree(
    infileN,EncodePairType(kPhoton,kPhoton)
  );
  diphot = new Diphoton();
  disEv = new DIS();


  // output file
  TString outfileN = infileN;
  outfileN(TRegexp("^.*/")) = "";
  outfileN = "diagDiphot/" + outfileN;
  cout << "outfileN = " << outfileN << endl;
  gROOT->ProcessLine(".! mkdir -p diagDiphot");
  TFile *outfile = new TFile(outfileN,"RECREATE");


  Histos *hists = new Histos();

  // dataState 0 -- RePair has not yet been done, so diphotons are
  // in their own entries, as photon-photon "dihadrons"; we can thus
  // iterate through these entries and fill histograms; to do this,
  // we use Ensemble::NextEvent() to loop over event ensembles
  if(dataState==0) {
    Int_t hadRowEns[2];
    Long_t nEns = 0;
    while(ens->NextEvent()) {
      if(ens->GetEnum()>1000) break; // limiter
       
      // loop through diphotons
      for(Long64_t di : ens->GetDiphotonList()) {

        // only need trajectories, for diphotons
        ev->GetTrajectories(di);

        // sanity cross check between Ensemble and EventTree
        ens->GetHadRow(di,hadRowEns[qA],hadRowEns[qB]);
        if( ev->evnum!=ens->GetEvnum() ||
            ev->hadRow[qA]!=hadRowEns[qA] ||
            ev->hadRow[qB]!=hadRowEns[qB] ) {
          fprintf(stderr,"ERROR: sanity check failed\n");
          return 1;
        };

        // need DIS kinematics
        disEv->CalculateKinematics(
          ev->GetElectronTraj(),
          ev->runnum
        );

        // calculate diphoton kinematics
        diphot->CalculateKinematics(
          ev->GetHadronTraj(qA),
          ev->GetHadronTraj(qB),
          disEv
        );

        // fill histograms
        hists->FillHistograms();

      };

    };
  }

  // dataState 1 -- RePair has been done, meaning diphotons are
  // paired with hadrons; here, we loop through EventTree; the same
  // diphoton may be seen several times per event, since it will
  // be paired with all the other hadrons of that event
  else if(dataState==1) {
    for(int i=0; i<ev->ENT; i++) {
      if(i>10000) break; // limiter

      // get the whole event, so we can use EventTree::Valid() later 
      // if we want to; this will also give us Diphoton pointers, 
      // if there is one
      ev->GetEvent(i);

      if(true) { // TODO: add pairType argument so we can use EventTree::Valid() here

        // loop over the 2 hadrons in the dihadron
        for(int h=0; h<2; h++) {
          if(ev->hadIdx[h]==kDiph) {
            diphot = ev->objDiphoton;
            hists->FillHistograms();
          };
        };
      };
    };
  }

  else {
    cerr << "ERROR: bad dataState" << endl;
    return 1;
  };



  // write to outfile
  hists->WriteHistograms(outfile);
  outfile->Close();



  return 0;
};
