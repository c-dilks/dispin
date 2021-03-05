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
#include "TRandomGen.h"

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

    void GetEvent(Long64_t i);
    Bool_t Valid();

    Int_t SpinState();
    Float_t Polarization();
    Float_t Rellum();
    Bool_t CheckVertex();
    Bool_t CheckSampFrac();
    Bool_t CheckHadChi2pid(Int_t had);
    Bool_t CheckMissingMass();

    void PrintEventVerbose();
    void PrintEvent();
    void PrintEventLine();

    Dihadron * GetDihadronObj();
    DIS * GetDISObj();

    //Float_t GetDepolarizationFactorApprox(Char_t kf);
    Float_t GetDepolarizationFactor(Char_t kf);

    Bool_t debug;
    Long64_t ENT;


    ///////////////////////////
    //   BRANCHES
    ///////////////////////////
    // DIS kinematics
    Float_t W,Q2,Nu,x,y,gamma,epsilon;

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
    Float_t hadYH[2];
    Float_t hadPperp[2];
    Float_t hadQt[2];
    Float_t hadVertex[2][3];
    Int_t hadStatus[2];
    Float_t hadChi2pid[2];
    Float_t hadBeta[2];

    // electron kinematics
    Float_t eleE;
    Float_t eleP;
    Float_t elePt;
    Float_t eleEta;
    Float_t eleTheta;
    Float_t elePhi;
    Float_t eleVertex[3];
    Int_t eleStatus;
    Float_t eleChi2pid;
    Float_t elePCALen,eleECINen,eleECOUTen;
    Int_t eleSector;

    // fiducial cuts
    Bool_t eleFiduCut;
    Bool_t hadFiduCut[2];

    // dihadron kinematics
    Float_t Mh,Zpair,PhiH,Mmiss,xF,alpha,YH;
    Float_t Z[2];
    Float_t zeta;
    Float_t theta;

    Float_t Ph,PhPerp;
    Float_t PhEta,PhPhi;
    Float_t R,RPerp,RT;

    // event-level branches
    Int_t evnum,runnum;
    Int_t helicity;
    static const Int_t NhelicityMC = 73;
    Int_t helicityMC[NhelicityMC];

    // PhiR 
    Float_t PhiR; // set to the preferred one
    Float_t PhiRq;
    Float_t PhiRp;
    Float_t PhiRp_r;
    Float_t PhiRp_g;

    Float_t PhiHR; // PhiH-PhiR

    // DSIDIS angles
    Float_t hadPhiH[2];
    Float_t PhiD; // hadPhiH[qA] - hadPhiH[qB]
    Float_t yhb;



    ///////////////////////////
    //   EventCuts
    ///////////////////////////
    Bool_t cutQ2,cutW,cutY,cutDIS,cutFR;
    Bool_t cutCFR[2];
    Bool_t cutTFR[2];
    Bool_t cutDihadron;
    Bool_t cutHelicity;
    Bool_t cutVertex;
    Bool_t cutFiducial;
    Bool_t cutPID;
    Bool_t cutElePID;
    Bool_t cutHadPID[2];


    // tree banches used for matching MCgen event
    Int_t whichHelicityMC;
    Bool_t MCrecMode;
    Float_t gen_W;
    Float_t gen_Q2;
    Float_t gen_Nu;
    Float_t gen_x;
    Float_t gen_y;
    // - generated electron kinematics branches
    Float_t gen_eleE;
    Float_t gen_eleP;
    Float_t gen_elePt;
    Float_t gen_eleEta;
    Float_t gen_eleTheta;
    Float_t gen_elePhi;
    Float_t gen_eleVertex[3];
    // - generated hadron branches
    Int_t gen_pairType;
    Int_t gen_hadRow[2];
    Int_t gen_hadIdx[2];
    Float_t gen_hadE[2];
    Float_t gen_hadP[2];
    Float_t gen_hadPt[2];
    Float_t gen_hadEta[2];
    Float_t gen_hadTheta[2];
    Float_t gen_hadPhi[2];
    Float_t gen_hadXF[2];
    Float_t gen_hadVertex[2][3];
    // - generated dihadron branches
    Float_t gen_Mh;
    Float_t gen_Mmiss;
    Float_t gen_Z[2];
    Float_t gen_Zpair;
    Float_t gen_xF;
    Float_t gen_alpha;
    Float_t gen_theta;
    Float_t gen_zeta;
    Float_t gen_Ph;
    Float_t gen_PhPerp;
    Float_t gen_PhEta;
    Float_t gen_PhPhi;
    Float_t gen_R;
    Float_t gen_RPerp;
    Float_t gen_RT;
    Float_t gen_PhiH;
    Float_t gen_PhiRq;
    Float_t gen_PhiRp;
    Float_t gen_PhiRp_r;
    Float_t gen_PhiRp_g;
    // - match quality
    Bool_t gen_eleIsMatch;
    Bool_t gen_hadIsMatch[2];
    Float_t gen_eleMatchDist;
    Float_t gen_hadMatchDist[2];
    // - other
    Int_t gen_hadParentIdx[2];
    Int_t gen_hadParentPid[2];

    Float_t eleSampFrac;

    Bool_t vzBoolEle;
    Bool_t vzdiffBool;
    Float_t vzdiff[2];

  private:
    TChain * chain;
    Int_t whichHad[2];

    Dihadron * objDihadron;
    Dihadron * candDih;
    DIS * objDIS;
    Trajectory * trEle;
    Trajectory * trHad[2];

    TLorentzVector hadMom[2];
    TLorentzVector eleMom;
    TLorentzVector hadMomBreit[2];
    TLorentzVector vmMomBreit;
    TVector3 breitVec;


    Bool_t vertexWarned;
    Bool_t sfcutDiag, sfcutSigma;
    Float_t dfA;

    Bool_t helicityMCinjected;
    TRandom * RNG;
    Int_t sps;


  ClassDef(EventTree,1);
};

#endif
