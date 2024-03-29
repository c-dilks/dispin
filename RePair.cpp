#include <cstdlib>
#include <iostream>

// ROOT
#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TMath.h"
#include "TSystem.h"
#include "TRegexp.h"
#include "TH1.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TROOT.h"

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
TString outdirN;
Bool_t debug;

int main(int argc, char** argv) {

  // ARGUMENTS
  if(argc-1==2) {
    infileN = TString(argv[1]);
    outdirN = TString(argv[2]);
  } else {
    fprintf(stderr,"USAGE: RePair.cpp [outrootFile.root] [outputDir]\n");
    return 1;
  };

  // OPTIONS
  debug = false; // print verbose


  // instantiation
  Ensemble *ens = new Ensemble(infileN);
  EventTree *ev = new EventTree(infileN);
  Diphoton *diphot = new Diphoton();
  Dihadron *dihad = new Dihadron();
  DIS *disEv = new DIS();


  // set up output file
  TString outfileN = infileN;
  outfileN(TRegexp("^.*/")) = outdirN+"/";
  cout << "outfileN = " << outfileN << endl;
  gROOT->ProcessLine(".! mkdir -p "+outdirN);
  TFile *outfile = new TFile(outfileN,"RECREATE");


  // define new tree
  // - copy branches from `calcKinematics.cpp`
  // - where possible, we keep the branch addresses the same
  // - some branches, e.g., fiducial cuts, are now just copied from EventTree;
  //   such branches are found by searching for `Banch.*\<ev\>`
  // - a few additional branches are defined for photons
  TTree *outTr = new TTree("tree","tree");
  // - DIS kinematics branches
  outTr->Branch("W",&(disEv->W),"W/F");
  outTr->Branch("Q2",&(disEv->Q2),"Q2/F");
  outTr->Branch("Nu",&(disEv->Nu),"Nu/F");
  outTr->Branch("x",&(disEv->x),"x/F");
  outTr->Branch("y",&(disEv->y),"y/F");
  outTr->Branch("beamE",&(disEv->BeamEn),"beamE/F");
  // - electron kinematics branches
  outTr->Branch("eleE",&(disEv->eleE),"eleE/F");
  outTr->Branch("eleP",&(disEv->eleP),"eleP/F");
  outTr->Branch("elePt",&(disEv->elePt),"elePt/F");
  outTr->Branch("eleEta",&(disEv->eleEta),"eleEta/F");
  outTr->Branch("elePhi",&(disEv->elePhi),"elePhi/F");
  outTr->Branch("eleVertex",disEv->eleVertex,"eleVertex[3]/F");
  outTr->Branch("eleStatus",&(disEv->eleStatus),"eleStatus/I");
  outTr->Branch("eleChi2pid",&(disEv->eleChi2pid),"eleChi2pid/F");
  outTr->Branch("eleFiduCut",&(ev->eleFiduCut),"eleFiduCut/O");
  outTr->Branch("elePCALen",&(ev->elePCALen),"elePCALen/F");
  outTr->Branch("eleECINen",&(ev->eleECINen),"eleECINen/F");
  outTr->Branch("eleECOUTen",&(ev->eleECOUTen),"eleECOUTen/F");
  outTr->Branch("eleSector",&(ev->eleSector),"eleSector/I");
  // - hadron branches
  outTr->Branch("pairType",&(dihad->pairType),"pairType/I");
  outTr->Branch("hadRow",dihad->hadRow,"hadRow[2]/I");
  outTr->Branch("hadIdx",dihad->hadIdx,"hadIdx[2]/I");
  outTr->Branch("hadE",dihad->hadE,"hadE[2]/F");
  outTr->Branch("hadP",dihad->hadP,"hadP[2]/F");
  outTr->Branch("hadPt",dihad->hadPt,"hadPt[2]/F");
  outTr->Branch("hadEta",dihad->hadEta,"hadEta[2]/F");
  outTr->Branch("hadPhi",dihad->hadPhi,"hadPhi[2]/F");
  outTr->Branch("hadXF",dihad->hadXF,"hadXF[2]/F");
  outTr->Branch("hadVertex",dihad->hadVertex,"hadVertex[2][3]/F");
  outTr->Branch("hadStatus",dihad->hadStatus,"hadStatus[2]/I");
  outTr->Branch("hadBeta",dihad->hadBeta,"hadBeta[2]/F");
  outTr->Branch("hadChi2pid",dihad->hadChi2pid,"hadChi2pid[2]/F");
  Bool_t hadFiduCut[2];
  Bool_t diphotFiduCut;
  outTr->Branch("hadFiduCut",hadFiduCut,"hadFiduCut[2]/O"); // re-determined below
  // - dihadron branches
  outTr->Branch("Mh",&(dihad->Mh),"Mh/F");
  outTr->Branch("Mmiss",&(dihad->Mmiss),"Mmiss/F");
  outTr->Branch("Z",dihad->z,"Z[2]/F");
  outTr->Branch("Zpair",&(dihad->zpair),"Zpair/F");
  outTr->Branch("xF",&(dihad->xF),"xF/F");
  outTr->Branch("alpha",&(dihad->alpha),"alpha/F");
  outTr->Branch("theta",&(dihad->theta),"theta/F");
  outTr->Branch("zeta",&(dihad->zeta),"zeta/F");
  outTr->Branch("Ph",&(dihad->PhMag),"Ph/F");
  outTr->Branch("PhPerp",&(dihad->PhPerpMag),"PhPerp/F");
  outTr->Branch("PhEta",&(dihad->PhEta),"PhEta/F");
  outTr->Branch("PhPhi",&(dihad->PhPhi),"PhPhi/F");
  outTr->Branch("R",&(dihad->RMag),"R/F");
  outTr->Branch("RPerp",&(dihad->RPerpMag),"RPerp/F");
  outTr->Branch("RT",&(dihad->RTMag),"RT/F");
  outTr->Branch("PhiH",&(dihad->PhiH),"PhiH/F");
  // -- phiR angles
  outTr->Branch("PhiRq",&(dihad->PhiRq),"PhiRq/F"); // via R_perp
  outTr->Branch("PhiRp",&(dihad->PhiRp),"PhiRp/F"); // via R_T
  outTr->Branch("PhiRp_r",&(dihad->PhiRp_r),"PhiRp_r/F"); // via R_T (frame-dependent)
  outTr->Branch("PhiRp_g",&(dihad->PhiRp_g),"PhiRp_g/F"); // via eq. 9 in 1408.5721
  // - event-level branches
  outTr->Branch("runnum",&(ev->runnum),"runnum/I");
  outTr->Branch("evnum",&(ev->evnum),"evnum/I");
  outTr->Branch("helicity",&(ev->helicity),"helicity/I");
  // - photon and diphoton branches
  outTr->Branch("photE",diphot->photE,"photE[2]/F");
  outTr->Branch("photPt",diphot->photPt,"photPt[2]/F");
  outTr->Branch("photEta",diphot->photEta,"photEta[2]/F");
  outTr->Branch("photPhi",diphot->photPhi,"photPhi[2]/F");
  outTr->Branch("photAng",diphot->photAng,"photAng[2]/F");
  outTr->Branch("photBeta",diphot->photBeta,"photBeta[2]/F");
  outTr->Branch("photChi2pid",diphot->photChi2pid,"photChi2pid[2]/F");
  outTr->Branch("diphM",&(diphot->M),"diphM/F");
  outTr->Branch("diphZE",&(diphot->ZE),"diphZE/F");
  outTr->Branch("diphVtxDiff",&(diphot->VtxDiff),"diphVtxDiff/F");
  ///////////////////// MC branches
  outTr->Branch("diphMCpi0",&(diphot->IsMCpi0),"diphMCpi0/O"); // true iff MC pi0 decay
  outTr->Branch("diphMCmatchDist",&(diphot->MCmatchDist),"diphMCmatchDist/D"); // hypot( photon1_matchDist, photon2_matchDist )
  /* (TODO: propagate generated kinematics branches
   * - need to generalize or duplicate EventTree::GetTrajectories, since
   *   we are looping with Ensemble here, not EventTree
   * - the commented out branches below are ready to be used, we just
   *   need to make sure that all the EventTree variables are set correctly
   */
  // // - generated DIS kinematics
  // outTr->Branch("gen_W",&(ev->gen_W),"gen_W/F");
  // outTr->Branch("gen_Q2",&(ev->gen_Q2),"gen_Q2/F");
  // outTr->Branch("gen_Nu",&(ev->gen_Nu),"gen_Nu/F");
  // outTr->Branch("gen_x",&(ev->gen_x),"gen_x/F");
  // outTr->Branch("gen_y",&(ev->gen_y),"gen_y/F");
  // // - generated electron kinematics branches
  // outTr->Branch("gen_eleE",&(ev->gen_eleE),"gen_eleE/F");
  // outTr->Branch("gen_eleP",&(ev->gen_eleP),"gen_eleP/F");
  // outTr->Branch("gen_elePt",&(ev->gen_elePt),"gen_elePt/F");
  // outTr->Branch("gen_eleEta",&(ev->gen_eleEta),"gen_eleEta/F");
  // outTr->Branch("gen_elePhi",&(ev->gen_elePhi),"gen_elePhi/F");
  // outTr->Branch("gen_eleVertex",ev->gen_eleVertex,"gen_eleVertex[3]/F");
  // // - generated hadron branches
  // outTr->Branch("gen_pairType",&(ev->gen_pairType),"gen_pairType/I");
  // outTr->Branch("gen_hadRow",ev->gen_hadRow,"gen_hadRow[2]/I");
  // outTr->Branch("gen_hadIdx",ev->gen_hadIdx,"gen_hadIdx[2]/I");
  // outTr->Branch("gen_hadE",ev->gen_hadE,"gen_hadE[2]/F");
  // outTr->Branch("gen_hadP",ev->gen_hadP,"gen_hadP[2]/F");
  // outTr->Branch("gen_hadPt",ev->gen_hadPt,"gen_hadPt[2]/F");
  // outTr->Branch("gen_hadEta",ev->gen_hadEta,"gen_hadEta[2]/F");
  // outTr->Branch("gen_hadPhi",ev->gen_hadPhi,"gen_hadPhi[2]/F");
  // outTr->Branch("gen_hadXF",ev->gen_hadXF,"gen_hadXF[2]/F");
  // outTr->Branch("gen_hadVertex",ev->gen_hadVertex,"gen_hadVertex[2][3]/F");
  // // - generated dihadron branches
  // outTr->Branch("gen_Mh",&(ev->gen_Mh),"gen_Mh/F");
  // outTr->Branch("gen_Mmiss",&(ev->gen_Mmiss),"gen_Mmiss/F");
  // outTr->Branch("gen_Z",ev->gen_Z,"gen_Z[2]/F");
  // outTr->Branch("gen_Zpair",&(ev->gen_Zpair),"gen_Zpair/F");
  // outTr->Branch("gen_xF",&(ev->gen_xF),"gen_xF/F");
  // outTr->Branch("gen_alpha",&(ev->gen_alpha),"gen_alpha/F");
  // outTr->Branch("gen_theta",&(ev->gen_theta),"gen_theta/F");
  // outTr->Branch("gen_zeta",&(ev->gen_zeta),"gen_zeta/F");
  // outTr->Branch("gen_Ph",&(ev->gen_Ph),"gen_Ph/F");
  // outTr->Branch("gen_PhPerp",&(ev->gen_PhPerp),"gen_PhPerp/F");
  // outTr->Branch("gen_PhEta",&(ev->gen_PhEta),"gen_PhEta/F");
  // outTr->Branch("gen_PhPhi",&(ev->gen_PhPhi),"gen_PhPhi/F");
  // outTr->Branch("gen_R",&(ev->gen_R),"gen_R/F");
  // outTr->Branch("gen_RPerp",&(ev->gen_RPerp),"gen_RPerp/F");
  // outTr->Branch("gen_RT",&(ev->gen_RT),"gen_RT/F");
  // outTr->Branch("gen_PhiH",&(ev->gen_PhiH),"gen_PhiH/F");
  // outTr->Branch("gen_PhiRq",&(ev->gen_PhiRq),"gen_PhiRq/F");
  // outTr->Branch("gen_PhiRp",&(ev->gen_PhiRp),"gen_PhiRp/F");
  // outTr->Branch("gen_PhiRp_r",&(ev->gen_PhiRp_r),"gen_PhiRp_r/F");
  // outTr->Branch("gen_PhiRp_g",&(ev->gen_PhiRp_g),"gen_PhiRp_g/F");
  // // - match quality
  // outTr->Branch("gen_eleIsMatch",&(ev->gen_eleIsMatch),"gen_eleIsMatch/O");
  // outTr->Branch("gen_hadIsMatch",ev->gen_hadIsMatch,"gen_hadIsMatch[2]/O");
  // outTr->Branch("gen_eleMatchDist",&(ev->gen_eleMatchDist),"gen_eleMatchDist/F");
  // outTr->Branch("gen_hadMatchDist",ev->gen_hadMatchDist,"gen_hadMatchDist[2]/F");
  // // - other
  // outTr->Branch("gen_hadParentIdx",ev->gen_hadParentIdx,"gen_hadParentIdx[2]/I");
  // outTr->Branch("gen_hadParentPid",ev->gen_hadParentPid,"gen_hadParentPid[2]/I");


  // ensemble loop ===================================
  Long64_t hi;
  Int_t qh;
  while(ens->NextEvent()) {
    //if(ens->GetEnum()>500000) break; // limiter

    // get DIS event, and calculate kinematics
    ev->GetTrajectories(ens->GetEnum());
    disEv->SetBeamEn(ev->beamE); // must use stored beamE, in case we changed it upstream somewhere
    disEv->CalculateKinematics(ev->GetElectronTraj());


    // loop over dihadrons ----------------------
    diphot->ResetVars(); // (to zero photon branches)
    for(Long64_t di : ens->GetDihadronList()) {

      // calculate dihadron kinematics
      ev->GetTrajectories(di);
      dihad->CalculateKinematics(
        ev->GetHadronTraj(qA),
        ev->GetHadronTraj(qB),
        disEv
      );
      for(int h=0; h<2; h++) hadFiduCut[h] = ev->hadFiduCut[h];

      // fill output tree
      outTr->Fill();

    }; // end dihadron loop
     

    // loop over diphotons --------------------------
    // - each diphoton will be paired with each hadron
    if(debug) cout << "event " << ens->GetEvnum() << endl;
    for(Long64_t di : ens->GetDiphotonList()) {
      if(debug) cout << "  di = " << di << endl;

      // calculate diphoton kinematics
      ev->GetTrajectories(di);
      diphot->CalculateKinematics(
        ev->GetHadronTraj(qA),
        ev->GetHadronTraj(qB),
        disEv
      );
      diphotFiduCut = ev->hadFiduCut[qA] && ev->hadFiduCut[qB];
      

      // loop over hadrons ------------------------------
      // - pair current diphoton with each hadron
      if(debug) cout << "  nhad = " << ens->GetHadronList().size() << endl;
      for(auto hadUID : ens->GetHadronList()) {
        hi = hadUID.first;
        qh = hadUID.second;
        if(debug) cout << "  had = " << hi << ", " << qh << endl;

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
          hadFiduCut[qA] = ev->hadFiduCut[qh];
          hadFiduCut[qB] = diphotFiduCut;
        } else {
          dihad->CalculateKinematics(
            diphot->GetDiphotonTraj(),
            ev->GetHadronTraj(qh),
            disEv
          );
          hadFiduCut[qA] = diphotFiduCut;
          hadFiduCut[qB] = ev->hadFiduCut[qh];
        };

        // fill output tree
        outTr->Fill();

      }; // end hadron loop
    }; // end diphoton loop

  }; // end ensemble loop ===========================


  // write output
  outTr->Write();
  outfile->Close();


  return 0;
};


