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
    BruAsymmetry(TString outdir_);
    ~BruAsymmetry();

    void AddNumerMod(Modulation * modu);
    void BuildPDF();
    void LoadDataSets(RooDataSet * rooData, RooDataSet * rooMC);
    void Fit();
    void PrintFitter() { FM->SetUp().WS().Print("v"); };


    // MCMC settings
    Int_t MCMC_iter; // number of samples
    Int_t MCMC_burnin; // number of burnin
    Float_t MCMC_norm; // ~ 1/stepSize

    
    HS::FIT::FitManager * FM;

  private:

    TString outdir;
    TTree *trData, *trMC;
    TString numerList, PDFstr;
    Int_t nThreads;


  ClassDef(BruAsymmetry,1);
};

#endif
