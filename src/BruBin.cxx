#include "BruBin.h"

ClassImp(BruBin)

// default constructor, just for streaming
BruBin::BruBin() {
  resultFile = new TFile();
  resultTree = new TTree();
  mcmcTree = new TTree();
  nllVsSampleHist = new TH1D();
};

// constructor
BruBin::BruBin(TString bruDir_, HS::FIT::Bins *HSbins, Int_t binnum0, Int_t binnum1, Int_t binnum2) {

  bruDir = bruDir_;
  
  // get number of dimensions and TAxes
  nDim = HSbins->GetNAxis();
  axes = HSbins->GetVarAxis();

  // get various variables for this bin
  if(nDim>=1) binNums.push_back(binnum0); // bin number
  if(nDim>=2) binNums.push_back(binnum1);
  if(nDim>=3) binNums.push_back(binnum2);
  if(nDim<1||nDim>3) { fprintf(stderr,"ERROR: bad BruBin::nDim\n"); return; };
  for(int i=0; i<nDim; i++) {
    centers.push_back(axes[i].GetBinCenter(binNums[i])); // bin center
    ivNames.push_back(axes[i].GetName()); // IV name
    lBounds.push_back(axes[i].GetBinLowEdge(binNums[i])); // lower bound
    uBounds.push_back(axes[i].GetBinUpEdge(binNums[i])); // upper bound
  };

  // get bin index and name, brufit syntax
  switch(nDim) { // bin index
    case 1: bruIdx = HSbins->FindBin(centers[0]); break;
    case 2: bruIdx = HSbins->FindBin(centers[0],centers[1]); break;
    case 3: bruIdx = HSbins->FindBin(centers[0],centers[1],centers[2]); break;
  };
  bruName = HSbins->GetBinName(bruIdx);
};


// calculate bin means, etc.
void BruBin::CalculateStats() {

  // open binTree, which we will use to fill histograms to calculate means, etc.
  TFile *binTreeFile = new TFile(bruDir+"/"+bruName+"/TreeData.root","READ");
  TTree *binTree = (TTree*) binTreeFile->Get("tree");

  // start histograms and set branch addresses
  for(int i=0; i<nDim; i++) {
    TString histname = Form("%s_hist_%d",ivNames[i].Data(),bruIdx);
    ivHists.push_back(new TH1D(histname,histname,100,lBounds[i]-0.05,uBounds[i]+0.05));
    binTree->SetBranchAddress(ivNames[i],&iv[i]);
  };

  // fill histograms
  for(Long64_t e=0; e<binTree->GetEntries(); e++) {
    binTree->GetEntry(e);
    for(int i=0; i<nDim; i++) ivHists[i]->Fill(iv[i]);
  };

  // calculate means
  for(auto ivHist : ivHists) {
    ivMeans.push_back(ivHist->GetMean());
  };

  // close binTree file
  binTreeFile->Close();

};


// open result file, read tree, prepare other objects
void BruBin::OpenResultFile(Int_t minimizer) {

  // open result file and tree
  TString resultFileN;
  switch(minimizer) {
    case mkMCMC:        resultFileN="ResultsHSRooMcmcSeq.root"; break;
    case mkMCMCthenCov: resultFileN="ResultsHSRooMcmcSeqThenCov.root"; break;
    case mkMinuit:      resultFileN="ResultsHSMinuit2.root"; break;
    default:
                        fprintf(stderr,"ERROR: unknown minimizer in BruBin::OpenResultFile\n");
                        return;
  };
  resultFile = new TFile(bruDir+"/"+bruName+"/"+resultFileN,"READ");
  resultTree = (TTree*) resultFile->Get("ResultTree");

  // if MCMC, prepare some additional plots
  if(minimizer==mkMCMC || minimizer==mkMCMCthenCov) {
    mcmcTree = (TTree*) resultFile->Get("MCMCTree");
    nSamples = (Int_t) mcmcTree->GetEntries();
    TString pName;
    for(int i=0; i<nParamsMax; i++) {
      pName = Form("bin%d_param%d_vs_sample",bruIdx,i);
      paramVsSampleHists.push_back(new TH1D(pName,pName,nSamples,1,nSamples+1));
    };
    pName = Form("bin%d_NLL_vs_sample",bruIdx);
    nllVsSampleHist = new TH1D(pName,pName,nSamples,1,nSamples+1);
  };
};


// accessors
TAxis BruBin::GetAxis(Int_t dim) { return GetElement( axes, dim, TAxis() ); };
Int_t BruBin::GetBinNum(Int_t dim) { return GetElement( binNums, dim, 0 ); };
Double_t BruBin::GetCenter(Int_t dim) { return GetElement( centers, dim, 0.0 ); };
TString BruBin::GetIvName(Int_t dim) { return GetElement( ivNames, dim, TString("") ); };
Double_t BruBin::GetLBound(Int_t dim) { return GetElement( lBounds, dim, 0.0 ); };
Double_t BruBin::GetUBound(Int_t dim) { return GetElement( uBounds, dim, 0.0 ); };
TH1D *BruBin::GetIvHists(Int_t dim) { return GetElement( ivHists, dim, (TH1D*)nullptr, [&](){CalculateStats();} ); };
Double_t BruBin::GetIvMean(Int_t dim) { return GetElement( ivMeans, dim, 0.0, [&](){CalculateStats();} ); };
TH1D* BruBin::GetParamVsSampleHist(Int_t param) { return GetElement( paramVsSampleHists, param, (TH1D*)nullptr ); };


// print out
void BruBin::PrintInfo() {
  printf("BIN %d\n",bruIdx);
  printf("  bruName = %s\n",bruName.Data());
  printf("  ivNames = "); PrintVector(ivNames);
  printf("  centers = "); PrintVector(centers);
  printf("  means   = "); PrintVector(ivMeans);
};

BruBin::~BruBin() {};
