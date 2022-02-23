#ifndef CatTree_
#define CatTree_

#include "EventTree.h"

class CatTree : public EventTree
{
  public:
    CatTree(TString treeFileN);
    ~CatTree();
    void GetEvent(Long64_t i) override;
    Float_t GetDepol2() { return (Float_t) d_Depol2; };
    Float_t GetDepol3() { return (Float_t) d_Depol3; };

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

  ClassDefOverride(CatTree,1);
};

#endif
