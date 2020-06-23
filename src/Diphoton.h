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

// dihbsa
#include "Constants.h"
#include "Trajectory.h"



class Diphoton : public TObject
{
  public:
    Diphoton();
    ~Diphoton();

    Bool_t debug;

    void SetEvent(Trajectory * traj1, Trajectory * traj2);
    void ResetVars();

    Trajectory * photon[2]; // photon trajectories
    Trajectory * Traj; // diphoton trajectory

    // photon kinematics
    Float_t photE[2];
    Float_t photPt[2];
    Float_t photEta[2];
    Float_t photPhi[2];
    Float_t photVertex[2][3];
    Float_t photChi2pid[2];

    // diphoton variables
    Float_t E; // energy of the pair
    Float_t Z; // energy sharing (E1-E2)/E
    Float_t Pt; // transverse momentum
    Float_t M; // invariant mass
    Float_t Alpha; // diphoton opening angle
    Float_t Eta; // pseudorapidity
    Float_t Phi; // azimuth

    // booleans
    Bool_t validDiphoton;



  private:

    TLorentzVector vecDiphoton;
    TVector3 momPhoton[2];



  ClassDef(Diphoton,1);
};

#endif
