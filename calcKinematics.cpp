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
TTree * outrootTr;

void SetParticleBranchAddress(Int_t par, TString brName, Float_t * brVar) {
  ditr->SetBranchAddress(TString(parName[par]+"_"+brName),&(brVar[par]));
};


int main(int argc, char** argv) {

  // ARGUMENTS
  TString infileN;
  if(argc<=1) {
    printf("USAGE: %s [diskim file]\n",argv[0]);
    exit(0);
  };
  if(argc>1) infileN = TString(argv[1]);


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
  Float_t pcal_found[nPar];
  Float_t pcal_sector[nPar];
  Float_t pcal_energy[nPar];
  Float_t pcal_time[nPar];
  Float_t pcal_path[nPar];
  Float_t pcal_x[nPar];
  Float_t pcal_y[nPar];
  Float_t pcal_z[nPar];
  Float_t pcal_lu[nPar];
  Float_t pcal_lv[nPar];
  Float_t pcal_lw[nPar];
  Float_t dcTrk_found[nPar];
  Float_t dcTrk_chi2[nPar];
  Float_t dcTrk_ndf[nPar];
  Float_t dcTrk_status[nPar];
  Float_t dcTraj_found[nPar];
  Float_t dcTraj_c1x[nPar];
  Float_t dcTraj_c1y[nPar];
  Float_t dcTraj_c1z[nPar];
  Float_t dcTraj_c2x[nPar];
  Float_t dcTraj_c2y[nPar];
  Float_t dcTraj_c2z[nPar];
  Float_t dcTraj_c3x[nPar];
  Float_t dcTraj_c3y[nPar];
  Float_t dcTraj_c3z[nPar];

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
    SetParticleBranchAddress(p,"Row",Row);
    SetParticleBranchAddress(p,"Pid",Pid);
    SetParticleBranchAddress(p,"Px",Px);
    SetParticleBranchAddress(p,"Py",Py);
    SetParticleBranchAddress(p,"Pz",Pz);
    SetParticleBranchAddress(p,"E",E);
    SetParticleBranchAddress(p,"Vx",Vx);
    SetParticleBranchAddress(p,"Vy",Vy);
    SetParticleBranchAddress(p,"Vz",Vz);
    SetParticleBranchAddress(p,"chi2pid",chi2pid);
    SetParticleBranchAddress(p,"status",status);
    SetParticleBranchAddress(p,"beta",beta);
    SetParticleBranchAddress(p,"pcal_found",pcal_found);
    SetParticleBranchAddress(p,"pcal_sector",pcal_sector);
    SetParticleBranchAddress(p,"pcal_energy",pcal_energy);
    SetParticleBranchAddress(p,"pcal_time",pcal_time);
    SetParticleBranchAddress(p,"pcal_path",pcal_path);
    SetParticleBranchAddress(p,"pcal_x",pcal_x);
    SetParticleBranchAddress(p,"pcal_y",pcal_y);
    SetParticleBranchAddress(p,"pcal_z",pcal_z);
    SetParticleBranchAddress(p,"pcal_lu",pcal_lu);
    SetParticleBranchAddress(p,"pcal_lv",pcal_lv);
    SetParticleBranchAddress(p,"pcal_lw",pcal_lw);
    SetParticleBranchAddress(p,"dcTrk_found",dcTrk_found);
    SetParticleBranchAddress(p,"dcTrk_chi2",dcTrk_chi2);
    SetParticleBranchAddress(p,"dcTrk_ndf",dcTrk_ndf);
    SetParticleBranchAddress(p,"dcTrk_status",dcTrk_status);
    SetParticleBranchAddress(p,"dcTraj_found",dcTraj_found);
    SetParticleBranchAddress(p,"dcTraj_c1x",dcTraj_c1x);
    SetParticleBranchAddress(p,"dcTraj_c1y",dcTraj_c1y);
    SetParticleBranchAddress(p,"dcTraj_c1z",dcTraj_c1z);
    SetParticleBranchAddress(p,"dcTraj_c2x",dcTraj_c2x);
    SetParticleBranchAddress(p,"dcTraj_c2y",dcTraj_c2y);
    SetParticleBranchAddress(p,"dcTraj_c2z",dcTraj_c2z);
    SetParticleBranchAddress(p,"dcTraj_c3x",dcTraj_c3x);
    SetParticleBranchAddress(p,"dcTraj_c3y",dcTraj_c3y);
    SetParticleBranchAddress(p,"dcTraj_c3z",dcTraj_c3z);
  };



  // instantiate useful objects
  DIS * disEv = new DIS();
  disEv->debug = 0;
  Dihadron * dih = new Dihadron();
  dih->debug = 0;
  dih->useBreit = false;
  Trajectory * traj[nPar];
  for(int p=0; p<nPar; p++) traj[p] = new Trajectory();


  // define outroot file
  TString outrootFileN = infileN;
  outrootFileN(TRegexp("^.*/")) = "outroot/";
  TFile * outrootFile = new TFile(outrootFileN,"RECREATE");


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


  // loop through diskim tree
  for(int i=0; i<ditr->GetEntries(); i++) {
    if(i%10000==0) printf("[+] %.2f%%\n",100*((float)i)/ditr->GetEntries());
    if(i>30000) break; // limiter
    ditr->GetEntry();

    // reset branches
    disEv->ResetVars();
    dih->ResetVars();

    // event level
    runnum = (Int_t) runnum_float;
    helicity = (Int_t) helicity_float;
    evnumLo = (Int_t) evnumLo_float;
    evnumHi = (Int_t) evnumHi_float;
    evnum = evnumLo + (evnumHi<<16);

    // set trajectories
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
    if(CorrectOrder( traj[kHadA]->Idx, traj[kHadB]->Idx )) {
      dih->CalculateKinematics( traj[kHadA], traj[kHadB], disEv );
    } else {
      dih->CalculateKinematics( traj[kHadB], traj[kHadA], disEv );
    };

    // fill the outroot tree
    outrootTr->Fill();
  };


  // write output and close
  outrootTr->Write();
  printf("tree written\n");
  outrootFile->Close();



  // electron fiducial cuts branches TODO
  /*
  Bool_t eleFidPCAL[FiducialCuts::nLevel];
  Bool_t eleFidDC[FiducialCuts::nLevel];
  TString brsuffix = Form("[%d]",FiducialCuts::nLevel);
  tree->Branch("eleFidPCAL",eleFidPCAL,TString("eleFidPCAL"+brsuffix+"/O"));
  tree->Branch("eleFidDC",eleFidDC,TString("eleFidDC"+brsuffix+"/O"));
  */

  // multiplicity branches TODO: remove from downstream code
  /*
  Int_t particleCntAll;
  TString particleCntStr = Form("particleCnt[%d]/I",nParticles);
  tree->Branch("particleCnt",trajCnt,particleCntStr); // number of each type of particle
  // that we consider, as defined in
  // Constants.h
  tree->Branch("particleCntAll",&particleCntAll,"particleCntAll/I"); // total number
  // of particles in the event (note:
  // includes particles not in
  // Constants.h)
  */
  //tree->Branch("hadOrder",&hadOrder,"hadOrder/I"); // TODO
  //tree->Branch("torus",&torus,"torus/F"); // TODO
  //tree->Branch("solenoid",&solenoid,"solenoid/F"); // TODO
  //tree->Branch("triggerBits",&triggerBits,"triggerBits/L"); // TODO


};
