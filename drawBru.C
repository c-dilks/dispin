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
Int_t nSamples;
TString bruDir;
HS::FIT::Bins * HSbins;
const int nParamsMax = 30;
Float_t asymPlotMin;
Float_t asymPlotMax;
Int_t minimizer;
enum minimEnum { mkMCMC, mkMinuit };
TString hTitle,vTitle,pName;
TString resultFileN;


//////////////////////////////////////////////////////////

class BruBin : public TObject {
  public:
    Int_t idx;
    TString name,var;
    Double_t center,mean,ub,lb;
    TH1D * hist;
    Double_t param[nParamsMax];
    Double_t paramErr[nParamsMax];
    TH1D * paramVsSample[nParamsMax];
    TFile * resultFile;
    TTree * resultTree;
    TTree * mcmcTree;

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

      // open result file, and read trees
      switch(minimizer) {
        case mkMCMC: resultFileN="ResultsHSRooMcmcSeq.root"; break;
        case mkMinuit: resultFileN="ResultsHSMinuit2.root"; break;
      };
      resultFile = new TFile(bruDir+"/"+name+"/"+resultFileN,"READ");
      resultTree = (TTree*) resultFile->Get("ResultTree");
      if(minimizer==mkMCMC) {
        mcmcTree = (TTree*) resultFile->Get("MCMCTree");
        nSamples = (Int_t) mcmcTree->GetEntries();
        for(int i=0; i<nParamsMax; i++) {
          pName = Form("bin%d_param%d_vs_sample",idx,i);
          paramVsSample[i] = new TH1D(pName,pName,nSamples,1,nSamples+1);
        };
      };

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

void drawBru(
  TString bruDir_ = "bruspin",
  TString minimizer_ = "minuit",
  Bool_t logscale = true, /* plot sample# on log scale */
  Float_t asymPlotMin_ = -10, /* units: % */
  Float_t asymPlotMax_ =  10  /* units: % */
) {

  asymPlotMin = asymPlotMin_ / 100.0;
  asymPlotMax = asymPlotMax_ / 100.0;

  // get minimizer type
  if(minimizer_=="mcmc") minimizer=mkMCMC;
  else if(minimizer_=="minuit") minimizer=mkMinuit;
  else { fprintf(stderr,"ERROR: unknown minimizer type\n"); return; };


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

  // initialize nSamples
  nSamples = 0;

  // build array of BruBin objects
  BruBinList = new TObjArray();
  TVectorD binCoord(nDim);
  if(nDim==1) {
    for(TAxis axis0 : HSbins->GetVarAxis()) {
      for(int bn=1; bn<=axis0.GetNbins(); bn++) {
        binCoord[0] = axis0.GetBinCenter(bn);
        BruBinList->AddLast(new BruBin(axis0,bn,binCoord));
      };
      hTitle = axis0.GetName(); // TODO: use dispin Binning to convert to proper title
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
  RooDataSet * paramSet;
  TString paramList[nParamsMax];
  Modulation * moduList[nParamsMax];
  Int_t nParams;
  TString paramName;
  Double_t paramval[nParamsMax];
  Long64_t entry;
  while((BB = (BruBin*) nextBin())) {

    // get parameter tree
    paramSet = (RooDataSet*) BB->resultFile->Get("FinalParameters");

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
    if(BB->resultTree->GetEntries()!=1)
      fprintf(stderr,"WARNING: ResultTree does not have 1 entry\n");
    for(int i=0; i<nParams; i++) {
      BB->resultTree->SetBranchAddress(paramList[i],&(BB->param[i]));
      BB->resultTree->SetBranchAddress(paramList[i]+"_err",&(BB->paramErr[i]));
      BB->resultTree->GetEntry(0);
    };

    // if MCMC was used, fill param vs sample graphs
    if(minimizer==mkMCMC) {
      BB->mcmcTree->SetBranchAddress("entry",&entry);
      for(int i=0; i<nParams; i++) {
        BB->mcmcTree->SetBranchAddress(paramList[i],&paramval[i]);
        vTitle = moduList[i] ? moduList[i]->AsymmetryTitle() : "N";
        BB->paramVsSample[i]->SetTitle(
          vTitle+" vs. MCMC sample;sample;"+vTitle);
      };
      for(Long64_t e=0; e<BB->mcmcTree->GetEntries(); e++) {
        BB->mcmcTree->GetEntry(e);
        for(int i=0; i<nParams; i++) 
          BB->paramVsSample[i]->Fill(entry+1,paramval[i]);
      };
    };
  };
  nextBin.Reset();


  // build graphs
  TString outfileN = bruDir+"/asym.root";
  TFile * outFile = new TFile(outfileN,"RECREATE");
  TGraphErrors * paramGr[nParamsMax];
  Int_t cnt;
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
  TCanvas * paramVsSampleCanv;
  TCanvas * cornerCanv;
  Float_t xMin,xMax,yMin,yMax;
  TLine * zeroLine;
  if(nDim==1) {

    // parameter result vs. horizizontal iv
    paramCanv = new TCanvas("canvAsym","canvAsym",1000,1000);
    paramCanv->Divide(4,(nParams-1)/4+1);
    for(int i=0; i<nParams; i++) {
      paramCanv->cd(i+1);
      yMin = asymPlotMin;
      yMax = asymPlotMax;
      if(paramGr[i]->GetYaxis()->GetXmin() < yMin)
        yMin = paramGr[i]->GetYaxis()->GetXmin();
      if(paramGr[i]->GetYaxis()->GetXmax() > yMax)
        yMax = paramGr[i]->GetYaxis()->GetXmax();
      paramGr[i]->GetYaxis()->SetRangeUser(yMin,yMax);
      paramGr[i]->Draw("APE");
      xMin = paramGr[i]->GetXaxis()->GetXmin();
      xMax = paramGr[i]->GetXaxis()->GetXmax();
      if(!paramList[i].Contains("Yld")) {
        zeroLine = new TLine(xMin,0,xMax,0);
        zeroLine->SetLineColor(kBlack);
        zeroLine->SetLineWidth(2);
        zeroLine->SetLineStyle(kDashed);
        zeroLine->Draw();
      };
    };
    paramCanv->Draw();
    paramCanv->Write();

    // parameter vs. sample
    if(minimizer==mkMCMC) {
      while((BB = (BruBin*) nextBin())) {
        paramVsSampleCanv = new TCanvas(
          Form("paramVsSample_%d",BB->idx),
          Form("paramVsSample_%d",BB->idx),
          1000,1000);
        paramVsSampleCanv->Divide(4,(nParams-1)/4+1);
        for(int i=0; i<nParams; i++) {
          paramVsSampleCanv->cd(i+1);
          if(logscale) gPad->SetLogx();
          if(!paramList[i].Contains("Yld"))
            BB->paramVsSample[i]->GetYaxis()->SetRangeUser(
              asymPlotMin,asymPlotMax);
          BB->paramVsSample[i]->Draw("HIST");
        };
        paramVsSampleCanv->Write();
      };
      nextBin.Reset();
      while((BB = (BruBin*) nextBin())) {
        //cornerCanv = (TCanvas*) BB->resultFile->Get("Corner Full Plot")->Clone();
        cornerCanv = (TCanvas*) BB->resultFile->Get("Corner Plot")->Clone();
        cornerCanv->Write(Form("cornerCanv_%d",BB->idx));
      };
      nextBin.Reset();
    };
  };

  // cleanup
  while((BB = (BruBin*) nextBin())) BB->resultFile->Close();
  nextBin.Reset();
  outFile->Close();
  printf("produced %s\n",outfileN.Data());

};
