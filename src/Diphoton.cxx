#include "Diphoton.h"

ClassImp(Diphoton)

using namespace std;


Diphoton::Diphoton() {
  debug = true;

  Traj = new Trajectory();
  Traj->Idx = kDiph;
  vecDiphoton.SetXYZM(0,0,0,0);

  printf("Diphoton instantiated\n");
};


void Diphoton::SetEvent(Trajectory * traj1, Trajectory * traj2) {

  ResetVars();

  photon[0] = traj1;
  photon[1] = traj2;
  vecDiphoton = photon[0]->Momentum + photon[1]->Momentum;


  // get photon kinematics
  for(int h=0; h<2; h++) {
    momPhoton[h] = (photon[h]->Momentum).Vect();
    photE[h] = (photon[h]->Momentum).E();
    photPt[h] = (photon[h]->Momentum).Pt();
    photEta[h] = (photon[h]->Momentum).Eta();
    photPhi[h] = (photon[h]->Momentum).Phi();
    photChi2pid[h] = photon[h]->chi2pid;
    photVertex[h][eX] = (photon[h]->Vertex).X();
    photVertex[h][eY] = (photon[h]->Vertex).Y();
    photVertex[h][eZ] = (photon[h]->Vertex).Z();
  };
  E = vecDiphoton.E();

  // transverse momentum
  Pt = vecDiphoton.Pt(); // IN LAB FRAME, wrt BEAM AXIS (maybe change to w.r.t. q?)


  // prevent dividing by 0 issues
  if(E<=0 || Pt<=0) {
    ResetVars();
    return;
  };


  // eta and phi
  Eta = vecDiphoton.Eta();
  Phi = vecDiphoton.Phi();

  // energy sharing
  Z = fabs(photE[0]-photE[1]) / E;

  // invariant mass
  M = vecDiphoton.M();

  // opening angle
  Alpha = TMath::ACos(
    momPhoton[0].Dot(momPhoton[1]) / ( momPhoton[0].Mag() * momPhoton[1].Mag() ) 
  );


  // set Trajectory
  Traj->Momentum = vecDiphoton;


  // set booleans
  //validDiphoton = Alpha < 0.3;
  validDiphoton = true;


};


void Diphoton::ResetVars() {
  for(int h=0; h<2; h++) {
    photE[h] = UNDEF;
    photPt[h] = UNDEF;
    photEta[h] = UNDEF;
    photPhi[h] = UNDEF;
    for(int c=0; c<3; c++) photVertex[h][c] = UNDEF;
  };
  E = UNDEF;
  Z = UNDEF;
  Pt = UNDEF;
  M = UNDEF;
  Alpha = UNDEF;
  Eta = UNDEF;
  Phi = UNDEF;

  validDiphoton = false;
};




Diphoton::~Diphoton() {
};

