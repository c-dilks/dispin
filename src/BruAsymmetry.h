#ifndef BruAsymmetry_
#define BruAsymmetry_

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <math.h>
#include <map>
#include <thread>

// ROOT
#include <TSystem.h>
#include <TStyle.h>
#include <TObject.h>
#include <TTree.h>
#include <TFile.h>
#include <TString.h>
#include <TMath.h>

// RooFit
#include <RooDataSet.h>
#include <RooCategory.h>

// BruFit
#include <FitManager.h>
#include <Minimiser.h>
#include <RooMcmc.h>
#include <Process.h>


// DiSpin
#include "Constants.h"
#include "Tools.h"
#include "Modulation.h"
#include "Binning.h"


class BruAsymmetry : public TObject
{
  public:
    BruAsymmetry(TString outdir_, TString minimizer_, Int_t whichSpinMC_=-1, Bool_t useDepol_=true);
    ~BruAsymmetry();

    void AddNumerMod(Modulation * modu);
    void AddDenomMod(Modulation * modu);
    void LoadDataSets(
        TString dataFileN,
        TString mcFileN="",
        TString weightFileN="",
        TString weightN="Signal",
        TString treeN="tree"
        );
    void LoadPDFweights(
        TString weightFileN="",
        TString weightN="Signal",
        TString weightObjN="HSsWeights"
        );
    void Bin(Binning * binscheme);
    void BuildPDF();
    void Fit();
    void PrintFitter() { FM->SetUp().WS().Print("v"); };

    Int_t GetNdim();
    Int_t GetNbins();
    void PrintBinScheme();

    void PrintLog(TString logString) {
      gSystem->RedirectOutput(outlog,"a");
      printf("%s\n",logString.Data());
      gSystem->RedirectOutput(0);
    };
    TString GetLogName() { return outlog; };

    // MCMC hyperparameters
    // - chain 1
    Int_t MCMC_iter; // number of MCMC MH steps
    Int_t MCMC_burnin; // discard the first `MCMC_burnin` steps ("burn-in")
    Float_t MCMC_norm; // ~ 1/stepSize
    // - chain 2 (for minimizer=="mcmccov")
    Int_t MCMC_cov_iter; // number of MCMC MH steps
    Int_t MCMC_cov_burnin; // discard the first `MCMC_burnin` steps ("burn-in")
    Float_t MCMC_cov_norm; // ~ 1/stepSize
    // - acceptance rate locks
    Double_t MCMC_lockacc_min;
    Double_t MCMC_lockacc_max;
    Double_t MCMC_lockacc_target;

    
    HS::FIT::FitManager * FM;

  private:

    TString outdir;
    Int_t minimizer;
    Int_t whichSpinMC;
    Bool_t useDepol;
    TString spinBranch;
    TString outlog;

    HS::FIT::Minuit2 *minuitAlgo;
    HS::FIT::RooMcmc *mcmcAlgo;

    TFile * infile[2];
    TFile * outfile[2];
    TTree * intr[2];
    TTree * outtr[2];
    Double_t Idx[2];
    TBranch * IdxBr[2];

    TString numerList, PDFstr, PDFweights;
    Int_t nThreads, nWorkers;
    TString numerFormu, denomFormu, formu;
    TString ampNameList,formuNameList;
    Int_t nDenomParams;

    Bool_t useMCint;


  ClassDef(BruAsymmetry,1);
};

#endif
