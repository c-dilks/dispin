// validateRePair
// - validate RePair.cpp algorithm

R__LOAD_LIBRARY(DiSpin)
#include <Constants.h>

TTree *tree[2];
TString cut;
const Int_t nbins=100;
TCanvas *canv;
TH1D *hist[2];

void compareHists(TString var, Float_t lb, Float_t ub) {
  TString hn,cn;
  Double_t ent[2];
  Double_t mean[2];
  Double_t stddev[2];
  TString varTrun = var;
  varTrun(TRegexp("\\["))="_";
  varTrun(TRegexp("\\]"))="";
  cn = "c_"+varTrun;
  canv = new TCanvas(cn,cn,2000,1000);
  canv->Divide(2,1);
  for(int f=0; f<2; f++) {
    canv->cd(f+1);
    hn = Form("h_%s_file%d",varTrun.Data(),f);
    hist[f] = new TH1D(hn,hn,nbins,lb,ub);
    tree[f]->Project(hn,var,cut);
    hist[f]->Draw();
    ent[f] = hist[f]->GetEntries();
    mean[f] = hist[f]->GetMean();
    stddev[f] = hist[f]->GetStdDev();
  };
  if(TMath::Abs(ent[0]-ent[1])>0.000001) 
    fprintf(stderr,"FAIL: %s has unequal entries; diff=%f\n",var.Data(),ent[0]-ent[1]);
  if(TMath::Abs(mean[0]-mean[1])>0.000001) 
    fprintf(stderr,"FAIL: %s has unequal mean; diff=%f\n",var.Data(),mean[0]-mean[1]);
  if(TMath::Abs(stddev[0]-stddev[1])>0.000001) 
    fprintf(stderr,"FAIL: %s has unequal stddev; diff=%f\n",var.Data(),stddev[0]-stddev[1]);
  canv->Print("tmp/validateRePair/"+varTrun+".png");
};



///////////////////////////////////////////
void validateRePair(
  TString origFileN="outroot.rga_inbending_sp19/skim4_006666.hipo.root",
  TString repairFileN="outroot.rga_inbending_sp19/repair/skim4_006666.hipo.root"
) {
  enum fenum {orig,repair};
  TFile *infile[2];
  infile[orig] = new TFile(origFileN,"READ");
  infile[repair] = new TFile(repairFileN,"READ");
  for(int f=0; f<2; f++) {
    tree[f] = (TTree*) infile[f]->Get("tree");
  };

  gROOT->ProcessLine(".! mkdir -p tmp/validateRePair");

  cut = "pairType==0x34"; // pi+/pi-
  cut = "pairType==0x13"; // pi+/p

  compareHists("W",0,5);
  compareHists("Q2",0,12);
  compareHists("Nu",0,1);
  compareHists("x",0,1);
  compareHists("y",0,1);
  // - electron kinematics branches
  compareHists("eleE",0,12);
  compareHists("eleP",0,12);
  compareHists("elePt",0,6);
  compareHists("eleEta",0,6);
  compareHists("elePhi",-PIe,PIe);
  compareHists("eleVertex",-100,100);
  compareHists("eleStatus",-4000,0);
  compareHists("eleChi2pid",-10,10);
  compareHists("eleFiduCut",-1,3);
  compareHists("elePCALen",0,12);
  compareHists("eleECINen",0,12);
  compareHists("eleECOUTen",0,12);
  compareHists("eleSector",0,8);
  // - hadron branches
  compareHists("pairType",0,0x102);
  for(int h=0; h<2; h++) {
    compareHists(Form("hadRow[%d]",h),0,20);
    compareHists(Form("hadIdx[%d]",h),0,0x12);
    compareHists(Form("hadE[%d]",h),0,10);
    compareHists(Form("hadP[%d]",h),0,10);
    compareHists(Form("hadPt[%d]",h),0,6);
    compareHists(Form("hadEta[%d]",h),0,6);
    compareHists(Form("hadPhi[%d]",h),-PIe,PIe);
    compareHists(Form("hadXF[%d]",h),-1,1);
    compareHists(Form("hadVertex[%d]",h),-100,100);
    compareHists(Form("hadStatus[%d]",h),0,4000);
    compareHists(Form("hadBeta[%d]",h),0,3);
    compareHists(Form("hadChi2pid[%d]",h),-10,10);
    compareHists(Form("hadFiduCut[%d]",h),-1,3);
    compareHists(Form("Z[%d]",h),0,1);
  };
  // - dihadron branches
  compareHists("Mh",0,4);
  compareHists("Mmiss",0,4);
  compareHists("Zpair",0,1);
  compareHists("xF",-1,1);
  compareHists("alpha",0,10);
  compareHists("theta",0,PIe);
  compareHists("zeta",-5,5);
  compareHists("Ph",0,6);
  compareHists("PhPerp",0,6);
  compareHists("PhEta",0,6);
  compareHists("PhPhi",-PIe,PIe);
  compareHists("R",0,6);
  compareHists("RPerp",0,6);
  compareHists("RT",0,6);
  compareHists("PhiH",-PIe,PIe);
  // -- phiR angles
  compareHists("PhiRq",-PIe,PIe);
  compareHists("PhiRp",-PIe,PIe);
  compareHists("PhiRp_r",-PIe,PIe);
  compareHists("PhiRp_g",-PIe,PIe);
  // - event-level branches
  compareHists("runnum",4000,20000);
  compareHists("evnum",0,1e7);
  compareHists("helicity",-2,3);
};

