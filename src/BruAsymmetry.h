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


// DiSpin
#include "Constants.h"
#include "Tools.h"
#include "Modulation.h"
#include "Binning.h"


class BruAsymmetry : public TObject
{
  public:
    BruAsymmetry();
    ~BruAsymmetry();


  private:


  ClassDef(BruAsymmetry,1);
};

#endif
