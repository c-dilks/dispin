#ifndef Trajectory_
#define Trajectory_

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
#include "FiducialCuts.h"
#include "TTree.h"
#include "TFile.h"
#include "TLorentzVector.h"
#include "TVector3.h"
#include "TString.h"

// dihbsa
#include "Constants.h"
#include "Tools.h"



class Trajectory : public FiducialCuts
{
  public:
    Trajectory(Int_t particle_index);
    ~Trajectory();

    void SetMomentum(Float_t px, Float_t py, Float_t pz);
    void SetVec(TLorentzVector v_) { Vec=v_; };
    void SetIdx(Int_t particle_index) { Idx = particle_index; };
    TString Name() { return PartName(Idx); };
    TString Title() { return PartTitle(Idx); };
    Int_t PID() { return PartPID(Idx); };
    Float_t Mass() { return PartMass(Idx); };
    Float_t Ptq(TLorentzVector q_);


    Int_t Idx;
    TLorentzVector Vec;

    TVector3 Vertex;
    void SetVertex(Float_t vx, Float_t vy, Float_t vz) { Vertex.SetXYZ(vx,vy,vz); };
    Float_t chi2pid;
    Int_t Status;
    Float_t Beta;


    Bool_t debug;
    
  private:


  ClassDef(Trajectory,1);
};

#endif
