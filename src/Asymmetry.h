#ifndef Asymmetry_
#define Asymmetry_

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <math.h>
#include <map>
#include <thread>

// ROOT
#include "TSystem.h"
#include "TStyle.h"
#include "TObject.h"
#include "TTree.h"
#include "TChain.h"
#include "TFile.h"
#include "TLorentzVector.h"
#include "TString.h"
#include "TMath.h"
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "TF1.h"
#include "TF2.h"
#include "TGraphErrors.h"
#include "TGraph2DErrors.h"
#include "TLine.h"
#include "TRegexp.h"

// RooFit
#include <RooGlobalFunc.h>
#include <RooGenericPdf.h>
#include <RooFitResult.h>
#include <RooExtendPdf.h>
#include <RooAbsReal.h>
#include <RooArgSet.h>
#include <RooDataSet.h>
#include <RooRealVar.h>
#include <RooCategory.h>
#include <RooDataHist.h>
#include <RooSimultaneous.h>
#include <RooAddPdf.h>
#include <RooPlot.h>
#include <RooNLLVar.h>


// dispin
#include "Constants.h"
#include "Tools.h"
#include "Modulation.h"
#include "Trajectory.h"
#include "DIS.h"
#include "Binning.h"
#include "EventTree.h"
#include "InjectionModel.h"


class Asymmetry : public TObject
{
  public:
    Asymmetry() {}; // empty default constructor for streaming instances to ROOT files
    Asymmetry(Binning * binScheme,  Int_t binNum);
    ~Asymmetry();


    void SetFitMode(Int_t fitMode);
    void FitAsymGraph();
    void FitAsymMLM();
    void SetAsymGrPoint(Int_t modBin_, Int_t modBin2_=-1);
    void FormuAppend(Int_t TW, Int_t L, Int_t M,
      Int_t lev=0, Int_t polarization=Modulation::kLU);
    void DenomAppend(Int_t TW, Int_t L, Int_t M, Int_t lev);
    Double_t DenomEval(Float_t phiR_, Float_t phiH_, Float_t theta_);


    Bool_t AddEvent(EventTree * ev);
    Float_t EvalModulation();
    Float_t EvalWeight();
    Float_t MeanDepolarization(Int_t amp);
    Bool_t KickEvent(TString reason,Float_t badValue);

    void ResetVars();
    void PrintSettings();

    void StreamData(TFile * tf);
    void AppendData(TFile * tf);
    TString AppFileName(TFile * tf);

    void ActivateTree(Bool_t isMC=false, InjectionModel *IM_=nullptr);

    Double_t nEvents;

    Bool_t success;
    Bool_t successIVmode;
    Bool_t debug;
    Bool_t extendMLM;
    Double_t yieldLimit;

    Binning * BS;

    // settings
    Int_t oaTw,oaL,oaM;
    Int_t whichDim;
    Int_t gridDim;
    Bool_t useWeighting;

    // fixed polarization value used for OA fits only
    // - this value is the weighted average of the polarizations from
    //   pass1 inbending data, for before and after the Wien angle change
    // - this is *not* used by the MLM fit
    static constexpr Float_t polOA = 0.863;



    // event-level variables -- these must be set for each event
    Float_t Mh;
    Float_t Mmiss;
    Float_t x;
    Float_t z;
    Float_t PhiH;
    Float_t PhiR;
    Float_t PhiD;
    Float_t PhPerp;
    Float_t Ph;
    Float_t Q2;
    Float_t y;
    Float_t xF;
    Float_t YH[2];
    Float_t DY,DYsgn;
    Float_t diphM;
    Float_t theta;
    Int_t spinn;
    Float_t pol,polErr;
    Float_t depol2,depol3;
    

    // number of bins
    static const Int_t iv1Bins = 100; // number of bins for ivDist1 plots
    static const Int_t iv2Bins = 50; // number of bins for ivDist2 plots
    static const Int_t iv3Bins = 30; // number of bins for ivDist3 plots
    static const Int_t nModBins = 7; // number of bins in azimuthal modulation
    static const Int_t nModBins2 = 8; // number of bins in 2d azimuthal modulation

    Float_t modMin,modMax,aziMax;
    Float_t weight;


    // "iv dist": finely-binned IV distribution (for each whichDim)
    TH1D * ivDist1;
    TH2D * ivDist2;
    TH3D * ivDist3;
    TString ivName,ivTitle;

    // "azimuthal modulation dist" filled with, e.g., Sin(phiR) for each spin, binned
    // for the asymmetry plots
    TH1D * aziDist[nSpin];
    TH2D * aziDist2[nSpin]; // for 2d modulations
    TString aziName[nSpin];
    TString aziTitle[nSpin];
    TH2D * aziDistCorrTest2[nSpin]; // for testing asymmetry correlations
    TString aziCorrTestName[nSpin];
    TString aziCorrTestTitle[nSpin];

    // "finely-binnd azimuthal modulation dist" filled for all spins, used for getting
    // mean value of azimuthal modulation 
    TH1D * modBinDist[nModBins]; // one for each modulation bin
    TH1D * modDist; // for all modulation bins
    TH2D * IVvsModDist;
    TString modName,modTitle;
    TString modBinName;
    TString modBinTitle;
    TString IVvsModName,IVvsModTitle;
    // for 2d modulations:
    TH2D * modBinDist2[nModBins2][nModBins2];
    TH2D * modDist2;

    // yield dist (for computing rellum)
    TH1D * yieldDist;

    // dp dist (for computing <K(y)>, mean depolarization ratio)
    enum DP_enum {dpA, dpB, dpC, dpV, dpW, dpWA, dpVA, dpCA, dpBA, Ndp};
    TH1D * dpDist[Ndp];
    Float_t dpVal[Ndp];

    // denom dist (for studying sigma_UU partial waves systematic; this
    // distribution is only used externally)
    TH1D * denomDist;
    Float_t denomLB,denomUB;

    // asymmetry vs. azimuthal modulation bin
    TGraphErrors * asymGr;
    TString asymName,asymTitle;
    TF1 * fitFunc;
    TString fitFuncName;
    // for 2d modulations:
    TGraph2DErrors * asymGr2;
    TH2D * asymGr2hist;
    TF2 * fitFunc2;
    TString fitFunc2formu;

    TString oaModulationTitle,oaModulationName;



    
    // variables for each dimension
    Int_t I[3]; // IV index number
    Int_t B[3]; // IV bin number
    Float_t iv[3]; // IV
    Float_t ivMin[3]; // IV minimum value (for histo ranges)
    Float_t ivMax[3]; // IV maximum value (for histo ranges)
    TString ivN[3]; // IV name
    TString ivT[3]; // IV title
    TString binT,binN; // bin title/name suffixes
    TString aName;

    Double_t rNumer,rDenom,rellum,rellumErr;
    Double_t average_rellum;


    TTree * tree;

    // RooFit variables
    RooDataSet * rfData;
    RooGenericPdf * rfPdf[2];
    RooExtendPdf * rfExtPdf[2];
    RooSimultaneous * rfSimPdf;
    RooCategory * rfSpinCateg;
    //RooFitResult * rfResult;
    RooArgSet * rfVars;
    RooArgSet * rfParams[2];
    TString rfPdfFormu[2];
    TString rfSpinName[2];

    TString asymFormu;
    TString denomFormu;
    TString rellumFactor[2];
    Float_t rfParamRange;

    RooRealVar *rfPhiH, *rfPhiR, *rfPhiD, *rfTheta;
    RooRealVar *rfWeight;
    RooRealVar *rfPol, *rfRellum;
    RooRealVar *rfIV[Binning::nIV];

    static const Int_t nAmp = 15;
    Int_t nAmpUsed;
    TString rfAname[nAmp];
    RooRealVar *rfA[nAmp];

    static const Int_t nDparam = 12;
    Int_t nDparamUsed;
    TString rfDname[nDparam];
    RooRealVar *rfD[nDparam];

    RooRealVar *rfYield[2];

    RooNLLVar * rfNLL;
    RooPlot * rfNLLplot[nAmp];

    Modulation * moduOA;
    Modulation * modu[nAmp];
    Modulation * moduD[nDparam];

    Float_t DparamVal[nDparam];

  private:
    Bool_t enablePW;

    Float_t modValOA;
    Int_t modbin,modbinH,modbinR;
    Int_t pointCnt;


    Double_t yL,yR;
    Double_t asymNumer,asymDenom;
    Double_t asymVal,modVal,modValH,modValR;
    Double_t asymErr,modErr,modErrH,modErrR;

    Double_t bMax;
    Int_t spinbin;

    TString objName,appName;
    RooDataSet * appRooDataSet;


    Int_t nThreads;
    Float_t dpUB,dpLB;
    TString dpName[Ndp];
    TString dpTitle[Ndp];
    Int_t dpIdx[nAmp]; // which dp associated with the amplitude

    Double_t tree_PhiH;
    Double_t tree_PhiR;
    Double_t tree_PhiD;
    Double_t tree_Theta;
    Double_t tree_Pol;
    Double_t tree_PolErr;
    Double_t tree_Depol2;
    Double_t tree_Depol3;
    Double_t tree_DepolA;
    Double_t tree_DepolC;
    Double_t tree_DepolW;
    Double_t tree_Rellum;
    Double_t tree_X;
    Double_t tree_Mh;
    Double_t tree_Mmiss;
    Double_t tree_Z;
    Double_t tree_PhPerp;
    Double_t tree_Q2;
    Double_t tree_Y;
    Double_t tree_XF;
    Double_t tree_DY,tree_DYsgn;
    Double_t tree_diphM;
    Double_t tree_Weight;
    Int_t tree_Spin_idx;
    Bool_t tree_diphIsMCpi0;
    Double_t tree_diphMCmatchDist;
    Int_t tree_SpinMC_idx[EventTree::NumInjectionsMax];

    Bool_t treeActivated;

    InjectionModel *IM;
    Bool_t injectAsym;

  ClassDef(Asymmetry,1);
};

#endif
