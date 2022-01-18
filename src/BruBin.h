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
    Double_t GetIvMean(Int_t dim); // by dimension number
    Double_t GetIvMean(TString varName); // by branch name

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
      T GetVectorElement(std::vector<T> v, Int_t k, T dflt, std::function<void()>actionIfEmpty=[](){}) {
        if(v.size()==0) actionIfEmpty();
        try { return v.at(k); }
        catch(const std::out_of_range &ex) {
          fprintf(stderr,"ERROR: BruBin::GetVectorElement out of range\n");
          return dflt;
        };
      }

    // similar method for map
    template<class K, class V>
      V GetMapElement(std::map<K,V> m, K k, V dflt, std::function<void()>actionIfEmpty=[](){}) {
        if(m.size()==0) actionIfEmpty();
        try { return m.at(k); }
        catch(const std::out_of_range &ex) {
          fprintf(stderr,"ERROR: BruBin::GetMapElement out of range\n");
          return dflt;
        };
      }

    // print all elements of a vector or map
    template<class T>
      void PrintVector(std::vector<T> v) {
        std::cout << "( ";
        std::for_each(v.begin(),v.end(),[](T &elem){ std::cout << elem << " "; });
        std::cout << ")" << std::endl;
      }
    template<class K, class V>
      void PrintMap(std::map<K,V> m) {
        std::cout << "[" << std::endl;
        for(auto &kv : m) std::cout << "    " << kv.first << ": " << kv.second << std::endl;
        std::cout << "]" << std::endl;
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

    std::vector<TString>       binTreeBranches;
    std::map<TString,TH1D*>    ivHists;
    std::map<TString,Double_t> ivMeans;

    TFile *binTreeFile = nullptr;
    TTree *binTree = nullptr;

    TFile *resultFile = nullptr;
    TTree *resultTree = nullptr;
    TTree *mcmcTree = nullptr;
    Int_t nSamples;
    std::vector<TH1D*> paramVsSampleHists;
    TH1D *nllVsSampleHist = nullptr;

  ClassDef(BruBin,1);
};

#endif
