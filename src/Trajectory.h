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
#include "TTree.h"
#include "TFile.h"
#include "TLorentzVector.h"
#include "TVector3.h"
#include "TString.h"

// dihbsa
#include "Constants.h"
#include "Tools.h"



class Trajectory
{
  public:
    Trajectory();
    ~Trajectory();

    // members
    TLorentzVector Momentum;
    TVector3 Vertex;
    Float_t chi2pid;
    Int_t Status;
    Float_t Beta;
    Int_t Row;


    // use Idx to set what type of particle this is, which
    // only matters if you use the following accessors
    Int_t Idx;
    TString Name() { return PartName(Idx); };
    TString Title() { return PartTitle(Idx); };
    Int_t PID() { return PartPID(Idx); };
    Float_t Mass() { return PartMass(Idx); };
    ////////////////////////////////////////////////////////


    // calcuate momentum transverse to q, denoted "Ptq" or "pTq"
    Float_t Ptq(TLorentzVector q_);



    Bool_t debug;
    
  private:


  ClassDef(Trajectory,1);
};

#endif
