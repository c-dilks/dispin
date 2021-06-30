#ifndef Diphoton_
#define Diphoton_

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
#include "TString.h"
#include "TMath.h"
#include "TLorentzVector.h"

// dispin
#include "Constants.h"
#include "Trajectory.h"
#include "DIS.h"


class Diphoton : public TObject
{
  public:
    Diphoton();
    ~Diphoton();

    Bool_t debug;

    void CalculateKinematics(
      Trajectory * trajA, Trajectory * trajB, DIS * disEv);
    void Classify();
    void ResetVars();
    void ResetBools();
    Trajectory * GetDiphotonTraj() { return diphot; };
    Trajectory * GetPhotonTraj(Int_t h) { return photon[h]; };

    // photon kinematics
    Float_t photE[2]; // energy
    Float_t photPt[2]; // transverse momentum (lab frame)
    Float_t photEta[2]; // pseudorapidity
    Float_t photPhi[2]; // azimuth
    Float_t photVertex[2][3];
    Float_t photChi2pid[2];
    Float_t photBeta[2];
    Float_t photAng[2]; // angle(electron,photon), in degrees

    // diphoton variables
    Float_t E; // energy of the pair
    Float_t Pt; // transverse momentum (lab frame)
    Float_t Eta; // pseudorapidity
    Float_t Phi; // azimuth
    Float_t ZE; // energy imbalance (E1-E2)/E
    Float_t M; // invariant mass
    Float_t VtxDiff; // distance between photon verteces

    // diphoton / pi0 cuts
    Bool_t cutPhotBeta;
    Bool_t cutPhotEn;
    Bool_t cutPhotTheta;
    Bool_t cutPhotEleAng;
    Bool_t cutMassPi0;
    Bool_t cutMassSB;
    Bool_t cutBasic;

    // diphoton classifier
    Int_t diphotClass;
    enum diphotClass_enum { dpNull, dpPi0, dpSB, dpIgnore };
    Int_t GetIdx() { return diphot->Idx; };
    /* dpNull: not a diphoton
     * dpPi0: likely a pi0
     * dpSB: sideband region (for BG estimate)
     * dpIgnore: neither pi0 or sideband, and/or did not
     *           satisfy basic cuts, such as minimum photon E
     */


  private:

    // trajectories
    Trajectory * photon[2]; // photon trajectories
    Trajectory * diphot; // diphoton trajectory

    Float_t photTheta[2]; // polar scattering angle


  ClassDef(Diphoton,1);
};

#endif
