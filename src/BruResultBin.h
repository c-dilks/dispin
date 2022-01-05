#ifndef BruResultBin_
#define BruResultBin_

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <stdexcept>
#include <vector>

// ROOT
#include "TObject.h"
#include "TTree.h"
#include "TFile.h"
#include "TString.h"
#include "TH1.h"

// Brufit
#include "Bins.h"

class BruResultBin : public TObject
{
  public:
    BruResultBin();
    ~BruResultBin();
    
  private:
    TString bruDir;
    Int_t nDim;
    std::vector<TAxis>    axes;
    std::vector<Int_t>    binNums;
    std::vector<Double_t> centers;
    std::vector<TString>  ivNames;
    std::vector<Double_t> lBounds;
    std::vector<Double_t> uBounds;
    std::vector<TH1D*>    ivHists;
    std::vector<Double_t> ivMeans;
    TFile* binTreeFile;
    TTree* binTrees;
    Double_t iv[3];
    Int_t bruIdx;
    TString bruName;

  ClassDef(BruResultBin,1);
};

#endif
