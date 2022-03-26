#ifndef CatTree_
#define CatTree_

#include "EventTree.h"

// class to support 'catTrees' = (conCATenated trees), for holding
// only the events that are analyzed by brufit
// - this class derives from EventTree, so that places where EventTree kinematic
//   variables are used (such as Mh,PhiH,etc.) are also compatible with CatTree
//   branches
// - CatTree variables are a small subset of those available in EventTree
// - EventTree variables are floats, CatTree variables are doubles; this inconsistency
//   remains for compatiblity with already produced ROOT files

class CatTree : public EventTree
{
  public:
    CatTree(TString treeFileN);
    ~CatTree();
    
    // get CatTree event, and set EventTree kinematic variables; does not set
    // Valid() booleans, since necessity of Valid() is assumed to only be
    // upstream of CatTree production
    void GetEvent(Long64_t i) override;

    // CatTree-specific accessors
    Float_t GetDepolarizationFactor(Char_t kf) override;
    Float_t GetDepolarizationRatio(Int_t twist) override;
    Float_t GetDepol2() override { return (Float_t) d_Depol2; };
    Float_t GetDepol3() override { return (Float_t) d_Depol3; };
    Double_t GetWeight() override { return d_Weight; };
    Int_t GetSpinIdx() override { return spinIdx; };

  private:

    TFile *ctreeFile;
    TTree *ctree;

    // branches
    Double_t d_PhiH;
    Double_t d_PhiR;
    Double_t d_PhiD;
    Double_t d_Theta;
    Double_t d_Depol2;
    Double_t d_Depol3;
    Double_t d_DepolA;
    Double_t d_DepolC;
    Double_t d_DepolW;
    Double_t d_Rellum;
    Double_t d_X;
    Double_t d_Mh;
    Double_t d_Mmiss;
    Double_t d_Z;
    Double_t d_PhPerp;
    Double_t d_Q2;
    Double_t d_XF;
    Double_t d_DY;
    Double_t d_DYsgn;
    Double_t d_diphM;
    Double_t d_Weight;
    Double_t d_Pol;
    Int_t spinIdx;

  ClassDefOverride(CatTree,1);
};

#endif
