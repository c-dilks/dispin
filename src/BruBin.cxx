#include "BruBin.h"

ClassImp(BruBin)

// default constructor, just for streaming
BruBin::BruBin() {
  // resultFile = new TFile();
  // resultTree = new TTree();
  // mcmcTree = new TTree();
  // nllVsSampleHist = new TH1D();
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
  binTreeFile = new TFile(bruDir+"/"+bruName+"/TreeData.root","READ");
  binTree = (TTree*) binTreeFile->Get("tree");

  // get list of branches
  TObjArrayIter nextBranch(binTree->GetListOfBranches());
  while(auto br = nextBranch()) {
    TString brName = br->GetName();
    if( !brName.Contains("Idx") && !brName.Contains("Spin") ) binTreeBranches.push_back(brName);
  };

  // start histograms and set branch addresses
  std::map<TString,Double_t> ivVals;
  for(auto brName : binTreeBranches) {
    TString histname = Form("%s_hist_%d",brName.Data(),bruIdx);
    Double_t lb = binTree->GetMinimum(brName);
    Double_t ub = binTree->GetMaximum(brName);
    Double_t rg = ub-lb;
    lb -= 0.05*rg;
    ub += 0.05*rg;
    ivHists.insert(std::pair<TString,TH1D*>( brName, new TH1D(histname,histname,100,lb,ub) ));
    ivVals.insert(std::pair<TString,Double_t>( brName, UNDEF ));
    binTree->SetBranchAddress( brName, &(ivVals.at(brName)) );
  };

  // fill histograms
  for(Long64_t e=0; e<binTree->GetEntries(); e++) {
    binTree->GetEntry(e);
    for(auto brName : binTreeBranches) ivHists.at(brName)->Fill(ivVals.at(brName));
  };

  // calculate means
  for(auto brName : binTreeBranches) {
    ivMeans.insert(std::pair<TString,Double_t>( brName, ivHists.at(brName)->GetMean() ));
  };

  // close binTree file
  // binTreeFile->Close(); // leave open, closing causes seg fault when streaming

};


// open result file, read tree, prepare other objects
void BruBin::OpenResultFile(Int_t minimizer) {

  // open result file and tree
  TString resultFileN;
  switch(minimizer) {
    case mkMCMCseq: resultFileN="ResultsHSRooMcmcSeq.root";        break;
    case mkMCMCcov: resultFileN="ResultsHSRooMcmcSeqThenCov.root"; break;
    case mkMinuit:  resultFileN="ResultsHSMinuit2.root";           break;
    default:
                        fprintf(stderr,"ERROR: unknown minimizer in BruBin::OpenResultFile\n");
                        return;
  };
  resultFile = new TFile(bruDir+"/"+bruName+"/"+resultFileN,"READ");
  resultTree = (TTree*) resultFile->Get("ResultTree");

  // if MCMC, prepare some additional plots
  if(minimizer==mkMCMCseq || minimizer==mkMCMCcov) {
    mcmcTree = (TTree*) resultFile->Get("MCMCTree");
    nSamples = (Int_t) mcmcTree->GetEntries();
    TString pName;
    for(int i=0; i<nParamsMax; i++) {
      pName = Form("bin%d_param%d_vs_sample",bruIdx,i);
      paramVsSampleHists.push_back(new TH1D(pName,pName,nSamples,1,nSamples+1));
    };
    pName = Form("bin%d_NLL_vs_sample",bruIdx);
    nllVsSampleHist = new TH1D(pName,pName,nSamples,1,nSamples+1);
  } else {
    // default instantiations, to support streaming
    mcmcTree = new TTree();
    nllVsSampleHist = new TH1D();
  };
};


// accessors
TAxis BruBin::GetAxis(Int_t dim) { return GetVectorElement( axes, dim, TAxis() ); };
Int_t BruBin::GetBinNum(Int_t dim) { return GetVectorElement( binNums, dim, 0 ); };
Double_t BruBin::GetCenter(Int_t dim) { return GetVectorElement( centers, dim, UNDEF ); };
TString BruBin::GetIvName(Int_t dim) { return GetVectorElement( ivNames, dim, TString("") ); };
Double_t BruBin::GetLBound(Int_t dim) { return GetVectorElement( lBounds, dim, UNDEF ); };
Double_t BruBin::GetUBound(Int_t dim) { return GetVectorElement( uBounds, dim, UNDEF ); };
TH1D* BruBin::GetParamVsSampleHist(Int_t param) { return GetVectorElement( paramVsSampleHists, param, (TH1D*)nullptr ); };
Double_t BruBin::GetIvMean(TString varName) { return GetMapElement( ivMeans, varName, UNDEF, [&](){CalculateStats();} ); };
Double_t BruBin::GetIvMean(Int_t dim) { return GetIvMean(GetIvName(dim)); };

// print out
void BruBin::PrintInfo() {
  printf("BIN %d\n",bruIdx);
  printf("  bruName = %s\n",bruName.Data());
  printf("  ivNames = "); PrintVector(ivNames);
  printf("  centers = "); PrintVector(centers);
  printf("  means   = "); PrintMap(ivMeans);
};

BruBin::~BruBin() {};
