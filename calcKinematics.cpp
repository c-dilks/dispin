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


enum parEnum {kEle,kHadA,kHadB};
TString parName[3];
TFile * diskimFile;
TTree * ditr;

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
  Float_t evnumLo;
  Float_t evnumHi;
  Float_t helicity;
  // - particle branch vars; array index corresponds to parEnum {kEle,kHadA,kHadB}
  Float_t Row[3];
  Float_t Pid[3];
  Float_t Px[3];
  Float_t Py[3];
  Float_t Pz[3];
  Float_t E[3];
  Float_t Vx[3];
  Float_t Vy[3];
  Float_t Vz[3];
  Float_t chi2pid[3];
  Float_t status[3];
  Float_t beta[3];
  Float_t pcal_found[3];
  Float_t pcal_sector[3];
  Float_t pcal_energy[3];
  Float_t pcal_time[3];
  Float_t pcal_path[3];
  Float_t pcal_x[3];
  Float_t pcal_y[3];
  Float_t pcal_z[3];
  Float_t pcal_lu[3];
  Float_t pcal_lv[3];
  Float_t pcal_lw[3];
  Float_t dcTrk_found[3];
  Float_t dcTrk_chi2[3];
  Float_t dcTrk_ndf[3];
  Float_t dcTrk_status[3];
  Float_t dcTraj_found[3];
  Float_t dcTraj_c1x[3];
  Float_t dcTraj_c1y[3];
  Float_t dcTraj_c1z[3];
  Float_t dcTraj_c2x[3];
  Float_t dcTraj_c2y[3];
  Float_t dcTraj_c2z[3];
  Float_t dcTraj_c3x[3];
  Float_t dcTraj_c3y[3];
  Float_t dcTraj_c3z[3];

  // - set branch addresses
  parName[kEle] = "ele";
  parName[kHadA] = "hadA";
  parName[kHadB] = "hadB";
  for(int p=0; p<3; p++) {
    SetParticleBranchAddress(p,"Row",Row);
  };

};
