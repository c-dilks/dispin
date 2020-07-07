#ifndef EventTree_
#define EventTree_

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
#include "TChain.h"
#include "TFile.h"
#include "TString.h"
#include "TMath.h"
#include "TRandom.h"

// dihbsa
#include "Constants.h"
#include "Trajectory.h"
#include "FiducialCuts.h"
#include "DIS.h"
#include "Dihadron.h"
#include "Tools.h"




class EventTree : public TObject
{
  public:
    EventTree(TString filelist, Int_t whichPair_);
    ~EventTree();

    void GetEvent(Int_t i);
    Bool_t Valid();

    Int_t SpinState();
    Float_t Polarization();
    Bool_t CheckVertex();

    void PrintEventVerbose();
    void PrintEvent();
    void PrintEventLine();

    Dihadron * GetDihadronObj();
    DIS * GetDISObj();

    Float_t GetKinematicFactor(Char_t kf);

    /*
    Bool_t BuildMatchTable();
    Bool_t FindEvent(Int_t evnum_, Dihadron * queryDih);
    */

    Bool_t debug;
    Long64_t ENT;


    ///////////////////////////
    //   BRANCHES
    ///////////////////////////
    // DIS kinematics
    Float_t W,Q2,Nu,x,y;

    // hadron kinematics
    Int_t pairType;
    Int_t hadIdx[2];
    Float_t hadE[2];
    Float_t hadP[2];
    Float_t hadPt[2];
    Float_t hadPtq[2];
    Float_t hadEta[2];
    Float_t hadTheta[2];
    Float_t hadPhi[2];
    Float_t hadXF[2];
    Float_t hadVertex[2][3];
    Int_t hadStatus[2];
    Float_t hadChi2pid[2];
    //Bool_t hadFidPCAL[2], hadFidDC[2];

    // electron kinematics
    Float_t eleE;
    Float_t eleP;
    Float_t elePt;
    Float_t eleEta;
    Float_t elePhi;
    Float_t eleVertex[3];
    Int_t eleStatus;
    Float_t eleChi2pid;

    // fiducial cuts
    Bool_t eleFiduCut[FiducialCuts::nLevel];
    Bool_t hadFiduCut[2][FiducialCuts::nLevel];

    // dihadron kinematics
    Float_t Mh,Zpair,PhiH,Mmiss,xF,alpha;
    Float_t Z[2];
    Float_t zeta;
    Float_t theta;

    Float_t Ph,PhPerp;
    Float_t PhEta,PhPhi;
    Float_t R,RPerp,RT;

    // event-level branches
    Int_t evnum,runnum;
    Int_t helicity;
    static const Int_t NhelicityMC = 12;
    Int_t helicityMC[NhelicityMC];

    // PhiR 
    Float_t PhiR; // set to the preferred one
    Float_t PhiRq;
    Float_t PhiRp;
    Float_t PhiRp_r;
    Float_t PhiRp_g;

    Float_t PhiHR; // PhiH-PhiR

    /*
    Float_t b_PhiRq;
    Float_t b_PhiRp;
    Float_t b_PhiRp_r;
    Float_t b_PhiRp_g;
    */



    ///////////////////////////
    //   EventCuts
    ///////////////////////////
    Bool_t cutQ2,cutW,cutY,cutDIS;
    Bool_t cutDihadron;
    Bool_t cutHelicity;
    Bool_t cutVertex;
    Bool_t cutFiducial;
    Bool_t cutMCmatch;

    // OTHER VARIABLES
    Int_t whichHelicityMC;

    /*
    // variables used for MCrec and MCgen matching
    Float_t MD,MDmin;
    Int_t iiFound;
    Float_t queryTheta[2];
    Float_t queryPhi[2];
    Float_t candTheta[2];
    Float_t candPhi[2];
    */

    // tree banches used for matching MCgen event
    Bool_t MCrecMode;
    Float_t matchDiff;
    Float_t gen_eleE;
    Float_t gen_elePt;
    Float_t gen_eleEta;
    Float_t gen_elePhi;
    Float_t gen_hadE[2]; // kinematics for matching MCgen hadron
    Float_t gen_hadPt[2];
    Float_t gen_hadEta[2];
    Float_t gen_hadTheta[2];
    Float_t gen_hadPhi[2];
    
  private:
    TChain * chain;
    Int_t whichHad[2];
    Int_t whichLevel;

    TRandom * RNG;

    Dihadron * objDihadron;
    Dihadron * candDih;
    DIS * objDIS;
    Trajectory * trEle;
    Trajectory * trHad[2];

    TLorentzVector hadMom[2];
    TLorentzVector eleMom;

    Bool_t vzBoolEle;
    Bool_t vzBoolHad[2];
    Bool_t vrBool;
    Bool_t vzdiffBool;
    Float_t vzdiffHads;
    Float_t vzdiff[2];

    /*
    std::map<Int_t,std::vector<Int_t>> evnumMap;
    std::vector<Int_t>iList;
    bool inserted;
    Int_t evnumTmp;
    */


  ClassDef(EventTree,1);
};

#endif
