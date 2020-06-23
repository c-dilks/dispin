#ifndef Binning_
#define Binning_

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
#include "TCanvas.h"
#include "TString.h"
#include "TMath.h"
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "TGraphErrors.h"
#include "TLine.h"

// dihbsa
#include "Constants.h"
#include "Tools.h"


class Binning : public TObject
{
  public:
    Binning() {}; // empty default constructor for streaming instances to ROOT files
    Binning(Int_t pairType_);
    ~Binning();
    void AddBinBound(Int_t ivIdx, Float_t newBound);
    void PrintBinBounds();
    Int_t GetBin(Int_t v_, Float_t iv_);
    TString GetBoundStr(Int_t bn, Int_t dim);

    Bool_t SetScheme(Int_t ivType);



    // enumerators 
    enum ivEnum { vX, vM, vZ, vPt, vPh, vQ, nIV }; // Independent Variables (IV)
    Float_t minIV[nIV];
    Float_t maxIV[nIV];


    // bin boundaries
    Int_t nBins[nIV]; // the number of bins
    std::vector<Float_t> bound[nIV]; // this is the lower bound
                                // for bin "bin#"; to get the upper bound
                                // of the highest bin, use bin#+1


    TString IVname[nIV];
    TString IVtitle[nIV];


    // variables used to define Asymmetry binning scheme
    Int_t dimensions; // total number of dimensions, for multi-dim binning
    Int_t ivVar[3]; // which IV the binning scheme is in [dimension]
    Int_t binNum;
    // scheme maps:
    // - each IV bin is assigned a 3-digit bin number
    // - use HashBinNum(...) to convert a set of bin numbers to the 3-digit bin number
    // - use binVecMap to convert the 3-digit number back to each bin number for each
    //   dimension 
    // - use binVec for looping over all bins (via range-based loops)
    std::vector<Int_t> binVec; // vector of 3-digit bin numbers, used for looping
    std::map<Int_t, Int_t> binVecMap[3]; // map 3-digit bin number back to each
                                         // single-digit bin number [dimension]
    

    // scheme accessors, which return things according to the current binning scheme
    Int_t GetNbins(Int_t dim);
    TString GetIVname(Int_t dim);
    TString GetIVtitle(Int_t dim);
    // additional scheme methods
    Bool_t CheckDim(Int_t dim_);
    Int_t HashBinNum(Int_t bin0, Int_t bin1=-1, Int_t bin2=-1);
    Int_t UnhashBinNum(Int_t bn, Int_t dim);

    // extra variables, used for Asymmetry
    // (it would be nicer to just stream Asymmetry objects to a root file,
    //  but so far it's not working...)
    Int_t oaTw,oaL,oaM;
    void SetOAnums(Int_t t_, Int_t l_, Int_t m_) {
      oaTw=t_; oaL=l_; oaM=m_;
    };
    Int_t whichHad[2];
    Bool_t useWeighting;
    Int_t gridDim;

    
  private:
    Int_t numKaons;


  ClassDef(Binning,1);
};

#endif
