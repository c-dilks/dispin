// read skim HIPO file and produce a ROOT tree, called an 'outroot file', which contains
// branches associated with dihadrons

#include <cstdlib>
#include <iostream>

// ROOT
#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TMath.h"
#include "TSystem.h"
#include "TRegexp.h"
#include "TObjArray.h"
#include "TLorentzVector.h"
#include "TRandom.h"

// dispin
#include "Constants.h"
#include "DIS.h"
#include "Trajectory.h"
#include "FiducialCuts.h"
#include "Dihadron.h"
#include "Diphoton.h"
#include "EventTree.h"


enum parEnum {kEle,kHadA,kHadB,nPar};
TString parName[nPar];
TFile * diskimFile;
TTree * ditr;
TFile * outrootFile;
TTree * outrootTr;

void SetParticleBranchAddress(Int_t par, TString brName, void * brAddr) {
  ditr->SetBranchAddress(TString(parName[par]+"_"+brName),brAddr);
};


int main(int argc, char** argv) {

  // ARGUMENTS
  TString infileN;
  if(argc<=1) {
    printf("USAGE: %s [diskim file]\n",argv[0]);
    exit(0);
  };
  if(argc>1) infileN = TString(argv[1]);


  // instantiate useful objects
  DIS * disEv = new DIS();
  disEv->debug = 0;
  Dihadron * dih = new Dihadron();
  dih->debug = 0;
  dih->useBreit = false;
  Trajectory * traj[nPar];
  FiducialCuts * fidu[nPar];
  for(int p=0; p<nPar; p++) traj[p] = new Trajectory();
  fidu[kEle] = new FiducialCuts(FiducialCuts::kElectron);
  fidu[kHadA] = new FiducialCuts(FiducialCuts::kHadron);
  fidu[kHadB] = new FiducialCuts(FiducialCuts::kHadron);


  // open diskim file
  diskimFile = new TFile(infileN,"READ");
  ditr = (TTree*) diskimFile->Get("ditr");


  // define branch vars
  // - event branch vars
  Float_t evnumLo_float;
  Float_t evnumHi_float;
  Float_t helicity_float;
  Float_t runnum_float;
  // - particle branch vars; array index corresponds to parEnum
  Float_t Row[nPar];
  Float_t Pid[nPar];
  Float_t Px[nPar];
  Float_t Py[nPar];
  Float_t Pz[nPar];
  Float_t E[nPar];
  Float_t Vx[nPar];
  Float_t Vy[nPar];
  Float_t Vz[nPar];
  Float_t chi2pid[nPar];
  Float_t status[nPar];
  Float_t beta[nPar];
  // set branch addresses
  // - event branches
  ditr->SetBranchAddress("runnum",&runnum_float);
  ditr->SetBranchAddress("evnumLo",&evnumLo_float);
  ditr->SetBranchAddress("evnumHi",&evnumHi_float);
  ditr->SetBranchAddress("helicity",&helicity_float);
  // - particle branches
  parName[kEle] = "ele";
  parName[kHadA] = "hadA";
  parName[kHadB] = "hadB";
  for(int p=0; p<nPar; p++) {
    // - particle info
    SetParticleBranchAddress(p,"Row",&(Row[p]));
    SetParticleBranchAddress(p,"Pid",&(Pid[p]));
    SetParticleBranchAddress(p,"Px",&(Px[p]));
    SetParticleBranchAddress(p,"Py",&(Py[p]));
    SetParticleBranchAddress(p,"Pz",&(Pz[p]));
    SetParticleBranchAddress(p,"E",&(E[p]));
    SetParticleBranchAddress(p,"Vx",&(Vx[p]));
    SetParticleBranchAddress(p,"Vy",&(Vy[p]));
    SetParticleBranchAddress(p,"Vz",&(Vz[p]));
    SetParticleBranchAddress(p,"chi2pid",&(chi2pid[p]));
    SetParticleBranchAddress(p,"status",&(status[p]));
    SetParticleBranchAddress(p,"beta",&(beta[p]));
    // - detector info
    SetParticleBranchAddress(p,"pcal_found",&(fidu[p]->part_Cal_PCAL_found[0]));
    SetParticleBranchAddress(p,"pcal_sector",&(fidu[p]->part_Cal_PCAL_sector[0]));
    SetParticleBranchAddress(p,"pcal_energy",&(fidu[p]->part_Cal_PCAL_energy[0]));
    SetParticleBranchAddress(p,"pcal_time",&(fidu[p]->part_Cal_PCAL_time[0]));
    SetParticleBranchAddress(p,"pcal_path",&(fidu[p]->part_Cal_PCAL_path[0]));
    SetParticleBranchAddress(p,"pcal_x",&(fidu[p]->part_Cal_PCAL_x[0]));
    SetParticleBranchAddress(p,"pcal_y",&(fidu[p]->part_Cal_PCAL_y[0]));
    SetParticleBranchAddress(p,"pcal_z",&(fidu[p]->part_Cal_PCAL_z[0]));
    SetParticleBranchAddress(p,"pcal_lu",&(fidu[p]->part_Cal_PCAL_lu[0]));
    SetParticleBranchAddress(p,"pcal_lv",&(fidu[p]->part_Cal_PCAL_lv[0]));
    SetParticleBranchAddress(p,"pcal_lw",&(fidu[p]->part_Cal_PCAL_lw[0]));
    //
    SetParticleBranchAddress(p,"dcTrk_found",&(fidu[p]->part_DC_Track_found[0]));
    SetParticleBranchAddress(p,"dcTrk_chi2",&(fidu[p]->part_DC_Track_chi2[0]));
    SetParticleBranchAddress(p,"dcTrk_ndf",&(fidu[p]->part_DC_Track_NDF[0]));
    SetParticleBranchAddress(p,"dcTrk_status",&(fidu[p]->part_DC_Track_status[0]));
    //
    SetParticleBranchAddress(p,"dcTraj_found",&(fidu[p]->part_DC_Traj_found[0]));
    SetParticleBranchAddress(p,"dcTraj_c1x",&(fidu[p]->part_DC_c1x[0]));
    SetParticleBranchAddress(p,"dcTraj_c1y",&(fidu[p]->part_DC_c1y[0]));
    SetParticleBranchAddress(p,"dcTraj_c1z",&(fidu[p]->part_DC_c1z[0]));
    SetParticleBranchAddress(p,"dcTraj_c2x",&(fidu[p]->part_DC_c2x[0]));
    SetParticleBranchAddress(p,"dcTraj_c2y",&(fidu[p]->part_DC_c2y[0]));
    SetParticleBranchAddress(p,"dcTraj_c2z",&(fidu[p]->part_DC_c2z[0]));
    SetParticleBranchAddress(p,"dcTraj_c3x",&(fidu[p]->part_DC_c3x[0]));
    SetParticleBranchAddress(p,"dcTraj_c3y",&(fidu[p]->part_DC_c3y[0]));
    SetParticleBranchAddress(p,"dcTraj_c3z",&(fidu[p]->part_DC_c3z[0]));
  };


  // define outroot file
  TString outrootFileN = infileN;
  outrootFileN(TRegexp("^.*/")) = "outroot/";
  outrootFile = new TFile(outrootFileN,"RECREATE");


  // define outroot tree
  outrootTr = new TTree("tree","tree");
  // - DIS kinematics branches
  outrootTr->Branch("W",&(disEv->W),"W/F");
  outrootTr->Branch("Q2",&(disEv->Q2),"Q2/F");
  outrootTr->Branch("Nu",&(disEv->Nu),"Nu/F");
  outrootTr->Branch("x",&(disEv->x),"x/F");
  outrootTr->Branch("y",&(disEv->y),"y/F");
  // - electron kinematics branches
  outrootTr->Branch("eleE",&(disEv->eleE),"eleE/F");
  outrootTr->Branch("eleP",&(disEv->eleP),"eleP/F");
  outrootTr->Branch("elePt",&(disEv->elePt),"elePt/F");
  outrootTr->Branch("eleEta",&(disEv->eleEta),"eleEta/F");
  outrootTr->Branch("elePhi",&(disEv->elePhi),"elePhi/F");
  outrootTr->Branch("eleVertex",disEv->eleVertex,"eleVertex[3]/F");
  outrootTr->Branch("eleStatus",&(disEv->eleStatus),"eleStatus/I");
  outrootTr->Branch("eleChi2pid",&(disEv->eleChi2pid),"eleChi2pid/F");
  outrootTr->Branch("eleFiduCut",fidu[kEle]->fiduCut,"eleFiduCut[3]/O");
  // - hadron branches
  outrootTr->Branch("pairType",&(dih->pairType),"pairType/I");
  outrootTr->Branch("hadRow",dih->hadRow,"hadRow[2]/I");
  outrootTr->Branch("hadIdx",dih->hadIdx,"hadIdx[2]/I");
  outrootTr->Branch("hadE",dih->hadE,"hadE[2]/F");
  outrootTr->Branch("hadP",dih->hadP,"hadP[2]/F");
  outrootTr->Branch("hadPt",dih->hadPt,"hadPt[2]/F");
  outrootTr->Branch("hadEta",dih->hadEta,"hadEta[2]/F");
  outrootTr->Branch("hadPhi",dih->hadPhi,"hadPhi[2]/F");
  outrootTr->Branch("hadXF",dih->hadXF,"hadXF[2]/F");
  outrootTr->Branch("hadVertex",dih->hadVertex,"hadVertex[2][3]/F");
  outrootTr->Branch("hadStatus",dih->hadStatus,"hadStatus[2]/I");
  outrootTr->Branch("hadBeta",dih->hadBeta,"hadBeta[2]/F");
  outrootTr->Branch("hadChi2pid",dih->hadChi2pid,"hadChi2pid[2]/F");
  Bool_t fiduCutHad[2][FiducialCuts::nLevel];
  outrootTr->Branch("hadFiduCut",fiduCutHad,"hadFiduCut[2][3]/O");
  // - dihadron branches
  outrootTr->Branch("Mh",&(dih->Mh),"Mh/F");
  outrootTr->Branch("Mmiss",&(dih->Mmiss),"Mmiss/F");
  outrootTr->Branch("Z",dih->z,"Z[2]/F");
  outrootTr->Branch("Zpair",&(dih->zpair),"Zpair/F");
  outrootTr->Branch("xF",&(dih->xF),"xF/F");
  outrootTr->Branch("alpha",&(dih->alpha),"alpha/F");
  outrootTr->Branch("theta",&(dih->theta),"theta/F");
  outrootTr->Branch("zeta",&(dih->zeta),"zeta/F");
  outrootTr->Branch("Ph",&(dih->PhMag),"Ph/F");
  outrootTr->Branch("PhPerp",&(dih->PhPerpMag),"PhPerp/F");
  outrootTr->Branch("PhEta",&(dih->PhEta),"PhEta/F");
  outrootTr->Branch("PhPhi",&(dih->PhPhi),"PhPhi/F");
  outrootTr->Branch("R",&(dih->RMag),"R/F");
  outrootTr->Branch("RPerp",&(dih->RPerpMag),"RPerp/F");
  outrootTr->Branch("RT",&(dih->RTMag),"RT/F");
  outrootTr->Branch("PhiH",&(dih->PhiH),"PhiH/F");
  // -- phiR angles
  outrootTr->Branch("PhiRq",&(dih->PhiRq),"PhiRq/F"); // via R_perp
  outrootTr->Branch("PhiRp",&(dih->PhiRp),"PhiRp/F"); // via R_T
  outrootTr->Branch("PhiRp_r",&(dih->PhiRp_r),"PhiRp_r/F"); // via R_T (frame-dependent)
  outrootTr->Branch("PhiRp_g",&(dih->PhiRp_g),"PhiRp_g/F"); // via eq. 9 in 1408.5721
  // - event-level branches
  Int_t runnum,evnum,evnumLo,evnumHi;
  Int_t helicity;
  outrootTr->Branch("runnum",&runnum,"runnum/I");
  outrootTr->Branch("evnum",&evnum,"evnum/I");
  outrootTr->Branch("helicity",&helicity,"helicity/I");



  //-----------------------------------
  // loop through diskim tree
  //-----------------------------------
  for(int i=0; i<ditr->GetEntries(); i++) {
    if(i%10000==0) printf("[+] %.2f%%\n",100*((float)i)/ditr->GetEntries());
    //if(i>30000) break; // limiter
    ditr->GetEntry(i);

    // reset branches
    disEv->ResetVars();
    dih->ResetVars();

    // event level
    runnum = (Int_t) runnum_float;
    helicity = (Int_t) helicity_float;
    evnumLo = (Int_t) evnumLo_float;
    evnumHi = (Int_t) evnumHi_float;
    evnum = evnumLo + (evnumHi<<16);

    // fiducial cuts
    for(int p=0; p<nPar; p++) {
      fidu[p]->ApplyCuts(runnum,(int)Pid[p]);
    };

    // set Trajectory values (not DC trajectories, but Trajectory class)
    for(int p=0; p<nPar; p++) {
      traj[p]->Row = (Int_t) Row[p];
      traj[p]->Idx = PIDtoIdx((Int_t)Pid[p]);
      traj[p]->Momentum.SetPxPyPzE(Px[p],Py[p],Pz[p],E[p]);
      traj[p]->Vertex.SetXYZ(Vx[p],Vy[p],Vz[p]);
      traj[p]->chi2pid = chi2pid[p];
      traj[p]->Status = (Int_t) status[p];
      traj[p]->Beta = beta[p];
    };

    // calculate DIS kinematics
    disEv->CalculateKinematics(traj[kEle]);

    // calculate dihadron kinematics
    // - CorrectOrder (from Constants.h) ensures the standard hadron
    //   ordering is used (see Constants::diHadIdx)
    //   - also ensures the hadrons' fiducial cuts are ordered properly
    if(CorrectOrder( traj[kHadA]->Idx, traj[kHadB]->Idx )) {
      dih->CalculateKinematics( traj[kHadA], traj[kHadB], disEv );
      for(int l=0; l<FiducialCuts::nLevel; l++) {
        fiduCutHad[qA][l] = fidu[kHadA]->fiduCut[l];
        fiduCutHad[qB][l] = fidu[kHadB]->fiduCut[l];
      };
    } else {
      dih->CalculateKinematics( traj[kHadB], traj[kHadA], disEv );
      for(int l=0; l<FiducialCuts::nLevel; l++) {
        fiduCutHad[qA][l] = fidu[kHadB]->fiduCut[l];
        fiduCutHad[qB][l] = fidu[kHadA]->fiduCut[l];
      };
    };


    // fill the outroot tree
    outrootTr->Fill();
  };


  // write output and close
  outrootFile->cd();
  outrootTr->Write();
  printf("tree written\n");
  outrootFile->Close();
};
