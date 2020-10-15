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

// dispin
#include "Constants.h"
#include "DIS.h"
#include "Trajectory.h"
#include "FiducialCuts.h"
#include "Dihadron.h"
#include "Diphoton.h"
#include "Modulation.h"
#include "EventTree.h"


enum parEnum {kEle,kHadA,kHadB,nPar};
TFile * diskimFile;
TTree * ditr;
TFile * outrootFile;
TTree * outrootTr;


void SetParticleBranchAddress(TString parStr, TString brName, void * brAddr) {
  ditr->SetBranchAddress(TString(parStr+"_"+brName),brAddr);
};


int main(int argc, char** argv) {

  // ARGUMENTS
  TString infileN;
  TString dataStream = "data";
  TString injectStream = "injgen";
  if(argc<=1) {
    printf("USAGE: %s [diskim file]\n",argv[0]);
    printf(" additional arguments for MC: [data/mcrec/mcgen] [injgen/injrec]\n");
    exit(0);
  };
  if(argc>1) infileN = TString(argv[1]);
  if(argc>2) dataStream = TString(argv[2]);
  if(argc>3) injectStream = TString(argv[3]);


  // instantiate useful objects
  DIS * disEv = new DIS();
  DIS * disEvMC = new DIS();
  disEv->debug = 0; disEvMC->debug = 0;
  Dihadron * dih = new Dihadron();
  Dihadron * dihMC = new Dihadron();
  dih->debug = 0; dihMC->debug = 0;
  dih->useBreit = false; dihMC->useBreit = false;
  Trajectory * traj[nPar];
  Trajectory * trajMC[nPar];
  FiducialCuts * fidu[nPar];
  for(int p=0; p<nPar; p++) {
    traj[p] = new Trajectory();
    trajMC[p] = new Trajectory();
    fidu[p] = new FiducialCuts();
  };

  // open diskim file
  diskimFile = new TFile(infileN,"READ");
  ditr = (TTree*) diskimFile->Get("ditr");

  // check MC arguments
  Bool_t useMC;
  if(dataStream=="data") useMC=false;
  else if(dataStream=="mcrec") {
    useMC=true;
    if(injectStream!="injrec" && injectStream!="injgen") {
      fprintf(stderr,"ERROR: unrecognized injectStream\n");
      return 0;
    };
  }
  else if(dataStream=="mcgen") useMC=true;
  else {
    fprintf(stderr,"ERROR: unrecognized dataStream\n");
    return 0;
  };


  // define branch vars
  // - event branch vars
  Float_t evnumLo_float;
  Float_t evnumHi_float;
  Float_t helicity_float;
  Float_t runnum_float;
  // - particle branch vars; array index corresponds to parEnum
  Float_t Row[nPar]; Float_t genRow[nPar];
  Float_t Pid[nPar]; Float_t genPid[nPar];
  Float_t Px[nPar];  Float_t genPx[nPar];
  Float_t Py[nPar];  Float_t genPy[nPar];
  Float_t Pz[nPar];  Float_t genPz[nPar];
  Float_t E[nPar];   Float_t genE[nPar];
  Float_t Vx[nPar];  Float_t genVx[nPar];
  Float_t Vy[nPar];  Float_t genVy[nPar];
  Float_t Vz[nPar];  Float_t genVz[nPar];
                     Float_t genMatchDist[nPar];
  Float_t chi2pid[nPar];
  Float_t status[nPar];
  Float_t beta[nPar];
  TString parName[nPar];
  parName[kEle] = "ele";
  parName[kHadA] = "hadA";
  parName[kHadB] = "hadB";
  TString parMCname[nPar];
  for(int p=0; p<nPar; p++) parMCname[p]="gen_"+parName[p];
  // set branch addresses
  // - event branches
  ditr->SetBranchAddress("runnum",&runnum_float);
  ditr->SetBranchAddress("evnumLo",&evnumLo_float);
  ditr->SetBranchAddress("evnumHi",&evnumHi_float);
  ditr->SetBranchAddress("helicity",&helicity_float);
  // - particle branches
  for(int p=0; p<nPar; p++) {
    // - particle info
    SetParticleBranchAddress(parName[p],"Row",&(Row[p]));
    SetParticleBranchAddress(parName[p],"Pid",&(Pid[p]));
    SetParticleBranchAddress(parName[p],"Px",&(Px[p]));
    SetParticleBranchAddress(parName[p],"Py",&(Py[p]));
    SetParticleBranchAddress(parName[p],"Pz",&(Pz[p]));
    SetParticleBranchAddress(parName[p],"E",&(E[p]));
    SetParticleBranchAddress(parName[p],"Vx",&(Vx[p]));
    SetParticleBranchAddress(parName[p],"Vy",&(Vy[p]));
    SetParticleBranchAddress(parName[p],"Vz",&(Vz[p]));
    SetParticleBranchAddress(parName[p],"chi2pid",&(chi2pid[p]));
    SetParticleBranchAddress(parName[p],"status",&(status[p]));
    SetParticleBranchAddress(parName[p],"beta",&(beta[p]));
    // - MC generated particle info, matched to recon info
    if(useMC && dataStream=="mcrec") {
      SetParticleBranchAddress(parMCname[p],"Row",&(genRow[p]));
      SetParticleBranchAddress(parMCname[p],"Pid",&(genPid[p]));
      SetParticleBranchAddress(parMCname[p],"Px",&(genPx[p]));
      SetParticleBranchAddress(parMCname[p],"Py",&(genPy[p]));
      SetParticleBranchAddress(parMCname[p],"Pz",&(genPz[p]));
      SetParticleBranchAddress(parMCname[p],"E",&(genE[p]));
      SetParticleBranchAddress(parMCname[p],"Vx",&(genVx[p]));
      SetParticleBranchAddress(parMCname[p],"Vy",&(genVy[p]));
      SetParticleBranchAddress(parMCname[p],"Vz",&(genVz[p]));
      SetParticleBranchAddress(parMCname[p],"matchDist",&(genMatchDist[p]));
    };
    // - detector info
    SetParticleBranchAddress(parName[p],"pcal_found",&(fidu[p]->part_Cal_PCAL_found[0]));
    SetParticleBranchAddress(parName[p],"pcal_sector",&(fidu[p]->part_Cal_PCAL_sector[0]));
    SetParticleBranchAddress(parName[p],"pcal_energy",&(fidu[p]->part_Cal_PCAL_energy[0]));
    SetParticleBranchAddress(parName[p],"pcal_time",&(fidu[p]->part_Cal_PCAL_time[0]));
    SetParticleBranchAddress(parName[p],"pcal_path",&(fidu[p]->part_Cal_PCAL_path[0]));
    SetParticleBranchAddress(parName[p],"pcal_x",&(fidu[p]->part_Cal_PCAL_x[0]));
    SetParticleBranchAddress(parName[p],"pcal_y",&(fidu[p]->part_Cal_PCAL_y[0]));
    SetParticleBranchAddress(parName[p],"pcal_z",&(fidu[p]->part_Cal_PCAL_z[0]));
    SetParticleBranchAddress(parName[p],"pcal_lu",&(fidu[p]->part_Cal_PCAL_lu[0]));
    SetParticleBranchAddress(parName[p],"pcal_lv",&(fidu[p]->part_Cal_PCAL_lv[0]));
    SetParticleBranchAddress(parName[p],"pcal_lw",&(fidu[p]->part_Cal_PCAL_lw[0]));
    //
    SetParticleBranchAddress(parName[p],"ecin_found",&(fidu[p]->part_Cal_ECIN_found[0]));
    SetParticleBranchAddress(parName[p],"ecin_sector",&(fidu[p]->part_Cal_ECIN_sector[0]));
    SetParticleBranchAddress(parName[p],"ecin_energy",&(fidu[p]->part_Cal_ECIN_energy[0]));
    SetParticleBranchAddress(parName[p],"ecin_time",&(fidu[p]->part_Cal_ECIN_time[0]));
    SetParticleBranchAddress(parName[p],"ecin_path",&(fidu[p]->part_Cal_ECIN_path[0]));
    SetParticleBranchAddress(parName[p],"ecin_x",&(fidu[p]->part_Cal_ECIN_x[0]));
    SetParticleBranchAddress(parName[p],"ecin_y",&(fidu[p]->part_Cal_ECIN_y[0]));
    SetParticleBranchAddress(parName[p],"ecin_z",&(fidu[p]->part_Cal_ECIN_z[0]));
    SetParticleBranchAddress(parName[p],"ecin_lu",&(fidu[p]->part_Cal_ECIN_lu[0]));
    SetParticleBranchAddress(parName[p],"ecin_lv",&(fidu[p]->part_Cal_ECIN_lv[0]));
    SetParticleBranchAddress(parName[p],"ecin_lw",&(fidu[p]->part_Cal_ECIN_lw[0]));
    //
    SetParticleBranchAddress(parName[p],"ecout_found",&(fidu[p]->part_Cal_ECOUT_found[0]));
    SetParticleBranchAddress(parName[p],"ecout_sector",&(fidu[p]->part_Cal_ECOUT_sector[0]));
    SetParticleBranchAddress(parName[p],"ecout_energy",&(fidu[p]->part_Cal_ECOUT_energy[0]));
    SetParticleBranchAddress(parName[p],"ecout_time",&(fidu[p]->part_Cal_ECOUT_time[0]));
    SetParticleBranchAddress(parName[p],"ecout_path",&(fidu[p]->part_Cal_ECOUT_path[0]));
    SetParticleBranchAddress(parName[p],"ecout_x",&(fidu[p]->part_Cal_ECOUT_x[0]));
    SetParticleBranchAddress(parName[p],"ecout_y",&(fidu[p]->part_Cal_ECOUT_y[0]));
    SetParticleBranchAddress(parName[p],"ecout_z",&(fidu[p]->part_Cal_ECOUT_z[0]));
    SetParticleBranchAddress(parName[p],"ecout_lu",&(fidu[p]->part_Cal_ECOUT_lu[0]));
    SetParticleBranchAddress(parName[p],"ecout_lv",&(fidu[p]->part_Cal_ECOUT_lv[0]));
    SetParticleBranchAddress(parName[p],"ecout_lw",&(fidu[p]->part_Cal_ECOUT_lw[0]));
    //
    SetParticleBranchAddress(parName[p],"dcTrk_found",&(fidu[p]->part_DC_Track_found[0]));
    SetParticleBranchAddress(parName[p],"dcTrk_chi2",&(fidu[p]->part_DC_Track_chi2[0]));
    SetParticleBranchAddress(parName[p],"dcTrk_ndf",&(fidu[p]->part_DC_Track_NDF[0]));
    SetParticleBranchAddress(parName[p],"dcTrk_status",&(fidu[p]->part_DC_Track_status[0]));
    //
    SetParticleBranchAddress(parName[p],"dcTraj_found",&(fidu[p]->part_DC_Traj_found[0]));
    SetParticleBranchAddress(parName[p],"dcTraj_c1x",&(fidu[p]->part_DC_c1x[0]));
    SetParticleBranchAddress(parName[p],"dcTraj_c1y",&(fidu[p]->part_DC_c1y[0]));
    SetParticleBranchAddress(parName[p],"dcTraj_c1z",&(fidu[p]->part_DC_c1z[0]));
    SetParticleBranchAddress(parName[p],"dcTraj_c2x",&(fidu[p]->part_DC_c2x[0]));
    SetParticleBranchAddress(parName[p],"dcTraj_c2y",&(fidu[p]->part_DC_c2y[0]));
    SetParticleBranchAddress(parName[p],"dcTraj_c2z",&(fidu[p]->part_DC_c2z[0]));
    SetParticleBranchAddress(parName[p],"dcTraj_c3x",&(fidu[p]->part_DC_c3x[0]));
    SetParticleBranchAddress(parName[p],"dcTraj_c3y",&(fidu[p]->part_DC_c3y[0]));
    SetParticleBranchAddress(parName[p],"dcTraj_c3z",&(fidu[p]->part_DC_c3z[0]));
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
  outrootTr->Branch("eleFiduCut",&(fidu[kEle]->fiduCut),"eleFiduCut/O");
  outrootTr->Branch("elePCALen",&(fidu[kEle]->part_Cal_PCAL_energy[0]),"elePCALen/F");
  outrootTr->Branch("eleECINen",&(fidu[kEle]->part_Cal_ECIN_energy[0]),"eleECINen/F");
  outrootTr->Branch("eleECOUTen",&(fidu[kEle]->part_Cal_ECOUT_energy[0]),"eleECOUTen/F");
  Int_t eleSector;
  outrootTr->Branch("eleSector",&eleSector,"eleSector/I");
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
  Bool_t fiduCutHad[2];
  outrootTr->Branch("hadFiduCut",fiduCutHad,"hadFiduCut[2]/O");
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


  // - MC branches
  Float_t gen_eleMatchDist;
  Float_t gen_hadMatchDist[2];
  Bool_t gen_eleIsMatch;
  Bool_t gen_hadIsMatch[2];
  Bool_t isMatch[nPar];
  if(useMC) {
    // MC matching generated branches
    if(dataStream=="mcrec") {
      // - generated DIS kinematics branches
      outrootTr->Branch("gen_W",&(disEvMC->W),"gen_W/F");
      outrootTr->Branch("gen_Q2",&(disEvMC->Q2),"gen_Q2/F");
      outrootTr->Branch("gen_Nu",&(disEvMC->Nu),"gen_Nu/F");
      outrootTr->Branch("gen_x",&(disEvMC->x),"gen_x/F");
      outrootTr->Branch("gen_y",&(disEvMC->y),"gen_y/F");
      // - generated electron kinematics branches
      outrootTr->Branch("gen_eleE",&(disEvMC->eleE),"gen_eleE/F");
      outrootTr->Branch("gen_eleP",&(disEvMC->eleP),"gen_eleP/F");
      outrootTr->Branch("gen_elePt",&(disEvMC->elePt),"gen_elePt/F");
      outrootTr->Branch("gen_eleEta",&(disEvMC->eleEta),"gen_eleEta/F");
      outrootTr->Branch("gen_elePhi",&(disEvMC->elePhi),"gen_elePhi/F");
      outrootTr->Branch("gen_eleVertex",disEvMC->eleVertex,"gen_eleVertex[3]/F");
      // - generated hadron branches
      outrootTr->Branch("gen_pairType",&(dihMC->pairType),"gen_pairType/I");
      outrootTr->Branch("gen_hadRow",dihMC->hadRow,"gen_hadRow[2]/I");
      outrootTr->Branch("gen_hadIdx",dihMC->hadIdx,"gen_hadIdx[2]/I");
      outrootTr->Branch("gen_hadE",dihMC->hadE,"gen_hadE[2]/F");
      outrootTr->Branch("gen_hadP",dihMC->hadP,"gen_hadP[2]/F");
      outrootTr->Branch("gen_hadPt",dihMC->hadPt,"gen_hadPt[2]/F");
      outrootTr->Branch("gen_hadEta",dihMC->hadEta,"gen_hadEta[2]/F");
      outrootTr->Branch("gen_hadPhi",dihMC->hadPhi,"gen_hadPhi[2]/F");
      outrootTr->Branch("gen_hadXF",dihMC->hadXF,"gen_hadXF[2]/F");
      outrootTr->Branch("gen_hadVertex",dihMC->hadVertex,"gen_hadVertex[2][3]/F");
      // - generated dihadron branches
      outrootTr->Branch("gen_Mh",&(dihMC->Mh),"gen_Mh/F");
      outrootTr->Branch("gen_Mmiss",&(dihMC->Mmiss),"gen_Mmiss/F");
      outrootTr->Branch("gen_Z",dihMC->z,"gen_Z[2]/F");
      outrootTr->Branch("gen_Zpair",&(dihMC->zpair),"gen_Zpair/F");
      outrootTr->Branch("gen_xF",&(dihMC->xF),"gen_xF/F");
      outrootTr->Branch("gen_alpha",&(dihMC->alpha),"gen_alpha/F");
      outrootTr->Branch("gen_theta",&(dihMC->theta),"gen_theta/F");
      outrootTr->Branch("gen_zeta",&(dihMC->zeta),"gen_zeta/F");
      outrootTr->Branch("gen_Ph",&(dihMC->PhMag),"gen_Ph/F");
      outrootTr->Branch("gen_PhPerp",&(dihMC->PhPerpMag),"gen_PhPerp/F");
      outrootTr->Branch("gen_PhEta",&(dihMC->PhEta),"gen_PhEta/F");
      outrootTr->Branch("gen_PhPhi",&(dihMC->PhPhi),"gen_PhPhi/F");
      outrootTr->Branch("gen_R",&(dihMC->RMag),"gen_R/F");
      outrootTr->Branch("gen_RPerp",&(dihMC->RPerpMag),"gen_RPerp/F");
      outrootTr->Branch("gen_RT",&(dihMC->RTMag),"gen_RT/F");
      outrootTr->Branch("gen_PhiH",&(dihMC->PhiH),"gen_PhiH/F");
      outrootTr->Branch("gen_PhiRq",&(dihMC->PhiRq),"gen_PhiRq/F");
      outrootTr->Branch("gen_PhiRp",&(dihMC->PhiRp),"gen_PhiRp/F");
      outrootTr->Branch("gen_PhiRp_r",&(dihMC->PhiRp_r),"gen_PhiRp_r/F");
      outrootTr->Branch("gen_PhiRp_g",&(dihMC->PhiRp_g),"gen_PhiRp_g/F");
      // - match quality
      outrootTr->Branch("gen_eleIsMatch",&gen_eleIsMatch,"gen_eleIsMatch/O");
      outrootTr->Branch("gen_hadIsMatch",gen_hadIsMatch,"gen_hadIsMatch[2]/O");
      outrootTr->Branch("gen_eleMatchDist",&gen_eleMatchDist,"gen_eleMatchDist/F");
      outrootTr->Branch("gen_hadMatchDist",gen_hadMatchDist,"gen_hadMatchDist[2]/F");
    };
  };



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


    // calculate dihadron kinematics and obtain hadron fiducial cuts
    // - CorrectOrder (from Constants.h) ensures the standard hadron
    //   ordering is used (see Constants::diHadIdx)
    //   - also ensures the hadrons' fiducial cuts are ordered properly
    if(CorrectOrder( traj[kHadA]->Idx, traj[kHadB]->Idx )) {
      dih->CalculateKinematics( traj[kHadA], traj[kHadB], disEv );
      fiduCutHad[qA] = fidu[kHadA]->fiduCut;
      fiduCutHad[qB] = fidu[kHadB]->fiduCut;
    } else {
      dih->CalculateKinematics( traj[kHadB], traj[kHadA], disEv );
      fiduCutHad[qA] = fidu[kHadB]->fiduCut;
      fiduCutHad[qB] = fidu[kHadA]->fiduCut;
    };

    
    // get EC sector for electron
    eleSector = fidu[kEle]->determineSectorEC(0);


    // MC information
    if(useMC) {

      // compute DIS and dihadron kinematics from generated, matched set
      if(dataStream=="mcrec") {
        // reset branch vars
        disEvMC->ResetVars();
        dihMC->ResetVars();
        gen_eleIsMatch = false;
        gen_eleMatchDist = UNDEF;
        for(int h=0; h<2; h++) {
          gen_hadIsMatch[h] = false;
          gen_hadMatchDist[h] = UNDEF;
        };
        // get momenta and verteces
        for(int p=0; p<nPar; p++) {
          trajMC[p]->Row = (Int_t) genRow[p];
          trajMC[p]->Idx = PIDtoIdx((Int_t)genPid[p]);
          trajMC[p]->Momentum.SetPxPyPzE(genPx[p],genPy[p],genPz[p],genE[p]);
          trajMC[p]->Vertex.SetXYZ(genVx[p],genVy[p],genVz[p]);
          isMatch[p] = trajMC[p]->Row>=0; // true if valid match
        };
        // calculate kinematics
        if(isMatch[kEle]) {
          gen_eleIsMatch = true;
          disEvMC->CalculateKinematics(trajMC[kEle]);
          gen_eleMatchDist = genMatchDist[kEle];
          if(isMatch[kHadA] && isMatch[kHadB]) {
            gen_hadIsMatch[qA] = true;
            gen_hadIsMatch[qB] = true;
            if(CorrectOrder( trajMC[kHadA]->Idx, trajMC[kHadB]->Idx )) {
              dihMC->CalculateKinematics(trajMC[kHadA],trajMC[kHadB],disEvMC);
              gen_hadMatchDist[qA] = genMatchDist[kHadA];
              gen_hadMatchDist[qB] = genMatchDist[kHadB];
            } else {
              dihMC->CalculateKinematics(trajMC[kHadB],trajMC[kHadA],disEvMC);
              gen_hadMatchDist[qA] = genMatchDist[kHadB];
              gen_hadMatchDist[qB] = genMatchDist[kHadA];
            };
          };
        };
      };
    }; // end MC info


    // fill the outroot tree
    outrootTr->Fill();
  };


  // write output and close
  outrootFile->cd();
  outrootTr->Write();
  printf("tree written\n");
  outrootFile->Close();
};
