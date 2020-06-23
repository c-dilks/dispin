#include "Dihadron.h"

ClassImp(Dihadron)

using namespace std;


Dihadron::Dihadron() {
  debug = false;
  debugTheta = false;
  useBreit = false;

  for(h=0; h<2; h++) vecHad[h] = TLorentzVector(0,0,0,0);
  vecPh = TLorentzVector(0,0,0,0);
  vecR = TLorentzVector(0,0,0,0);

  //printf("Dihadron instantiated\n");
};


void Dihadron::SetEvent(
  Trajectory * trajA, 
  Trajectory * trajB,
  DIS * disEvent
) {
  hadron[qA] = trajA;
  hadron[qB] = trajB;
  disEv = disEvent;


  // get disEv vectors
  disVecBeam = disEv->vecBeam;
  disVecTarget = disEv->vecTarget;
  disVecElectron = disEv->vecElectron;
  disVecW = disEv->vecW;
  disVecQ = disEv->vecQ;
  if(useBreit) {
    disVecBeam.Boost(disEv->BreitBoost);
    disVecTarget.Boost(disEv->BreitBoost);
    disVecElectron.Boost(disEv->BreitBoost);
    disVecW.Boost(disEv->BreitBoost);
    disVecQ.Boost(disEv->BreitBoost);
  };


  // compute 4-momenta Ph and R
  for(h=0; h<2; h++) {
    vecHad[h] = hadron[h]->Vec;
    if(useBreit) vecHad[h].Boost(disEv->BreitBoost);
  };
  vecPh = vecHad[qA] + vecHad[qB];
  vecR = 0.5 * ( vecHad[qA] - vecHad[qB] );


  // get 3-momenta from 4-momenta
  pQ = disVecQ.Vect();
  pL = disVecElectron.Vect();
  pPh = vecPh.Vect();
  pR = vecR.Vect();
  for(h=0; h<2; h++) pHad[h] = vecHad[h].Vect();



  // compute z
  for(h=0; h<2; h++) z[h] = disVecTarget.Dot(vecHad[h]) / disVecTarget.Dot(disVecQ);
  zpair = disVecTarget.Dot(vecPh) / disVecTarget.Dot(disVecQ);

  // compute invariant mass (and hadron masses)
  Mh = vecPh.M();
  for(h=0; h<2; h++) hadM[h] = vecHad[h].M();
  

  // compute missing mass
  vecMmiss = disVecW - vecPh;
  Mmiss = vecMmiss.M();

  // compute opening angle
  alpha = Tools::AngleSubtend(pHad[qA],pHad[qB]);

  // compute xF
  vecPh_com = vecPh;
  disVecQ_com = disVecQ;
  for(h=0; h<2; h++) vecHad_com[h] = vecHad[h];
  // -- (if in Breit frame, boost to lab frame)
  if(useBreit) { 
    vecPh_com.Boost(-1*(disEv->BreitBoost));
    disVecQ_com.Boost(-1*(disEv->BreitBoost));
    for(h=0; h<2; h++) vecHad_com[h].Boost(-1*(disEv->BreitBoost));
  };
  // -- boost to CoM frame
  vecPh_com.Boost(disEv->ComBoost);
  disVecQ_com.Boost(disEv->ComBoost);
  for(h=0; h<2; h++) vecHad_com[h].Boost(disEv->ComBoost);

  pPh_com = vecPh_com.Vect();
  pQ_com = disVecQ_com.Vect();
  for(h=0; h<2; h++) pHad_com[h] = vecHad_com[h].Vect();

  xF = 2 * pPh_com.Dot(pQ_com) / (disEv->W * pQ_com.Mag());
  for(h=0; h<2; h++)
    hadXF[h] = 2 * pHad_com[h].Dot(pQ_com) / (disEv->W * pQ_com.Mag());


  // get vertex and chi2pid
  for(h=0; h<2; h++) {
    hadVertex[h][eX] = (hadron[h]->Vertex).X();
    hadVertex[h][eY] = (hadron[h]->Vertex).Y();
    hadVertex[h][eZ] = (hadron[h]->Vertex).Z();
    hadChi2pid[h] = hadron[h]->chi2pid;
    hadStatus[h] = hadron[h]->Status;
    hadBeta[h] = hadron[h]->Beta;
  };


  // compute theta
  // -- boost hadron momenta to dihadron CoM frame
  dihComBoost = -1*vecPh.BoostVector();
  for(h=0; h<2; h++) {
    vecHad_dihCom[h] = vecHad[h];
    if(useBreit) vecHad_dihCom[h].Boost(-1*(disEv->BreitBoost));
    vecHad_dihCom[h].Boost(dihComBoost);
    pHad_dihCom[h] = vecHad_dihCom[h].Vect();
  };

  // -- in this frame, Ph=0, and the boost direction was along Ph
  // -- calculate theta as the angle between boosted hadron momentum
  //    and original Ph direction
  theta = Tools::AngleSubtend(pHad_dihCom[qA],pPh);
  //thetaAlt = Tools::AngleSubtend(pHad_dihCom[qB],pPh); // == PI - theta
  thetaAlt = Tools::AngleSubtend(pHad_dihCom[qA],pPh_com); // timothy's theta
  // -- test calculation via breit frame Ph
  /*
  vecPh_breit = vecPh;
  if(!useBreit) vecPh_breit.Boost(disEv->BreitBoost);
  pPh_breit = vecPh_breit.Vect();
  thetaAlt = Tools::AngleSubtend(pHad_dihCom[qA],pPh_breit); 
  */


  // calculate lorentz invariant theta (suggestion from Alessandro)
  // -- first compute zeta
  zeta = 2 * vecR.Dot(disVecTarget) / ( vecPh.Dot(disVecTarget) );
  // -- then compute sqrt(M_k^2-|R|^2) for k=1,2
  for(h=0; h<2; h++) 
    MRterm[h] = TMath::Sqrt( vecHad[h].M2() + pR.Mag2() );
  // -- then compute theta
  thetaLI = TMath::ACos(  ( MRterm[qA] - MRterm[qB] - Mh*zeta ) / ( 2*pR.Mag() )  );

  //printf("MRterm[qA]=%f  MRterm[qB]=%f\n",MRterm[qA],MRterm[qB]);
  //printf("vecHad[qA].M=%f  vecHad[qB].M=%f\n",vecHad[qA].M(),vecHad[qB].M());
  //printf("pRmag = %f\n",pR.Mag());
  //printf("theta=%f  thetaAlt=%f  thetaLI=%f  zeta=%f\n",
    //theta,thetaAlt,thetaLI,zeta);



  if(debugTheta) {
    printf("========== theta calculation:\n");
    for(h=0; h<2; h++) {
      printf("lab frame hadron %d: ",h); vecHad[h].Print();
    };
    printf("lab frame dihadron: "); vecPh.Print(); printf("---\n");
    for(h=0; h<2; h++) {
      printf("com frame hadron %d: ",h); vecHad_dihCom[h].Print();
    };
    printf("com frame dihadron: ");
    (vecHad_dihCom[qA]+vecHad_dihCom[qB]).Print(); // should have p=0, E=Mh
    printf("---\n");
    printf("Mh = %f\n",Mh); // should equal energy component of 4-momenta sum
    printf("theta = %f\n\n",theta);
  };


  // compute angles
  ComputeAngles();
};


void Dihadron::ComputeAngles() {
  // sign convention:
  //   P_1 = pi+ momentum
  //   P_2 = pi- momentum
  //  
  // regarding transverse components, there are two 
  // frames to consider (see arXiv:1707.04999):
  // -- perp-frame: "transverse" plane is normal to fragmenting
  //                quark, i.e., to q
  // -- T-frame: "transverse" plane is normal to Ph


  // first compute PhiH angle
  PhiH = PlaneAngle(pQ,pL,pQ,pPh);


  // then compute PhiR angle (all the ways)
  //
  

  // perp-frame hadron 3-momenta
  for(h=0; h<2; h++) pHad_Perp[h] = Tools::Reject(pHad[h],pQ);
  pPh_Perp = Tools::Reject(pPh,pQ);


  // compute transverse components of R in different frames
  // (and with different equations)
  // -- in T-frame, via kT relation (following 1707.04999)
  pR_T_byKt = 1 / ( z[qA] + z[qB] ) *  
              ( z[qB] * pHad_Perp[qA]  -  z[qA] * pHad_Perp[qB] );

  // -- in T-frame, via rejection
  pR_T_byRej = Tools::Reject(pR,pPh);

  // -- in perp-frame, via rejection
  pR_Perp = Tools::Reject(pR,pQ);

  // -- by projection operator (eq. 9 in 1408.5721)
  xi = 2 * vecR.Dot(disVecTarget) / ( vecPh.Dot(disVecTarget) );
  ratio = (xi*Mh*Mh - (hadM[qA]*hadM[qA]-hadM[qB]*hadM[qB])) / ( (disEv->Q2) * zpair );
  vecR_T_byProj = vecR - (xi/2.0)*vecPh + (disEv->x) * ratio * disVecTarget;
  pR_T_byProj = vecR_T_byProj.Vect();


  // momentum magnitudes
  PhMag = pPh.Mag(); // dihadron total momentum Ph
  PhPerpMag = pPh_Perp.Mag(); // trans. comp. of Ph (perp frame, obviously)
  RMag = pR.Mag(); // dihadron relative momentum R
  RTMag = pR_T_byKt.Mag(); // trans. comp. of R (perp frame)
  RPerpMag = pR_Perp.Mag(); // trans. comp. of R (perp frame)


  // eta & phi of dihadron
  if(PhMag>0 && PhPerpMag>0) {
    PhEta = pPh.Eta();
    PhPhi = pPh.Phi();
  } else {
    PhEta = UNDEF;
    PhPhi = UNDEF;
  };



  // -- COMPASS 1702.07317, but used vector rejection to get R_perp
  PhiRq = PlaneAngle(pQ,pL,pQ,pR_Perp);

  // -- HERMES 0803.2367 angle, but used Matevosyan et al 1707.04999
  //    to obtain R_T vector
  PhiRp = PlaneAngle(pQ,pL,pQ,pR_T_byKt);

  // -- HERMES 0803.2367 angle
  PhiRp_r = PlaneAngle(pQ,pL,pQ,pR_T_byRej);

  // equation 9 in 1408.5721 (gliske, bacchetta, radici)
  PhiRp_g = PlaneAngle(pQ,pL,pQ,pR_T_byProj);

  // set preferred PhiR definition
  PhiR = PhiRp;
};


// MUST BE CALLED *AFTER* SetEvent()
// returns "PhiH" angle for hadron h_idx
Float_t Dihadron::GetSingleHadronPhiH(Int_t h_idx) {
  if(h_idx==qA || h_idx==qB) return PlaneAngle(pQ,pL,pQ,pHad[h_idx]);
  else {
    fprintf(stderr,"ERROR: bad h_idx in GetSingleHadronPhiH\n");
    return UNDEF;
  }
};




// compute angle between planes given by
// vA x vB and vC x vD
// the angle is with respect to vA x vB
Float_t Dihadron::PlaneAngle(
  TVector3 vA, TVector3 vB,
  TVector3 vC, TVector3 vD
) {
  crossAB = vA.Cross(vB); // AxB
  crossCD = vC.Cross(vD); // CxD

  sgn = crossAB.Dot(vD); // (AxB).D

  if(fabs(sgn)<0.00001) {
    //fprintf(stderr,"WARNING: Dihadron::PlaneAngle (AxB).D == 0\n");
    return UNDEF;
  };

  sgn /= fabs(sgn); // sign of (AxB).D

  numer = crossAB.Dot(crossCD); // (AxB).(CxD)
  denom = crossAB.Mag() * crossCD.Mag(); // |AxB|*|CxD|

  if(fabs(denom)<0.00001) {
    //fprintf(stderr,"WARNING: Dihadron::PlaneAngle |AxB|*|CxD| == 0\n");
    return UNDEF;
  };

  return sgn * TMath::ACos(numer/denom);
};


void Dihadron::ResetVars() {
  Mh = UNDEF;
  Mmiss = UNDEF;
  for(h=0; h<2; h++) z[h] = UNDEF;
  zpair = UNDEF;
  xF = UNDEF;
  for(h=0; h<2; h++) {
    hadXF[h] = UNDEF;
    hadChi2pid[h] = UNDEF;
    hadStatus[h] = UNDEF;
    hadBeta[h] = UNDEF;
    for(int c=0; c<3; c++) hadVertex[h][c] = UNDEF;
  };
  alpha = UNDEF;

  PhMag = UNDEF;
  PhPerpMag = UNDEF;
  PhEta = UNDEF;
  PhPhi = UNDEF;

  RMag = UNDEF;
  RPerpMag = UNDEF;
  RTMag = UNDEF;

  PhiH = UNDEF;

  PhiRq = UNDEF;
  PhiRp = UNDEF;
  PhiRp_r = UNDEF;
  PhiRp_g = UNDEF;
};


  

Dihadron::~Dihadron() {
};

