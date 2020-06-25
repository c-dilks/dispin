#ifndef Dihadron_
#define Dihadron_

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <map>
#include <vector>

// ROOT
#include "TSystem.h"
#include "TObject.h"
#include "TTree.h"
#include "TFile.h"
#include "TLorentzVector.h"
#include "TString.h"
#include "TMath.h"

// dihbsa
#include "Constants.h"
#include "Trajectory.h"
#include "DIS.h"
#include "Tools.h"




class Dihadron : public TObject
{
  public:
    Dihadron();
    ~Dihadron();

    Bool_t debug;
    Bool_t debugTheta;
    Bool_t useBreit;

    void CalculateKinematics(
      Trajectory * trajA, Trajectory * trajB, DIS * disEv);
    void ComputeAngles();
    Float_t PlaneAngle(TVector3 vA, TVector3 vB,
                       TVector3 vC, TVector3 vD);

    void ResetVars();

    Trajectory * hadron[2];
    DIS * disEv;

    TLorentzVector vecHad[2]; // hadron momentum
    TLorentzVector vecPh; // dihadron total momentum
    TLorentzVector vecR; // dihadron relative momentum
    TLorentzVector vecMmiss; // used to compute missing mass

    TLorentzVector disVecBeam;
    TLorentzVector disVecTarget;
    TLorentzVector disVecElectron;
    TLorentzVector disVecW;
    TLorentzVector disVecQ;


    // BRANCHES  ///////////////////////////////////////
    //
    Int_t pairType; // two digit number with Idx of each hadron (see Constants.h)
    Int_t hadIdx[2]; // Idx for each hadron (redundant, but useful)
    Int_t hadRow[2]; // row in REC::Particle
    Float_t hadE[2]; // hadron energy
    Float_t hadP[2]; // hadron momentum
    Float_t hadPt[2]; // hadron transverse momentum
    Float_t hadEta[2]; // hadron pseudorapidity
    Float_t hadPhi[2];  // hadron lab-frame azimuth
    Float_t PhMag; // dihadron total momentum
    Float_t PhPerpMag; // transverse component of dihadron total momentum (perp frame)
    Float_t PhEta; // pseudorapidity of dihadron pair
    Float_t PhPhi; // azimuth of dihadron pair

    Float_t RMag; // dihadron relative momentum
    Float_t RTMag; // transverse componet of relative momentum (T-frame)
    Float_t RPerpMag; // transverse componet of relative momentum (perp-frame)

    Float_t PhiH; // angle[ reaction_plane, Ph^q ]
    Float_t z[2]; // fraction of energy of fragmenting parton
                  // carried by the hadron
    Float_t zpair; // fraction of energy of fragmenting parton
                   // carried by the hadron pair
    Float_t Mh; // dihadron invariant mass
    Float_t hadM[2]; // hadron mass
    Float_t Mmiss; // missing mass
    Float_t xF; // feynman-x
    Float_t hadXF[2]; // feynman-x for each hadron
    Float_t hadVertex[2][3]; // vertex
    Float_t hadChi2pid[2]; // chi2 of PID
    Int_t hadStatus[2]; // status variable
    Float_t hadBeta[2]; // beta measured by TOF

    Float_t alpha; // dihadron opening angle
    Float_t zeta; // lab-frame energy sharing
    Float_t theta; // CoM-frame angle between Ph and P1
    Float_t thetaAlt; // alternative definiton of theta
    Float_t thetaLI; // lorentz-invariant theta

    // single-hadron PhiH
    Float_t GetSingleHadronPhiH(Int_t h_idx);


    // PhiR angle
    // defined a couple different ways since transverse components of R vary in
    // definition; there are two frames to consider (see arXiv:1707.04999):
    // -- perp-frame: "transverse" plane is normal to fragmenting
    //                quark, i.e., to q
    // -- T-frame: "transverse" plane is normal to Ph
    //

    Float_t PhiRq; // use R_perp computed via rejection w.r.t. q
    // -- COMPASS 1702.07317, but used vector rejection to get R_perp

    Float_t PhiRp; // use R_T computed via covariant kT formula
    // -- HERMES 0803.2367 angle, but used Matevosyan et al 1707.04999

    Float_t PhiRp_r; // use R_T computed via rejection w.r.t. Ph (not frame independent)
    // -- HERMES 0803.2367 angle

    Float_t PhiRp_g; // use R_T computed by projection operator "g_T", following
    // equation 9 in 1408.5721 (gliske, bacchetta, radici)

    Float_t PhiR; // preferred definition
    
    //
    ///////////////////////////////////////////////////

  private:
    int h;

    TVector3 pQ,pL,pPh,pR;
    TVector3 pHad[2];
    TVector3 pHad_Perp[2];
    TVector3 pPh_Perp;

    TVector3 pR_Perp;
    TVector3 pR_T_byKt;
    TVector3 pR_T_byRej;
    TVector3 pR_T_byProj;
    TLorentzVector vecR_T_byProj;

    Float_t xi,ratio;

    TVector3 crossAB,crossCD;
    Float_t sgn,numer,denom;

    TLorentzVector vecPh_com; // P+q COM frame Ph
    TLorentzVector vecPh_breit; // breit frame Ph
    TLorentzVector disVecQ_com; // P+q COM frame Q
    TLorentzVector vecHad_com[2]; // P+q COM frame hadron momenta
    TLorentzVector vecHad_dihCom[2]; // dihadron COM frame hadron momenta
    TVector3 pHad_com[2];
    TVector3 pHad_dihCom[2];
    TVector3 pPh_com,pPh_breit;
    TVector3 pQ_com;

    TVector3 dihComBoost;

    Float_t MRterm[2];




  ClassDef(Dihadron,1);
};

#endif
