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
#include "TArrayD.h"

// dispin
#include "Constants.h"
#include "Tools.h"
#include "EventTree.h"


class Binning : public TObject
{
  public:
    Binning();
    ~Binning();
    void AddBinBound(Int_t ivIdx, Float_t newBound);
    void PrintBinBounds();
    Int_t GetBin(Int_t v_, Float_t iv_);
    TString GetBoundStr(Int_t bn, Int_t dim);

    Bool_t SetScheme(Int_t pairType, Int_t ivType, Int_t nb0=-1, Int_t nb1=-1, Int_t nb2=-1);


    // enumerators 
    enum ivEnum { vX, vM, vZ, vPt, vDY, vQ, vXF, nIV }; // Independent Variables (IV)
    Float_t minIV[nIV];
    Float_t maxIV[nIV];


    // bin boundaries
    Int_t nBins[nIV]; // the number of bins
    std::vector<Float_t> bound[nIV]; // this is the lower bound
                                // for bin "bin#"; to get the upper bound
                                // of the highest bin, use bin#+1


    TString IVname[nIV];
    TString IVtitle[nIV];
    TString IVoutrootBranchName[nIV];


    // variables used to define Asymmetry binning scheme
    Int_t dimensions; // total number of dimensions, for multi-dim binning
    Int_t ivVar[3]; // which IV the binning scheme is in [dimension]
    Int_t binNum;
    TArrayD * binArray[3]; // array of bin boundaries for each [dimension]
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
    Int_t GetNbinsTotal();
    Int_t GetNbinsHighDim(); // number of IV1 and IV2 bins
    TArrayD * GetBinArray(Int_t dim);
    TString GetIVname(Int_t dim);
    TString GetIVtitle(Int_t dim);
    TString GetIVoutrootBranchName(Int_t dim);
    Float_t GetIVmin(Int_t dim);
    Float_t GetIVmax(Int_t dim);
    Float_t GetIVval(Int_t dim); // return iv value for this dim (MUST CALL FindBin FIRST)
    // additional scheme methods
    // - bin finding
    Int_t FindBin(EventTree * ev);
    Int_t FindBin(Float_t iv0, Float_t iv1=0, Float_t iv2=0);
    Int_t FindBinGen(EventTree * ev);
    // -misc
    Bool_t CheckDim(Int_t dim_);
    Int_t HashBinNum(Int_t bin0, Int_t bin1=-1, Int_t bin2=-1);
    Int_t UnhashBinNum(Int_t bn, Int_t dim);
    void BinNumToIBL(Int_t bn_, Int_t &I_, Int_t &BL_);

    // extra variables, used for Asymmetry
    // (it would be nicer to just stream Asymmetry objects to a root file,
    //  but so far it's not working...)
    Int_t oaTw,oaL,oaM;
    void SetOAnums(Int_t t_, Int_t l_, Int_t m_) {
      oaTw=t_; oaL=l_; oaM=m_;
    };
    Bool_t useWeighting;
    Int_t gridDim;

    Int_t whichHad[2];
    
  private:

    TString schemeVersion;
    Float_t ivVal[3];
    Float_t ivValGen[3];


  ClassDef(Binning,1);
};

#endif
