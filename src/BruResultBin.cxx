#include "BruResultBin.h"

ClassImp(BruResultBin)

BruResultBin::BruResultBin(TString bruDir_, HS::FIT::Bins *HSbins, Int_t binnum0, Int_t binnum1, Int_t binnum2) {

  bruDir = bruDir_;
  
  // get number of dimensions and TAxes
  nDim = HSbins->GetNAxis();
  axes = HSbins->GetVarAxis();

  // get various variables for this bin
  if(nDim>=1) binNums.push_back(binnum0); // bin number
  if(nDim>=2) binNums.push_back(binnum1);
  if(nDim>=3) binNums.push_back(binnum2);
  if(nDim<1||nDim>3) { fprintf(stderr,"ERROR: bad BruResultBin::nDim\n"); return; };
  for(int i=0; i<nDim; i++) {
    centers.push_back(axes[i].GetBinCenter(binNums[i])); // bin center
    ivNames.push_back(axes[i].GetName()); // IV name
    lBounds.push_back(axes[i].GetBinLowEdge(binNums[i]); // lower bound
    uBounds.push_back(axes[i].GetBinUpEdge(binNums[i]); // upper bound

  // get bin index and name, brufit syntax
  switch(nDim) { // bin index
    case 1: bruIdx = HSbins->FindBin(centers[0]); break;
    case 2: bruIdx = HSbins->FindBin(centers[0],centers[1]); break;
    case 3: bruIdx = HSbins->FindBin(centers[0],centers[1],centers[2]); break;
  };
  bruName = HSbins->GetBinName(bruIdx);

  // open binTree, which we will use to fill histograms to calculate means, etc.
  binTreeFile = new TFile(bruDir+"/"+bruName+"/TreeData.root","READ");
  binTree = (TTree*) binTreeFile->Get("tree");

  // start histograms and set branch addresses
  for(int i=0; i<nDim; i++) {
    TString histname = Form("%s_hist_%d",ivNames[i].Data(),idx);
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

  // close binTree files
  for(auto binTreeFile : binTreeFiles) binTreeFile->Close();

};

BruResultBin::~BruResultBin() {};
