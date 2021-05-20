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
#include "TTree.h"

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
Int_t cutState;
Int_t pairType;
EventTree *ev;
Ensemble *ens;
Diphoton *diphot;
DIS *disEv;
const Int_t nbins = 300;
TTree * diphTr;


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
        "|Vtx(#gamma_{1})-Vtx(#gamma_{2})| Distribution;|Vtx(#gamma_{1})-Vtx(#gamma_{2})|",
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

    void Warn() {
      if(MDist->GetEntries()<1) 
        fprintf(stderr,"WARNING: histograms are empty; is dataState wrong?\n");
    };
      
};



////////////////////////////////////////
int main(int argc, char** argv) {

  // ARGUMENTS
  infileN = "outroot.root";
  dataState = 0;
  cutState = -1;
  if(argc==1) {
    fprintf(stderr,"USAGE: %s [outrootFile.root] [dataState] [cutState]\n",argv[0]);
    fprintf(stderr,"\n");
    fprintf(stderr," -dataState = 0: not RePaired\n");
    fprintf(stderr,"  --- one histogram entry = one unique diphoton\n");
    fprintf(stderr,"  --- N.B.: some events may have only diphotons, and\n");
    fprintf(stderr,"            therefore may not be in the RePaired tree\n");
    fprintf(stderr,"   -cutState = -1 (default): any diphoton\n");
    fprintf(stderr,"   -cutState non-negative: pass diphoton cuts\n");
    fprintf(stderr,"     -cutState in [0,10): classify diphoton\n");
    fprintf(stderr,"       -cutState = %d: pi0\n",Diphoton::dpPi0);
    fprintf(stderr,"       -cutState = %d: pi0 BG (sideband)\n",Diphoton::dpSB);
    fprintf(stderr,"       -cutState = %d: neither pi0 nor sideband\n",Diphoton::dpIgnore);
    fprintf(stderr,"     -cutState = 10: do not classify\n");
    fprintf(stderr,"\n");
    fprintf(stderr," -dataState = 1: RePaired\n");
    fprintf(stderr,"  --- one histogram entry = a diphoton from a unique pair(hadron,diphoton)\n");
    fprintf(stderr,"   -cutState = -1 (default): any diphoton, do not use EventTree::Valid() cuts\n");
    fprintf(stderr,"   -cutState >= 0: interpret this as pairType, and apply EventTree::Valid()\n");
    fprintf(stderr,"    cuts; the diphoton will be classified, and pairType will filter for the\n");
    fprintf(stderr,"    class you want\n");
    fprintf(stderr,"   -N.B. if you want to see the full M_gg spectrum, with EventTree::Valid()\n");
    fprintf(stderr,"         cuts and diphoton basic cuts enabled, use kDiphBasic in pairType;\n");
    fprintf(stderr,"         this spectrum is what should be fitted for pi0 signal purity\n");
    fprintf(stderr,"\n");
    fprintf(stderr,"         EXAMPLE for (pi+,diphoton) M_gg spectrum:\n");
    fprintf(stderr,"           %s 1 0x%x\n",argv[0],EncodePairType(kPip,kDiphBasic));
    fprintf(stderr,"\n\n");
    return 1;
  };
  if(argc>1) infileN = TString(argv[1]);
  if(argc>2) dataState = (Int_t)strtof(argv[2],NULL);
  if(argc>3) cutState = (Int_t)strtof(argv[3],NULL);


  // instantiations
  pairType = dataState==1 && cutState>=0 ?
    cutState : EncodePairType(kPhoton,kPhoton);
  ev = new EventTree(infileN,pairType);
  ens = new Ensemble(infileN);
  diphot = new Diphoton();
  disEv = new DIS();


  // output file
  TString outfileN = infileN;
  outfileN(TRegexp("^.*/")) = "";
  outfileN = "diagdiph/set/" + outfileN;
  cout << "outfileN = " << outfileN << endl;
  gROOT->ProcessLine(".! mkdir -p diagdiph/set");
  TFile *outfile = new TFile(outfileN,"RECREATE");


  // output tree, to be used for binning Mh distributions for fits
  Double_t X_tr;
  Double_t Mh_tr;
  Double_t Z_tr;
  Double_t PhPerp_tr;
  Double_t DY_tr;
  Double_t Q2_tr;
  Double_t XF_tr;
  Double_t diphM_tr;
  Double_t diphE_tr;
  Double_t diphPt_tr;
  Double_t diphEta_tr;
  Double_t diphPhi_tr;
  Double_t diphZE_tr;
  Double_t diphVtxDiff_tr;
  if(dataState==1) {
    diphTr = new TTree("diphTr","diphTr");
    // event level
    diphTr->Branch("runnum",&(ev->runnum),"runnum/I");
    diphTr->Branch("evnum",&(ev->evnum),"evnum/I");
    // Binning IVs (branch names match catTree tree from
    // Asymmetry::ActivateTree); they must be converted
    // to doubles, since practically everything in EventTree
    // is floats...
    diphTr->Branch("X",&X_tr,"X/D");
    diphTr->Branch("Mh",&Mh_tr,"Mh/D");
    diphTr->Branch("Z",&Z_tr,"Z/D");
    diphTr->Branch("PhPerp",&PhPerp_tr,"PhPerp/D");
    diphTr->Branch("DY",&DY_tr,"DY/D");
    diphTr->Branch("Q2",&Q2_tr,"Q2/D");
    diphTr->Branch("XF",&XF_tr,"XF/D");
    // diphoton vars
    diphTr->Branch("diphM",&diphM_tr,"diphM/D");
    diphTr->Branch("diphE",&diphE_tr,"diphE/D");
    diphTr->Branch("diphPt",&diphPt_tr,"diphPt/D");
    diphTr->Branch("diphEta",&diphEta_tr,"diphEta/D");
    diphTr->Branch("diphPhi",&diphPhi_tr,"diphPhi/D");
    diphTr->Branch("diphZE",&diphZE_tr,"diphZE/D");
    diphTr->Branch("diphVtxDiff",&diphVtxDiff_tr,"diphVtxDiff/D");
  };

  Histos *hists = new Histos();
  Bool_t validCut,diphotCut;

  // dataState 0 -- RePair has not yet been done, so diphotons are
  // in their own entries, as photon-photon "dihadrons"; we can thus
  // iterate through these entries and fill histograms; to do this,
  // we use Ensemble::NextEvent() to loop over event ensembles
  if(dataState==0) {
    Int_t hadRowEns[2];
    Long_t nEns = 0;
    while(ens->NextEvent()) {
      //if(ens->GetEnum()>1000) break; // limiter
       
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

        // classify diphoton, and check cuts
        if(cutState<0) { // don't classify or check basic cuts
          diphotCut=true;
        } else if(cutState<10) { // classify and check basic cuts
          diphot->Classify();
          diphotCut = (cutState == diphot->diphotClass);
        } else if(cutState==10) { // just check basic cuts
          diphot->Classify();
          diphotCut = diphot->cutBasic;
        } else {
          fprintf(stderr,"ERROR: bad cutState\n");
          return 1;
        };

        // if this event ensemble had only diphotons, and no hadrons, then
        // these diphotons will not appear in the RePaired tree (unless
        // diphotons are paired with other diphotons); such diphotons may have
        // a different distribution than other diphotons; use this line to
        // filter out such events:
        //if(ens->GetDihadronList().size()==0) continue;

        // fill histograms
        if(diphotCut) {
          hists->FillHistograms();
        };

      };

    };
  }

  // dataState 1 -- RePair has been done, meaning diphotons are
  // paired with hadrons; here, we loop through EventTree; the same
  // diphoton may be seen several times per event, since it will
  // be paired with all the other hadrons of that event
  else if(dataState==1) {
    for(int i=0; i<ev->ENT; i++) {
      //if(i>50000) break; // limiter

      // get the whole event, so we can use EventTree::Valid() later 
      // if we want to; this will also give us Diphoton pointers, 
      // if there is one
      ev->GetEvent(i);

      // check event selection cuts
      validCut = cutState>=0 ? ev->Valid() : true;
      if(validCut) {

        // loop over the 2 hadrons in the dihadron
        for(int h=0; h<2; h++) {

          // is this hadron a diphoton?
          diphotCut = ev->hadIdx[h]==kDiph
                   || ev->hadIdx[h]==kPio
                   || ev->hadIdx[h]==kPioBG
                   || ev->hadIdx[h]==kDiphBasic;

          // if it is a diphoton, fill histograms and output tree
          if(diphotCut) {

            // fill histograms
            diphot = ev->objDiphoton;
            hists->FillHistograms();

            // typecast floats to doubles (for compatability with catTree)
            X_tr = (Double_t) ev->x;
            Mh_tr = (Double_t) ev->Mh;
            Z_tr = (Double_t) ev->Zpair;
            PhPerp_tr = (Double_t) ev->PhPerp;
            DY_tr = (Double_t) ev->DY;
            Q2_tr = (Double_t) ev->Q2;
            XF_tr = (Double_t) ev->xF;
            diphM_tr = (Double_t) diphot->M;
            diphE_tr = (Double_t) diphot->E;
            diphPt_tr = (Double_t) diphot->Pt;
            diphEta_tr = (Double_t) diphot->Eta;
            diphPhi_tr = (Double_t) diphot->Phi;
            diphZE_tr = (Double_t) diphot->ZE;
            diphVtxDiff_tr = (Double_t) diphot->VtxDiff;

            // fill tree
            diphTr->Fill();
          };
        };
      };
    };
  }

  else {
    cerr << "ERROR: bad dataState" << endl;
    return 1;
  };

  // print warning, if histograms are empty
  hists->Warn();

  // write to outfile
  if(dataState==1) diphTr->Write();
  hists->WriteHistograms(outfile);
  outfile->Close();


  return 0;
};
