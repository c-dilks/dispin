#ifndef Ensemble_
#define Ensemble_

#include <stdio.h>
#include <stdlib.h>
#include <stdexcept>
#include <map>
#include <vector>

// ROOT
#include "TSystem.h"
#include "TObject.h"
#include "TTree.h"
#include "TChain.h"
#include "TFile.h"
#include "TString.h"
#include "TMath.h"

// dispin
#include "Constants.h"
#include "Tools.h"
#include "Ensemble.h"

using std::vector;
using std::pair;

class Ensemble : public TObject
{
  public:

    Ensemble(TString infiles);

    // iterator: builds next event ensemble, returning true
    // if successful; returns false after building the
    // last event
    // - you can use this in a while loop, to loop over event ensembles
    // - use StartOver() to reset the iterator
    Bool_t NextEvent();

    // initialize entry numbers, so iterations using NextEvent()
    // will start at the beginning of the chain
    void StartOver();

    // build the event ensemble, which contains the dihadron/diphoton at entry `i`
    // - ideally you want to call this once per event, not once per
    //   dihadron/diphoton 
    // - after calling Build, you can skip to the next event by calling Build on
    //   entry ub+1; this functionality is implemented by NextEvent()
    void Build(Long64_t i);

    // check if current entry's dihadron/diphoton is in the same event as
    // that of the ensemble we are trying to build
    Bool_t SameEvent();

    // add entry `e` to the lists
    // - diphotonList is a list of tree entry numbers for this ensemble's 
    //   diphotons
    // - dihadronList is a list of tree entry numbers for this ensemble's 
    //   dihadrons (excludes diphotons)
    // - hadronList is a list of (tree entry numbers, hadron numbers), for
    //   each unique hadron found
    // - rowList keeps track of what hadrons have been added to hadronList
    void AppendLists(Long64_t j);
    // reset lists, clearing all contents
    void ResetLists();


    // ACCESSORS //////////////////////////////////
    // - get relevant data from the current event ensemble
    vector<Long64_t> GetDiphotonList() { return diphotonList; };
    vector<Long64_t> GetDihadronList() { return dihadronList; };
    vector<pair<Long64_t,Int_t>> GetHadronList() { return hadronList; };
    Long64_t GetLB() { return lb; }; // upper bound of ensemble
    Long64_t GetUB() { return ub; }; // lower bound of ensemble
    Long64_t GetEnum() { return e; }; // current entry num
    Int_t GetEvnum() { return evnumEns; };
    Int_t GetRunnum() { return runnumEns; };

    // get hadron row IDs (useful for sanity checks, if you are
    // reading the chain from another class)
    void GetHadRow(Long64_t i, Int_t &ha, Int_t &hb) {
      tr->GetEntry(i); ha = hadRow[qA]; hb = hadRow[qB];
    };


  private:
    TChain *tr;
    Long64_t lb,ub;
    vector<Long64_t> diphotonList;
    vector<Long64_t> dihadronList;
    vector<pair<Long64_t,Int_t>> hadronList;

    vector<Int_t> rowList;

    Int_t evnumEns;
    Int_t runnumEns;

    Bool_t readForward,readBackward;
    Long64_t e,eF,eB,eNext,progCnt;

    // CHAIN VARS ///////////////////////////////////
    Long64_t ENT;
    Int_t evnum;
    Int_t runnum;
    Int_t hadRow[2];
    Int_t hadIdx[2];


    ClassDef(Ensemble,1);
};

#endif
