#include "Trajectory.h"

ClassImp(Trajectory)

using namespace std;


Trajectory::Trajectory(Int_t particle_index) {
  debug = false;
  Idx = particle_index;
  Vec = TLorentzVector(0,0,0,0);
  Vec.SetXYZM(
    0.0,
    0.0,
    0.0,
    PartMass(Idx) > 0 ? PartMass(Idx) : 0.0
  );

  Vertex.SetXYZ(UNDEF,UNDEF,UNDEF);
  chi2pid = UNDEF;
  Status = UNDEF;
  Beta = UNDEF;

  if(debug) printf("Trajectory instantiated\n");
};


void Trajectory::SetMomentum(Float_t px, Float_t py, Float_t pz) {
  Vec.SetXYZM(
    px,
    py,
    pz,
    PartMass(Idx) > 0 ? PartMass(Idx) : 0.0
  );
};




// calcuate momentum transverse to q, denoted "Ptq" or "pTq"
Float_t Trajectory::Ptq(TLorentzVector q_) {
  TVector3 pVec_ = Vec.Vect();
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

