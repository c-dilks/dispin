#include "Diphoton.h"

ClassImp(Diphoton)

using namespace std;


Diphoton::Diphoton() {
  debug = true;

  // instantiate diphoton trajectory
  diphot = new Trajectory();
  this->ResetVars();

  printf("Diphoton instantiated\n");
};


void Diphoton::CalculateKinematics(
  Trajectory * trajA, Trajectory * trajB, DIS * disEv) {

  this->ResetVars();

  photon[qA] = trajA;
  photon[qB] = trajB;
  diphot->Momentum = photon[qA]->Momentum + photon[qB]->Momentum;
  
  // get photon kinematics
  for(int h=0; h<2; h++) {
    photE[h] = photon[h]->Momentum.E();
    photPt[h] = photon[h]->Momentum.Pt();
    photEta[h] = photon[h]->Momentum.Eta();
    photPhi[h] = photon[h]->Momentum.Phi();
    photChi2pid[h] = photon[h]->chi2pid;
    photBeta[h] = photon[h]->Beta;
    photVertex[h][eX] = photon[h]->Vertex.X();
    photVertex[h][eY] = photon[h]->Vertex.Y();
    photVertex[h][eZ] = photon[h]->Vertex.Z();
  };

  // get diphoton kinematics
  E = diphot->Momentum.E();
  Pt = diphot->Momentum.Pt();
  Eta = diphot->Momentum.Eta();
  Phi = diphot->Momentum.Phi();
  M = diphot->Momentum.M();

  // calculate energy imbalance
  ZE = TMath::Abs(photE[qA]-photE[qB]) / E;

  // calculate angle between electron and photon
  for(int h=0; h<2; h++) {
    photAng[h] = Tools::AngleSubtend(
      disEv->vecElectron.Vect(),
      photon[h]->Momentum.Vect()
    ) * TMath::RadToDeg();
  };

  // calculate vertex difference between photons
  VtxDiff = (photon[qA]->Vertex - photon[qB]->Vertex).Mag();
  //printf("vtxdiff = %f\n",VtxDiff);
  // set diphoton vertex to that of leading energy photon
  // TODO: maybe not the right thing to do; check VtxDiff
  diphot->Vertex = photE[qA] >= photE[qB] ?
    photon[qA]->Vertex : photon[qB]->Vertex;

  // determine if it's from a MC pi0 decay
  // - NOTE: likely you want to cut on `MCmatchDist` too; we do not
  //   apply such a cut on `IsMCpi0` here, for more flexibility downstream,
  //   since the exact cut value should be determined from the distribution
  //   of `MCmatchDist` (with `IsMCpi0==true`)
  IsMCpi0 = false;
  MCmatchDist = 10000;
  if( photon[qA]->gen_parentPid == PartPID(kPio) &&
      photon[qB]->gen_parentPid == PartPID(kPio) &&
      photon[qA]->gen_parentIdx == photon[qB]->gen_parentIdx &&
      photon[qA]->gen_isMatch &&
      photon[qB]->gen_isMatch
    )
  {
    IsMCpi0 = true;
    MCmatchDist = TMath::Hypot( photon[qA]->gen_matchDist, photon[qB]->gen_matchDist );
  };

};


// classify diphoton, to see if we have a pi0,
// and apply event selection criteria
void Diphoton::Classify() {

  // reset booleans and classifiers
  this->ResetBools();

  // beta cut // 0.9<beta<1.1
  cutPhotBeta = TMath::Abs(photBeta[qA]-1.0)<0.1 &&
                TMath::Abs(photBeta[qB]-1.0)<0.1;

  // minimum energy cut
  cutPhotEn = photE[qA]>0.6 &&
              photE[qB]>0.6;

  // photon polar angle cut
  for(int h=0; h<2; h++) photTheta[h] = Tools::EtaToTheta(photEta[h]);
  cutPhotTheta = 5<photTheta[qA] && photTheta[qA]<35 &&
                 5<photTheta[qB] && photTheta[qB]<35;

  // electron cone cut
  // photon must be far enough away from electron
  cutPhotEleAng = photAng[qA]>8.0 &&
                  photAng[qB]>8.0;

  // invariant mass cuts for pi0 and sideband
  cutMassPi0 = M > 0.107071 &&
               M < 0.155837; // hard-coded 2sigma from RGA inbending data; see diphotonFit.cpp
  cutMassSB = M > 0.17 &&
              M < 0.4; // hard-coded; above pi0 region, below eta region
  if(cutMassPi0 && cutMassSB) {
    fprintf(stderr,"ERROR: conflict of diphoton mass cuts\n");
    diphotClass = dpIgnore;
    return;
  };


  // "basic" cuts, in the sense that they apply to all
  // diphotons, no matter the classification; someday they
  // may not be so basic
  cutBasic = cutPhotBeta
          && cutPhotEn
          && cutPhotTheta
          && cutPhotEleAng;

  // classify diphoton
  if(cutBasic) {
    // basic cuts passed, now check mass cuts
    if(cutMassPi0) diphotClass = dpPi0; // pi0
    else if(cutMassSB) diphotClass = dpSB; // sideband
    else diphotClass = dpIgnore; // neither pi0 nor sideband
  }
  else {
    diphotClass = dpIgnore; // failed basic cuts
  };

  // assign trajectory index
  switch(diphotClass) {
    case dpPi0: diphot->Idx = kPio; break;
    case dpSB: diphot->Idx = kPioBG; break;
    default: diphot->Idx = kDiph;
  };

};


void Diphoton::ResetVars() {

  // diphoton trajectory
  diphot->Idx = kDiph;
  // unused trajectory variables
  diphot->chi2pid = 0;
  diphot->Status = 0;
  diphot->Beta = 0;
  diphot->Row = 0;

  // vars
  for(int h=0; h<2; h++) {
    photE[h] = UNDEF;
    photPt[h] = UNDEF;
    photEta[h] = UNDEF;
    photPhi[h] = UNDEF;
    for(int c=0; c<3; c++) photVertex[h][c] = UNDEF;
    photChi2pid[h] = UNDEF;
    photBeta[h] = UNDEF;
    photAng[h] = UNDEF;
  };
  E = UNDEF;
  Pt = UNDEF;
  Eta = UNDEF;
  Phi = UNDEF;
  ZE = UNDEF;
  M = UNDEF;
  VtxDiff = UNDEF;
  MCmatchDist = 10000;
  IsMCpi0 = false;
  this->ResetBools();
};

void Diphoton::ResetBools() {
  cutPhotBeta = false;
  cutPhotEn = false;
  cutPhotTheta = false;
  cutPhotEleAng = false;
  cutMassPi0 = false;
  cutMassSB = false;
  diphotClass = dpNull;
};


Diphoton::~Diphoton() {
};

