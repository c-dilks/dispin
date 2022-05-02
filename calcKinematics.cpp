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
#include "TRandomGen.h"

// dispin
#include "Constants.h"
#include "DIS.h"
#include "Trajectory.h"
#include "FiducialCuts.h"
#include "Dihadron.h"


enum parEnum {kEle,kHadA,kHadB,nPar};
TFile * diskimFile;
TTree * ditr;
TFile * outrootFile;
TTree * outrootTr;
TRandom * RNG;
Int_t evnumTmp;
Float_t beamEtmp;


void SetParticleBranchAddress(TString parStr, TString brName, void * brAddr) {
  ditr->SetBranchAddress(TString(parStr+"_"+brName),brAddr);
};
Float_t RadiativeBeamEn(Int_t runnum_, Int_t evnum_);


int main(int argc, char** argv) {

  // ARGUMENTS
  TString infileN;
  TString outrootDir = "outroot";
  TString dataStream = "data";
  TString injectStream = "injgen";
  if(argc<=1) {
    printf("USAGE: %s [diskim file] [outrootDir(default=outroot)] [dataStream(default=data)]\n",argv[0]);
    printf(" - data streams: data, mcrec, mcgen\n");
    printf("   - add \"rad\" to the data stream string to use radiative-corrected beam energy model\n");
    printf(" - additional argument for MC: [injgen/injrec]\n");
    exit(0);
  };
  if(argc>1) infileN = TString(argv[1]);
  if(argc>2) outrootDir = TString(argv[2]);
  if(argc>3) dataStream = TString(argv[3]);
  if(argc>4) injectStream = TString(argv[4]);
  printf("RUN: %s\n",argv[0]);


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
  RNG = new TRandomMixMax(42567);
  evnumTmp = -10000;
  beamEtmp = 0.0;

  // open diskim file
  diskimFile = new TFile(infileN,"READ");
  ditr = (TTree*) diskimFile->Get("ditr");

  // check MC arguments
  Bool_t useMC, useRadBeam;
  if(dataStream.Contains(TRegexp("rad$"))) {
    useRadBeam = true;
    fprintf(stderr,"\nWARNING: using radiative-corrected beam energy model !!!\n\n");
    dataStream(TRegexp("rad$")) = "";
  } else useRadBeam = false;
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
                     Float_t genParentIdx[nPar];
                     Float_t genParentPid[nPar];
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
      SetParticleBranchAddress(parMCname[p],"parentIdx",&(genParentIdx[p]));
      SetParticleBranchAddress(parMCname[p],"parentPid",&(genParentPid[p]));
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
  outrootFileN(TRegexp("^.*/")) = outrootDir+"/";
  outrootFile = new TFile(outrootFileN,"RECREATE");


  // define outroot tree
  outrootTr = new TTree("tree","tree");
  // - DIS kinematics branches
  outrootTr->Branch("W",&(disEv->W),"W/F");
  outrootTr->Branch("Q2",&(disEv->Q2),"Q2/F");
  outrootTr->Branch("Nu",&(disEv->Nu),"Nu/F");
  outrootTr->Branch("x",&(disEv->x),"x/F");
  outrootTr->Branch("y",&(disEv->y),"y/F");
  outrootTr->Branch("beamE",&(disEv->BeamEn),"beamE/F");
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
  Int_t gen_hadParentIdx[2];
  Int_t gen_hadParentPid[2];
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
      outrootTr->Branch("gen_hadParentIdx",gen_hadParentIdx,"gen_hadParentIdx[2]/I");
      outrootTr->Branch("gen_hadParentPid",gen_hadParentPid,"gen_hadParentPid[2]/I");
    };
  };



  //-----------------------------------
  // loop through diskim tree
  //-----------------------------------
  for(int i=0; i<ditr->GetEntries(); i++) {
    if(i%10000==0) printf("[+] %.2f%%\n",100*((float)i)/ditr->GetEntries());
    //if(i>100000) break; // limiter
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

    // CUT: keep only diphotons where both photons pass fiducial cuts
    //      (to conserve disk space)
    if( traj[kHadA]->Idx==kPhoton && traj[kHadB]->Idx==kPhoton ) {
      if( !fidu[kHadA]->fiduCut || !fidu[kHadB]->fiduCut) continue;
    };

    // calculate DIS kinematics
    if(!useRadBeam) disEv->SetBeamEnFromRun(runnum);
    else disEv->SetBeamEn(RadiativeBeamEn(runnum,evnum));
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
          gen_hadParentIdx[h] = -1;
          gen_hadParentPid[h] = -1;
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
          if(!useRadBeam) disEvMC->SetBeamEnFromRun(runnum);
          else disEvMC->SetBeamEn(disEv->BeamEn); // if useRadBeam, copy BeamEn from disEv
          disEvMC->CalculateKinematics(trajMC[kEle]);
          gen_eleMatchDist = genMatchDist[kEle];
          if(isMatch[kHadA] && isMatch[kHadB]) {
            gen_hadIsMatch[qA] = true;
            gen_hadIsMatch[qB] = true;
            if(CorrectOrder( trajMC[kHadA]->Idx, trajMC[kHadB]->Idx )) {
              dihMC->CalculateKinematics(trajMC[kHadA],trajMC[kHadB],disEvMC);
              gen_hadMatchDist[qA] = genMatchDist[kHadA];
              gen_hadMatchDist[qB] = genMatchDist[kHadB];
              gen_hadParentIdx[qA] = (Int_t) genParentIdx[kHadA];
              gen_hadParentIdx[qB] = (Int_t) genParentIdx[kHadB];
              gen_hadParentPid[qA] = (Int_t) genParentPid[kHadA];
              gen_hadParentPid[qB] = (Int_t) genParentPid[kHadB];
            } else {
              dihMC->CalculateKinematics(trajMC[kHadB],trajMC[kHadA],disEvMC);
              gen_hadMatchDist[qA] = genMatchDist[kHadB];
              gen_hadMatchDist[qB] = genMatchDist[kHadA];
              gen_hadParentIdx[qA] = (Int_t) genParentIdx[kHadB];
              gen_hadParentIdx[qB] = (Int_t) genParentIdx[kHadA];
              gen_hadParentPid[qA] = (Int_t) genParentPid[kHadB];
              gen_hadParentPid[qB] = (Int_t) genParentPid[kHadA];
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


//////////////////////////////////////////////////////////////////////////////////
// beam energy, modified for radiative corrections model (from Timothy/Harut)
Float_t RadiativeBeamEn(Int_t runnum_, Int_t evnum_) {

  // generate beam energy iff it is a new event
  if(evnum_==evnumTmp) return beamEtmp;
  else evnumTmp = evnum_;

  Float_t nominalBeamEn = RundepBeamEn(runnum_); // get nominal beam energy from run number

  Float_t beam_percentage[] = {
    0.99995, 0.98985, 0.97975, 0.96965, 0.95955, 0.94945, 0.93935, 
    0.92925, 0.91915, 0.90905, 0.89895, 0.88885, 0.87875, 0.86865, 
    0.85855, 0.84845, 0.83835, 0.82825, 0.81815, 0.80805, 0.79795, 
    0.78785, 0.77775, 0.76765, 0.75755, 0.74745, 0.73735, 0.72725, 
    0.71715, 0.70705, 0.69695, 0.68685, 0.67675, 0.66665, 0.65655, 
    0.64645, 0.63635, 0.62625, 0.61615, 0.60605, 0.59595, 0.58585, 
    0.57575, 0.56565, 0.55555, 0.54545, 0.53535, 0.52525, 0.51515, 
    0.50505, 0.49495, 0.48485, 0.47475, 0.46465, 0.45455, 0.44445, 
    0.43435, 0.42425, 0.41415, 0.40405, 0.39395, 0.38385, 0.37375, 
    0.36365, 0.35355, 0.34345, 0.33335, 0.32325, 0.31315, 0.30305, 
    0.29295, 0.28285, 0.27275, 0.26265, 0.25255, 0.24245, 0.23235, 
    0.22225, 0.21215, 0.20205, 0.19195, 0.18185, 0.17175, 0.16165, 
    0.15155, 0.14145, 0.13135, 0.12125, 0.11115, 0.10105, 0.09095, 
    0.08085, 0.07075, 0.06065, 0.05055, 0.04045, 0.03035, 0.02025, 
    0.01015, 0.0000499999};

  Float_t beam_likelihood[] = {
    0.791947, 0.808145, 0.825926, 0.838389, 0.847606, 0.855139, 
    0.861519, 0.867214, 0.872296, 0.876636, 0.88062, 0.884296, 0.887679, 
    0.890929, 0.893896, 0.896875, 0.899571, 0.902198, 0.904849, 0.907113, 
    0.909285, 0.911526, 0.913519, 0.915516, 0.917553, 0.919402, 0.921154, 
    0.923104, 0.924949, 0.926656, 0.928376, 0.929996, 0.931685, 0.933225, 
    0.934817, 0.936259, 0.937816, 0.939315, 0.940875, 0.942379, 0.943863, 
    0.945302, 0.946724, 0.948114, 0.949478, 0.950765, 0.952125, 0.953384, 
    0.954781, 0.956184, 0.957518, 0.958847, 0.960215, 0.961672, 0.963001, 
    0.964353, 0.96572, 0.967056, 0.968406, 0.969727, 0.971283, 0.972702, 
    0.974054, 0.975527, 0.976977, 0.97845, 0.980008, 0.981609, 0.983213, 
    0.984729, 0.986328, 0.987938, 0.989692, 0.991332, 0.992895, 0.994582, 
    0.996264, 0.997781, 0.999169, 0.999838, 1., 1., 1., 1., 1., 1., 1., 
    1., 1., 1., 1., 1., 1., 1., 1., 1., 1., 1., 1., 1.};

  Double_t double_random = RNG->Uniform(); // \in (0,1)
  for(int i=0; i<sizeof(beam_likelihood)/sizeof(beam_likelihood[0]); i++) {
    if(double_random<beam_likelihood[i]) {
      beamEtmp = nominalBeamEn * beam_percentage[i];
      return beamEtmp;
    }
  };

  fprintf(stderr,"ERROR: failed to set radiative-corrected beam energy; setting to nominal energy instead\n");
  beamEtmp = nominalBeamEn;
  return beamEtmp;
};
