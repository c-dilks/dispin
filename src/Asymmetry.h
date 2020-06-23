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
#include <RooMinuit.h>


// dihbsa
#include "Constants.h"
#include "Tools.h"
#include "Modulation.h"
#include "Trajectory.h"
#include "DIS.h"
#include "Binning.h"
#include "EventTree.h"


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


    Bool_t AddEvent(EventTree * ev);
    Float_t EvalModulation();
    Float_t EvalWeight();
    Float_t EvalKinematicFactor(EventTree * ev);
    Int_t SpinState(Int_t spin_);
    Bool_t KickEvent(TString reason,Float_t badValue);

    void ResetVars();
    void PrintSettings();

    void StreamData(TFile * tf);
    void AppendData(TFile * tf);
    TString AppFileName(TFile * tf);


    Double_t nEvents;

    Bool_t success;
    Bool_t successIVmode;
    Bool_t debug;
    Bool_t enableRellum;
    Bool_t extendMLM;

    Binning * BS;

    // settings
    Int_t oaTw,oaL,oaM;
    Int_t whichDim;
    Int_t gridDim;
    Bool_t useWeighting;



    // event-level variables -- these must be set for each event
    Float_t Mh;
    Float_t x;
    Float_t z;
    Float_t PhiH;
    Float_t PhiR;
    Float_t PhPerp;
    Float_t Ph;
    Float_t Q2;
    Float_t theta;
    Float_t pol;
    Int_t spinn;
    
    Float_t kfA,kfC,kfW;


    // number of bins
    static const Int_t iv1Bins = 100; // number of bins for ivDist1 plots
    static const Int_t iv2Bins = 50; // number of bins for ivDist2 plots
    static const Int_t iv3Bins = 30; // number of bins for ivDist3 plots
    static const Int_t nModBins = 7; // number of bins in azimuthal modulation
    static const Int_t nModBins2 = 8; // number of bins in 2d azimuthal modulation

    Float_t modMin,modMax,aziMax;
    Float_t weight;
    Float_t kf;


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

    // kf dist (for computing <K(y)>)
    TH1D * kfDist;

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



    // RooFit variables
    RooDataSet * rfData;
    RooGenericPdf * rfPdf[2];
    RooExtendPdf * rfPdfExtended[2];
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

    RooRealVar *rfPhiH, *rfPhiR, *rfTheta;
    RooRealVar *rfWeight;
    RooRealVar *rfPol, *rfRellum;

    static const Int_t nAmp = 12;
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

    Float_t DparamVal;

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
    TH1D * appDist1;
    TH2D * appDist2;
    TH3D * appDist3;
    RooDataSet * appRooDataSet;


    Int_t nThreads;
    Float_t kfUB,kfLB;





  ClassDef(Asymmetry,1);
};

#endif
