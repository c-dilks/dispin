#include "CatTree.h"

ClassImp(CatTree)

CatTree::CatTree(TString treeFileN) : EventTree() {
  ctreeFile = new TFile(treeFileN,"READ");
  ctree = (TTree*) ctreeFile->Get("tree");

  ctree->SetBranchAddress("PhiH",&d_PhiH);
  ctree->SetBranchAddress("PhiR",&d_PhiR);
  ctree->SetBranchAddress("PhiD",&d_PhiD);
  ctree->SetBranchAddress("Theta",&d_Theta);
  ctree->SetBranchAddress("Depol2",&d_Depol2); // accessor: GetDepol2()
  ctree->SetBranchAddress("Depol3",&d_Depol3); // accessor: GetDepol3()
  ctree->SetBranchAddress("DepolA",&d_DepolA);
  ctree->SetBranchAddress("DepolC",&d_DepolC);
  ctree->SetBranchAddress("DepolW",&d_DepolW);
  ctree->SetBranchAddress("Rellum",&d_Rellum);
  ctree->SetBranchAddress("X",&d_X);
  ctree->SetBranchAddress("Mh",&d_Mh);
  ctree->SetBranchAddress("Mmiss",&d_Mmiss);
  ctree->SetBranchAddress("Z",&d_Z);
  ctree->SetBranchAddress("PhPerp",&d_PhPerp);
  ctree->SetBranchAddress("Q2",&d_Q2);
  ctree->SetBranchAddress("XF",&d_XF);
  ctree->SetBranchAddress("DY",&d_DY);
  ctree->SetBranchAddress("DYsgn",&d_DYsgn);
  ctree->SetBranchAddress("diphM",&d_diphM);
  ctree->SetBranchAddress("Weight",&d_Weight);
  ctree->SetBranchAddress("Pol",&d_Pol);
  if(ctree->GetBranch("Spin_idx")) ctree->SetBranchAddress("Spin_idx",&spinIdx);
  else spinIdx=0;

  ENT = ctree->GetEntries();
  printf("opened catTree from %s\n",treeFileN.Data());
  printf("number of entries: %lld\n",ENT);
}

void CatTree::GetEvent(Long64_t i) {

  if(i%10000==0) printf("[^^] %.2f%%\n",100*(float)i/((float)ENT));
  ctree->GetEntry(i);

  // set EventTree floats from CatTree doubles
  // - outroot files (and EventTrees) store Float_t's, catTrees store Double_t's
  PhiH   = d_PhiH;
  PhiR   = d_PhiR;
  PhiD   = d_PhiD;
  theta  = d_Theta;
  x      = d_X;
  Mh     = d_Mh;
  Mmiss  = d_Mmiss;
  Zpair  = d_Z;
  PhPerp = d_PhPerp;
  Q2     = d_Q2;
  xF     = d_XF;
  DY     = d_DY;
  DYsgn  = d_DYsgn;

};

Float_t CatTree::GetDepolarizationFactor(Char_t kf) {
  if(kf=='A')      return (Float_t) d_DepolA; // typecast to floats for backward compatibility...
  else if(kf=='C') return (Float_t) d_DepolC;
  else if(kf=='W') return (Float_t) d_DepolW;
  else {
    fprintf(stderr,"ERROR: unknown depolarization factor %c; returning 0\n",kf);
    return 0.0;
  };
};
Float_t CatTree::GetDepolarizationRatio(Int_t twist) {
  switch(twist) {
    case 2: return GetDepol2(); break;
    case 3: return GetDepol3(); break;
    default:
            fprintf(stderr,"ERROR: unknown twist %d for depolarization factor ratio; return 0\n",twist);
            return 0;
  };
};

CatTree::~CatTree() {};
