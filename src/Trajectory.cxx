#include "Trajectory.h"

ClassImp(Trajectory)

using namespace std;


Trajectory::Trajectory() {
  debug = false;
  Idx = UNDEF;
  Row = UNDEF;
  Momentum = TLorentzVector(0,0,0,0);
  Vertex.SetXYZ(UNDEF,UNDEF,UNDEF);
  chi2pid = UNDEF;
  Status = UNDEF;
  Beta = UNDEF;
  if(debug) printf("Trajectory instantiated\n");
};



// calcuate momentum transverse to q, denoted "Ptq" or "pTq"
Float_t Trajectory::Ptq(TLorentzVector q_) {
  TVector3 pVec_ = Momentum.Vect();
  TVector3 qVec_ = q_.Vect();
  TVector3 PtqVec = Tools::Reject(pVec_,qVec_);
  /*
  printf("=== q_\n"); q_.Print();
  printf("=== qVec\n"); qVec_.Print();
  printf("=== pVec\n"); pVec_.Print();
  printf("=== PtqVec\n"); PtqVec.Print();
  */
  return PtqVec.Mag();
};



Trajectory::~Trajectory() {
};
