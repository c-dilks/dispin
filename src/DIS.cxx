#include "DIS.h"

ClassImp(DIS)

using namespace std;


DIS::DIS() {
  //printf("DIS instantiated\n");
  debug = false;
  speedup = false;

  BeamEn = DEFAULT_BEAM_ENERGY;

  vecBeam = TLorentzVector(
    0.0,
    0.0,
    TMath::Sqrt(TMath::Power(BeamEn,2)-TMath::Power(PartMass(kE),2)),
    BeamEn
  );

  vecTarget = TLorentzVector(
    0.0,
    0.0,
    0.0,
    PartMass(kP)
  );
};

void DIS::SetBeamEn(Float_t BeamEn_) {
  BeamEn = BeamEn_;
  vecBeam.SetPz(Tools::EMtoP(BeamEn,PartMass(kE)));
  vecBeam.SetE(BeamEn);
};
void DIS::SetBeamEnFromRun(Int_t runnum) {
  this->SetBeamEn(RundepBeamEn(runnum)); // see Constants.h
};


void DIS::SetElectron(Trajectory * tr) {
  vecElectron = tr->Momentum;
  eleE = vecElectron.E();
  eleP = vecElectron.P();
  elePt = vecElectron.Pt();
  eleEta = vecElectron.Eta();
  elePhi = vecElectron.Phi();
  eleChi2pid = tr->chi2pid;
  eleStatus = tr->Status;
  eleVertex[eX] = (tr->Vertex).X();
  eleVertex[eY] = (tr->Vertex).Y();
  eleVertex[eZ] = (tr->Vertex).Z();
};


// compute DIS kinematics
// - IMPORTANT: it is wise to set the beam energy (e.g., `SetBeamEn` or
//   `SetBeamEnFromRun`), otherwise it will assume whatever is currently stored
//   in `this->BeamEn`
void DIS::CalculateKinematics(Trajectory * tr) {

  this->ResetVars();
  this->SetElectron(tr);

  // compute W
  vecW = vecBeam + vecTarget - vecElectron;
  std::cout << "vecTarget: "; vecTarget.Print();
  std::cout << "vecBeam: "; vecBeam.Print();
  std::cout << "vecElectron: "; vecElectron.Print();
  W = vecW.M();


  // speedup: only compute x,Q2 if W>2
  if(speedup) { if( W<2.0) return; };

  // compute Q2
  vecQ = vecBeam - vecElectron;
  Q2 = -1 * vecQ.M2();

  // compute Nu
  Nu = vecTarget.Dot(vecQ) / PartMass(kP);

  // compute x
  x = Q2 / ( 2 * vecQ.Dot(vecTarget) );

  // compute y
  y = vecTarget.Dot(vecQ) / vecTarget.Dot(vecBeam);

  // compute boost vectors
  boostvecBreit = vecQ + 2*x*vecTarget;
  BreitBoost = -1 * boostvecBreit.BoostVector();

  boostvecCom = vecQ + vecTarget;
  ComBoost = -1 * boostvecCom.BoostVector();


  if(debug) {
    PrintEvent();
    BreitPrint();
    ComPrint();
  };

  return;
};


void DIS::PrintEvent() {

  printf("------------------\n");
  printf("Kinematics:\n");
  printf("  x=%.2f",x);
  printf("  Q2=%.2f",Q2);
  printf("  W=%.2f",W);
  printf("\n");
  printf("  Nu=%.2f",Nu);
  printf("  y=%.2f",y);
  printf("\n");

  printf("\n[DIS] Lab Frame:\n");
  printf("beam\t");
  vecBeam.Print();
  printf("target\t");
  vecTarget.Print();
  printf("elec\t");
  vecElectron.Print();
  printf("Q\t");
  vecQ.Print();
  printf("W\t");
  vecW.Print();
}


void DIS::ComputeBreitKinematics() {
  breitBeam = vecBeam;
  breitTarget = vecTarget;
  breitElectron = vecElectron;
  breitW = vecW;
  breitQ = vecQ;

  breitBeam.Boost(BreitBoost);
  breitTarget.Boost(BreitBoost);
  breitElectron.Boost(BreitBoost);
  breitW.Boost(BreitBoost);
  breitQ.Boost(BreitBoost);
};

void DIS::ComputeComKinematics() {
  comBeam = vecBeam;
  comTarget = vecTarget;
  comElectron = vecElectron;
  comW = vecW;
  comQ = vecQ;

  comBeam.Boost(ComBoost);
  comTarget.Boost(ComBoost);
  comElectron.Boost(ComBoost);
  comW.Boost(ComBoost);
  comQ.Boost(ComBoost);
};


void DIS::BreitPrint() {
  ComputeBreitKinematics();
  printf("\n[DIS] Breit Frame:\n");
  //printf("beam\t");
  //breitBeam.Print();
  printf("breit target\t");
  breitTarget.Print();
  //printf("breit elec\t");
  //breitElectron.Print();
  //printf("breit W\t");
  //breitW.Print();
  printf("breit Q\t\t");
  breitQ.Print();
  printf("breit 2xp\t");
  (2*x*breitTarget).Print();

  // check breit frame properties:
  printf("\n");
  // -- 2xp momentum components equal & opposite of Q momentum
  //    (where p is the target momentum in the breit frame)
  printf("Q+2xp:\t");
  (breitQ + 2*x*breitTarget).Print();
  printf("angle(Q,p) = %f\n",breitQ.Vect().Angle(breitTarget.Vect()));
  // -- q is entirely space-like, so E component should be zero
  printf("q0 = %f\n",breitQ.E());

  printf("\n");
};


void DIS::ComPrint() {
  ComputeComKinematics();
  printf("\n[DIS] target and virtual photon CoM Frame:\n");
  printf("com target\t");
  comTarget.Print();
  //printf("com elec\t");
  //comElectron.Print();
  //printf("com W\t");
  //comW.Print();
  printf("com Q\t\t");
  comQ.Print();

  // check COM frame properties:
  printf("\n");
  // -- target and virtual photon should be equal and opposite, so their
  //    sum in the COM frame should have all momentum components equal to zero
  printf("Q+p:\t\t");
  (comQ + comTarget).Print();

  printf("\n");
};


void DIS::ResetVars() {
  W = UNDEF;
  Q2 = UNDEF;
  Nu = UNDEF;
  x = UNDEF;
  y = UNDEF;
  for(int c=0; c<3; c++) eleVertex[c] = UNDEF;
  eleE = UNDEF;
  eleP = UNDEF;
  elePt = UNDEF;
  eleEta = UNDEF;
  elePhi = UNDEF;
  return;
};

