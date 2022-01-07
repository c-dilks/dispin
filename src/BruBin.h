#ifndef BruBin_
#define BruBin_

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <functional>
#include <algorithm>

// ROOT
#include "TObject.h"
#include "TTree.h"
#include "TFile.h"
#include "TString.h"
#include "TH1.h"

// DiSpin
#include "Constants.h"

// Brufit
#include "Bins.h"

class BruBin : public TObject
{
  public:
    BruBin(); // default constructor, just for streaming
    BruBin(TString bruDir_, HS::FIT::Bins *HSbins, Int_t binnum0, Int_t binnum1=-1, Int_t binnum2=-1);
    ~BruBin();

    // actions
    void CalculateStats(); // calculate bin means, etc.
    void OpenResultFile(Int_t minimizer); // open fit results file

    // accessors
    TAxis    GetAxis(Int_t dim);
    Int_t    GetBinNum(Int_t dim);
    Double_t GetCenter(Int_t dim);
    TString  GetIvName(Int_t dim);
    Double_t GetLBound(Int_t dim);
    Double_t GetUBound(Int_t dim);
    TH1D     *GetIvHists(Int_t dim);
    Double_t GetIvMean(Int_t dim);

    TFile *GetResultFile() { return resultFile; };
    TTree *GetResultTree() { return resultTree; };
    TTree *GetMcmcTree() { return mcmcTree; };
    Int_t GetNSamples() { return nSamples; };
    TH1D *GetParamVsSampleHist(Int_t param);
    TH1D *GetNllVsSampleHist() { return nllVsSampleHist; };
    Int_t GetBruIdx() { return bruIdx; };

    // parameter values and errors (used as TTree branchs)
    static const int nParamsMax = 30;
    Double_t paramVal[nParamsMax];
    Double_t paramErr[nParamsMax];

    // other
    void PrintInfo(); 

  protected:

    // return element `k` from vector `v<T>`, with exception handling; called by public `Get*` methods
    // - optionally execute `actionIfEmpty` if `v` has no elements
    template<class T>
      T GetElement(std::vector<T> v, Int_t k, T dflt, std::function<void()>actionIfEmpty=[](){}) {
        if(v.size()==0) actionIfEmpty();
        try { return v.at(k); }
        catch(const std::out_of_range &ex) {
          fprintf(stderr,"ERROR: BruBin::GetElement out of range\n");
          return dflt;
        };
      };

    // print all elements of a vector
    template<class T>
      void PrintVector(std::vector<T> v) {
        std::cout << "( ";
        std::for_each(v.begin(),v.end(),[](T &elem){ std::cout << elem << " "; });
        std::cout << ")" << std::endl;
      }
    
  private:
    TString bruDir;
    TString bruName;
    Int_t nDim;
    Int_t bruIdx;

    std::vector<TAxis>    axes;
    std::vector<Int_t>    binNums;
    std::vector<Double_t> centers;
    std::vector<TString>  ivNames;
    std::vector<Double_t> lBounds;
    std::vector<Double_t> uBounds;
    std::vector<TH1D*>    ivHists;
    std::vector<Double_t> ivMeans;

    Double_t iv[3];

    TFile *resultFile;
    TTree *resultTree;
    TTree *mcmcTree;
    Int_t nSamples;
    std::vector<TH1D*> paramVsSampleHists;
    TH1D *nllVsSampleHist;

  ClassDef(BruBin,1);
};

#endif
