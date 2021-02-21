/*
#include <cstdlib>
#include <iostream>
#include <vector>
#include <map>
#include <TFile.h>
#include <TString.h>
#include <TCanvas.h>
#include <TGraphErrors.h>
#include "BruAsymmetry.h"
*/
R__LOAD_LIBRARY(DiSpin)
#include "BruAsymmetry.h"

//////////////////////////////////////////////////////////

TObjArray * BruBinList;
Int_t nDim, nBins;
TString bruDir;
HS::FIT::Bins * HSbins;
const int nParamsMax = 30;
const Float_t ASYM_PLOT_MIN = -0.07;
const Float_t ASYM_PLOT_MAX = 0.07;


//////////////////////////////////////////////////////////

class BruBin : public TObject {
  public:
    Int_t idx;
    TString name,var;
    Double_t center,mean,ub,lb;
    TH1D * hist;
    Double_t param[nParamsMax];
    Double_t paramErr[nParamsMax];
    // -constructor
    BruBin(TAxis axis, Int_t binnum, TVectorD coord) {
      idx = HSbins->FindBin(coord); // bin index
      name = HSbins->GetBinName(idx); // bin name (brufit syntax)
      var = axis.GetName(); // bin variable
      // get bin center, lower bound, upper bound
      center = axis.GetBinCenter(binnum);
      lb = axis.GetBinLowEdge(binnum);
      ub = axis.GetBinUpEdge(binnum);
      // get bin mean (requires opening the bin's tree)
      hist = new TH1D( Form("ivHist%d",idx),Form("ivHist%d",idx),
        100,lb-0.05,ub+0.05);
      TFile * binTreeFile;
      TTree * binTree;
      Double_t iv;
      binTreeFile = new TFile(bruDir+"/"+name+"/TreeData.root","READ");
      binTree = (TTree*) binTreeFile->Get("tree");
      binTree->SetBranchAddress(var,&iv);
      for(Long64_t e=0; e<binTree->GetEntries(); e++) {
        binTree->GetEntry(e);
        hist->Fill(iv);
      };
      mean = hist->GetMean();
      binTreeFile->Close();
      // misc
      for(int i=0; i<nParamsMax; i++) {
        param[i] = UNDEF;
        paramErr[i] = UNDEF;
      };
    };
    // -print out
    void PrintInfo() {
      printf("BIN %d\n",idx);
      printf("  name   = %s\n",name.Data());
      printf("  var    = %s\n",var.Data());
      printf("  center = %.2f\n",center);
      printf("  mean   = %.2f\n",mean);
      printf("  range  = %.2f to %.2f\n",lb,ub);
    };
};

//////////////////////////////////////////////////////////

void drawBru(TString bruDir_="bruspin") {

  // get nDim
  bruDir = bruDir_;
  TFile * binFile = new TFile(bruDir+"/DataBinsConfig.root","READ");
  HSbins = (HS::FIT::Bins*) binFile->Get("HSBins");
  nDim = HSbins->GetNAxis();
  printf("nDim = %d\n",nDim);
  binFile->Close();

  // get Nbins
  nBins = HSbins->GetN();
  printf("nBins = %d\n",nBins);

  // build array of BruBin objects
  BruBinList = new TObjArray();
  TVectorD binCoord(nDim);
  TString hTitle;
  if(nDim==1) {
    for(TAxis axis0 : HSbins->GetVarAxis()) {
      for(int bn=1; bn<=axis0.GetNbins(); bn++) {
        binCoord[0] = axis0.GetBinCenter(bn);
        BruBinList->AddLast(new BruBin(axis0,bn,binCoord));
      };
      hTitle = axis0.GetName();
    };
  };


  // define BruBin iterator, and print bins
  BruBin * BB;
  TObjArrayIter nextBin(BruBinList);
  Tools::PrintSeparator(30);
  while((BB = (BruBin*) nextBin())) BB->PrintInfo();
  Tools::PrintSeparator(30);
  nextBin.Reset();


  // get parameter values from results trees
  Bool_t first = true;
  TFile * resultFile;
  TTree * resultTree;
  RooDataSet * paramSet;
  TString paramList[nParamsMax];
  Modulation * moduList[nParamsMax];
  Int_t nParams;
  TString paramName;
  while((BB = (BruBin*) nextBin())) {

    // get parameter tree
    resultFile = new TFile(
      bruDir+"/"+BB->name+"/ResultsHSRooMcmcSeq.root","READ");
    resultTree = (TTree*) resultFile->Get("ResultTree");
    paramSet = (RooDataSet*) resultFile->Get("FinalParameters");

    // get parameter list
    if(first) {
      nParams = 0;
      for(int i=0; i<paramSet->get()->size(); i++) {
        if(nParams>nParamsMax) {
          fprintf(stderr,"ERROR: too many params\n"); return 1; };
        paramName = (*(paramSet->get()))[i]->GetName();
        if(paramName=="NLL") continue;
        if(paramName.Contains("Yld")) moduList[nParams] = nullptr;
        else moduList[nParams] = new Modulation(paramName);
        paramList[nParams] = paramName;
        printf("param %d:  %s\n",nParams,paramList[nParams].Data());
        nParams++;
      };
      Tools::PrintSeparator(30);
      first = false;
    };

    // get parameter values
    if(resultTree->GetEntries()!=1)
      fprintf(stderr,"WARNING: ResultTree does not have 1 entry\n");
    for(int i=0; i<nParams; i++) {
      resultTree->SetBranchAddress(paramList[i],&(BB->param[i]));
      resultTree->SetBranchAddress(paramList[i]+"_err",&(BB->paramErr[i]));
      resultTree->GetEntry(0);
    };
    resultFile->Close();
  };
  nextBin.Reset();


  // build graphs
  TFile * outFile = new TFile(bruDir+"/asym.root","RECREATE");
  TGraphErrors * paramGr[nParamsMax];
  Int_t cnt;
  TString vTitle;
  /*
   * 1dim
   *   one graph per param
   * 2dim
   *   categ0: horizontal axis
   *   one graph per param, times numBins in categ1
   */
  if(nDim==1) {
    for(int i=0; i<nParams; i++) {

      // define graph
      paramGr[i] = new TGraphErrors();
      paramGr[i]->SetName("gr_"+paramList[i]);
      vTitle = moduList[i] ? moduList[i]->AsymmetryTitle() : "N";
      paramGr[i]->SetTitle(vTitle+" vs. "+hTitle+";"+hTitle+";"+vTitle);
      paramGr[i]->SetMarkerStyle(kFullCircle);
      paramGr[i]->SetMarkerColor(kAzure);
      paramGr[i]->SetLineColor(kAzure);

      // add points to graph
      cnt=0;
      while((BB = (BruBin*) nextBin())) {
        paramGr[i]->SetPoint(cnt,BB->mean,BB->param[i]);
        paramGr[i]->SetPointError(cnt,0,BB->paramErr[i]);
        cnt++;
      };
      nextBin.Reset();
      paramGr[i]->Write();
    };
  };

  // build canvases
  TCanvas * paramCanv;
  Float_t xMin,xMax,yMin,yMax;
  TLine * zeroLine;
  if(nDim==1) {
    paramCanv = new TCanvas("canv","canv",1000,1000);
    paramCanv->Divide(4,(nParams-1)/4+1);
    for(int i=0; i<nParams; i++) {
      paramCanv->cd(i+1);
      yMin = ASYM_PLOT_MIN;
      yMax = ASYM_PLOT_MAX;
      if(paramGr[i]->GetYaxis()->GetXmin() < yMin)
        yMin = paramGr[i]->GetYaxis()->GetXmin();
      if(paramGr[i]->GetYaxis()->GetXmax() > yMax)
        yMax = paramGr[i]->GetYaxis()->GetXmax();
      paramGr[i]->GetYaxis()->SetRangeUser(yMin,yMax);
      paramGr[i]->Draw("APE");
      xMin = paramGr[i]->GetXaxis()->GetXmin();
      xMax = paramGr[i]->GetXaxis()->GetXmax();
      zeroLine = new TLine(xMin,0,xMax,0);
      zeroLine->SetLineColor(kBlack);
      zeroLine->SetLineWidth(2);
      zeroLine->SetLineStyle(kDashed);
      zeroLine->Draw();
    };
    paramCanv->Draw();
    paramCanv->Write();
  };
  outFile->Close();

};
