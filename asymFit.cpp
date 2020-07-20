// asymmetry fitter
// - fits spinroot cat file (from catSpinroot.cpp) for extracting asymmetry amplitudes
// - produces the file spinroot/asym.root
// - you can specify a specific `spinroot` directory, if you want

#include <cstdlib>
#include <iostream>
#include <vector>
#include <map>

// ROOT
#include "TFile.h"
#include "TString.h"
#include "TMath.h"
#include "TRegexp.h"
#include "TH1.h"
#include "TH2.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TLine.h"
#include "TStyle.h"
#include "TMultiGraph.h"
#include "TGraphAsymmErrors.h"
#include "TSystem.h"
#include "TObjArray.h"
#include "TLegend.h"

// DihBsa
#include "Constants.h"
#include "Binning.h"
#include "Asymmetry.h"


// subroutines
void DrawKinDepGraph(TGraph * g_, Binning * B_, Int_t d_);
void DrawSimpleGraph(TGraphErrors * g_, Binning * B_, Int_t d_, Bool_t setRange_=true);
void DrawAsymGr(TGraphErrors * g_);
void DrawAsymGr2(TGraph2DErrors * g_);
TGraphAsymmErrors * ShiftGraph(TGraphAsymmErrors * gr, Int_t nShift);
void SetCloneName(TH1 * clone_);

// global variables
Int_t N_AMP,N_D;
TString dihTitle,dihName;
Int_t gridDim;
Binning * BS;
Asymmetry * A;

// plot range for asymmetries
const Float_t ASYM_PLOT_MIN = -0.07;
const Float_t ASYM_PLOT_MAX = 0.07;



int main(int argc, char** argv) {

  //////////////////////////////////////////////
  // ARGUMENTS
  TString spinrootDir = "spinroot";
  Int_t fitMode, fitAlgo;
  Float_t DparamVal[Asymmetry::nDparam]; // (for sigma_UU systematic)
  Int_t nD = 0; // (for sigma_UU systematic)
  TString DparamStr = ""; // (for sigma_UU systematic)
  if(argc<3) {
    fprintf(stderr,"USAGE: asymFit.cpp [fitMode] [fitAlgo] [spinrootDir] [(opt)DparamVal List]\n");
    fprintf(stderr," [fitMode] which modulations to fit (see Asymmetry::SetFitMode)\n");
    fprintf(stderr," [fitAlgo] which fit algorithm to use:\n");
    fprintf(stderr,"\t0 = MLM fit: single or multi-amp, depending on fitMode\n");
    fprintf(stderr,"\t1 = 1d fit: single-amp only (depends on what was used as twist,L,M in buildSpinroot\n");
    fprintf(stderr,"\t2 = 2d fit: single or multi-amp, depending on fitMode\n");
    fprintf(stderr," [(opt)spinrootDir]: directory of spinroot files (default=spinroot)\n");
    fprintf(stderr," [(opt)DparamValue List]: for studying variations in UU cross section modulation amps\n");
    return 0;
  }
  if(argc>1) fitMode = (Int_t)strtof(argv[1],NULL);
  if(argc>2) fitAlgo = (Int_t)strtof(argv[2],NULL);
  if(argc>3) spinrootDir = TString(argv[3]);
  if(argc>4) {
    for(int ar=4; ar<argc; ar++) {
      if(ar-4<Asymmetry::nDparam) {
        DparamVal[ar-4] = (Float_t)strtof(argv[ar],NULL);
        DparamStr = Form("%s_%.2f",DparamStr.Data(),DparamVal[ar-4]);
        nD++;
      };
    }
  };
  //
  if(nD>0) {
    printf("D params:\n");
    for(int ar=0; ar<nD; ar++) printf("%f\n",DparamVal[ar]);
  };
  //////////////////////////////////////////////



  //////////////////////////////////////////////
  // OPTIONS
  Bool_t includeOAonMultiGr = false;
  gStyle->SetOptFit(1);
  //////////////////////////////////////////////


  // check fitAlgo
  if(fitAlgo<0 || fitAlgo>2) { 
    fprintf(stderr,"ERROR: bad fitAlgo value\n");
    return 0;
  }


  // open spinroot cat file and result file
  TString asymFileN = Form("%s/asym_%d%s.root",
    spinrootDir.Data(),fitMode,DparamStr.Data());
  TFile * catFile = new TFile(TString(spinrootDir+"/cat.root"),"READ");


  // instantiate Binning and Asymmetry, and extract them from catFile
  std::map<Int_t, Asymmetry*> asymMap;
  catFile->GetObject("BS",BS);
  for(Int_t bn : BS->binVec) {
    A = new Asymmetry(BS,bn);
    if(A->success) {
      A->AppendData(catFile);
      asymMap.insert(std::pair<Int_t,Asymmetry*>(bn,A));
    }
    else return 0;
  };
  dihTitle = PairTitle(BS->whichHad[qA],BS->whichHad[qB]);
  dihName = PairName(BS->whichHad[qA],BS->whichHad[qB]);


  // print which IV will be analyzed
  TString modN = A->oaModulationName;
  printf("--------> Analysing %s asymmetries vs. %s ",
    modN.Data(),(BS->GetIVname(0)).Data());
  if(BS->dimensions>=2) printf("in bins of %s ",(BS->GetIVname(1)).Data());
  if(BS->dimensions>=3) printf("and %s ",(BS->GetIVname(2)).Data());
  printf("\n\n");
  gridDim = A->gridDim;


  // perform the fits
  printf("--- calculate asymmetries\n");
  for(Int_t bn : BS->binVec) {
    A = asymMap.at(bn);

    // preparation
    if(nD>0) { for(int v=0; v<nD; v++) A->DparamVal[v]=DparamVal[v]; };
    A->SetFitMode(fitMode);

    // fitting
    A->FitAsymGraph();
    if(fitAlgo==0) A->FitAsymMLM();
  };


  // build maps from Binning::binVec number to plots etc.
  TGraphErrors * kindepGrOA;
  TGraphAsymmErrors * kindepGrMA[Asymmetry::nAmp];
  TGraphErrors * chindfGrOA;
  TGraphErrors * rellumGr;
  TMultiGraph * multiGr;

  std::map<Int_t, TGraphErrors*> kindepOAmap;
  std::map<Int_t, TGraphAsymmErrors*> kindepMAmap[Asymmetry::nAmp];
  std::map<Int_t, TMultiGraph*> multiMap;
  std::map<Int_t, TGraphErrors*> chindfOAmap;
  std::map<Int_t, TGraphErrors*> rellumMap;

  TString grTitle,grTitleSuffix,grName,grNameSuffix;
  TString polOAstr = Form("(pol. fixed at %.2f)",Asymmetry::polOA);
  Bool_t first = true;
  for(Int_t bn : BS->binVec) {
    A = asymMap.at(bn);

    // get number of RooFit params
    if(first) { 
      N_AMP = A->nAmpUsed;
      N_D = A->nDparamUsed;
      first = false;
    };

    // set graph title and name suffixes
    switch(BS->dimensions) {
      case 1:
        grTitleSuffix = BS->GetIVtitle(0) + ";" + BS->GetIVtitle(0);
        grNameSuffix = BS->GetIVname(0);
        break;
      case 2:
        grTitleSuffix = BS->GetIVtitle(0) + " :: " +
          BS->GetBoundStr(bn,1) + ";" +
          BS->GetIVtitle(0);
        grNameSuffix = Form("%s_bin_%s%d",
            (BS->GetIVname(0)).Data(),
            (BS->GetIVname(1)).Data(), BS->UnhashBinNum(bn,1)
            );
        break;
      case 3:
        grTitleSuffix = BS->GetIVtitle(0) + " :: " +
          BS->GetBoundStr(bn,1) + ", " +
          BS->GetBoundStr(bn,2) + ";" +
          BS->GetIVtitle(0);
        grNameSuffix = Form("%s_bin_%s%d_%s%d",
            (BS->GetIVname(0)).Data(),
            (BS->GetIVname(1)).Data(), BS->UnhashBinNum(bn,1),
            (BS->GetIVname(2)).Data(), BS->UnhashBinNum(bn,2)
            );
        break;
    };


    // instantiate graphs; only needs to be done for each IV1 and IV2 bin, since the
    // horizontal axis of these graphs are all IV0 (hence the if statement here)
    if(BS->UnhashBinNum(bn,0)==0) {

      // instantiate kindep graphs
      grTitle = dihTitle + " A_{LU}[" + A->oaModulationTitle + "] " + 
        " vs. " + grTitleSuffix;
      grName = "kindepOA_" + grNameSuffix;
      kindepGrOA = new TGraphErrors();
      kindepGrOA->SetName(grName);
      kindepGrOA->SetTitle(grTitle);
      grTitle = TString(kindepGrOA->GetTitle()) + " " + polOAstr;
      kindepGrOA->SetTitle(grTitle);
      kindepGrOA->SetLineStyle(2);

      for(int aa=0; aa<N_AMP; aa++) {
        grTitle = dihTitle + " " + TString(A->rfA[aa]->GetTitle()) + 
          " vs. " + grTitleSuffix;
        grName = "kindepMA_A" + TString::Itoa(aa,10) + "_" + grNameSuffix;
        kindepGrMA[aa] = new TGraphAsymmErrors();
        kindepGrMA[aa]->SetName(grName);
        kindepGrMA[aa]->SetTitle(grTitle);
      };


      // instantiate multiGraph, for plotting kindep graphs together
      grTitle = dihTitle + " A_{LU}[" + A->oaModulationTitle + "] " + 
        " vs. " + grTitleSuffix;
      grName = "multiGr_" + grNameSuffix;
      multiGr = new TMultiGraph();
      multiGr->SetName(grName);
      multiGr->SetTitle(grTitle);


      // instantiate chi2/ndf graphs
      grTitle = "#chi^{2}/NDF of " +
        dihTitle + " A_{LU}[" + A->oaModulationTitle + "]_{l.f.} " + 
        " vs. " + grTitleSuffix;
      grName = "chindf_" + grNameSuffix;
      chindfGrOA = new TGraphErrors();
      chindfGrOA->SetName(grName);
      chindfGrOA->SetTitle(grTitle);
      grTitle = TString(chindfGrOA->GetTitle()) + " " + polOAstr;
      chindfGrOA->SetTitle(grTitle);


      // instantiate relative luminosity graphs
      grTitle = "relative luminosity vs. " + grTitleSuffix;
      grName = "rellum_" + grNameSuffix;
      rellumGr = new TGraphErrors();
      rellumGr->SetName(grName);
      rellumGr->SetTitle(grTitle);

    }


    // insert objects into maps (note: these are many-to-one maps, i.e., several
    // bin numbers will map to the same pointer)
    kindepOAmap.insert(std::pair<Int_t,TGraphErrors*>(bn,kindepGrOA));
    multiMap.insert(std::pair<Int_t,TMultiGraph*>(bn,multiGr));
    chindfOAmap.insert(std::pair<Int_t,TGraphErrors*>(bn,chindfGrOA));
    rellumMap.insert(std::pair<Int_t,TGraphErrors*>(bn,rellumGr));
    for(int aa=0; aa<N_AMP; aa++) {
      kindepMAmap[aa].insert(std::pair<Int_t,TGraphAsymmErrors*>(bn,kindepGrMA[aa]) );
    };
  };

  // overall summary plots
  TH1F * chisqDist = new TH1F("chisqDist",
      "#chi^{2} distribution (from linear fit results)",100,0,20);

  // graph asymmetry results
  Float_t asymValueOA,asymErrorOA;
  Float_t asymValueMA[Asymmetry::nAmp];
  Float_t asymErrorMA[Asymmetry::nAmp];
  Float_t asymErrorMAhi[Asymmetry::nAmp];
  Float_t asymErrorMAlo[Asymmetry::nAmp];
  Float_t meanKF;
  Float_t kinValue,kinError;
  Float_t chisq,ndf;
  for(Int_t bn : BS->binVec) {
    A = asymMap.at(bn);

    if( ( A->gridDim==1 && A->fitFunc!=NULL) || 
        ( A->gridDim==2 && A->fitFunc2!=NULL) ) {

      // linear fit result (one-amp only!)
      if(A->gridDim==1) {
        asymValueOA = A->fitFunc->GetParameter(1);
        asymErrorOA = A->fitFunc->GetParError(1);
      } else {
        asymValueOA = 0; // not used
        asymErrorOA = 0; // not used
      };

      // multi-amplitude fit result
      for(int aa=0; aa<N_AMP; aa++) {
        switch(fitAlgo) {
          case 0:
            asymValueMA[aa] = A->rfA[aa]->getVal();
            asymErrorMA[aa] = A->rfA[aa]->getError(); // HESSE (parabolic)
            // MINOS errors (asymmetric)
            // note: if MINOS was not used, `hi` and `lo` will be HESSE errors
            asymErrorMAhi[aa] = TMath::Abs(A->rfA[aa]->getErrorHi());
            asymErrorMAlo[aa] = TMath::Abs(A->rfA[aa]->getErrorLo());
            break;
          case 1:
            asymValueMA[aa] = 0; // not used
            asymErrorMA[aa] = 0;
            asymErrorMAhi[aa] = 0;
            asymErrorMAlo[aa] = 0;
            break;
          case 2:
            asymValueMA[aa] = A->fitFunc2->GetParameter(aa);
            asymErrorMA[aa] = A->fitFunc2->GetParError(aa);
            asymErrorMAhi[aa] = asymErrorMA[aa];
            asymErrorMAlo[aa] = asymErrorMA[aa];
        }
      };


      // divide out mean kinematic factor
      /*
      meanKF = A->kfDist->GetMean();
      asymValueOA /= meanKF;
      for(int aa=0; aa<N_AMP; aa++) asymValueMA[aa] /= meanKF;
      */


      // IV value and uncertainty
      switch(BS->dimensions) {
        case 1:
          kinValue = A->ivDist1->GetMean();
          kinError = A->ivDist1->GetRMS();
          break;
        case 2:
          kinValue = A->ivDist2->GetMean(1);
          kinError = A->ivDist2->GetRMS(1);
          break;
        case 3:
          kinValue = A->ivDist3->GetMean(1);
          kinError = A->ivDist3->GetRMS(1);
          break;
      };
      kinError = 0; // OVERRIDE (since this should be a systematic uncertainty)

      // chi2 and ndf
      if(A->gridDim==1) {
        chisq = A->fitFunc->GetChisquare();
        ndf = A->fitFunc->GetNDF();
      } else {
        chisq = A->fitFunc2->GetChisquare();
        ndf = A->fitFunc2->GetNDF();
      };
      chisqDist->Fill(chisq);

      // set graph points
      kindepGrOA = kindepOAmap.at(bn);
      kindepGrOA->SetPoint(A->B[0],kinValue,asymValueOA);
      kindepGrOA->SetPointError(A->B[0],kinError,asymErrorOA);

      for(int aa=0; aa<N_AMP; aa++) {
        kindepGrMA[aa] = kindepMAmap[aa].at(bn);
        kindepGrMA[aa]->SetPoint(A->B[0],kinValue,asymValueMA[aa]);
        kindepGrMA[aa]->SetPointError(A->B[0],
          kinError,kinError,asymErrorMAlo[aa],asymErrorMAhi[aa]);
      };

      chindfGrOA = chindfOAmap.at(bn);
      chindfGrOA->SetPoint(A->B[0],kinValue,chisq/ndf);

      rellumGr = rellumMap.at(bn);
      rellumGr->SetPoint(A->B[0],kinValue,A->rellum);
      rellumGr->SetPointError(A->B[0],0,A->rellumErr);

    };

  };


  // -- instantiate canvases
  Int_t NB[3]; // # of bins for each IV
  for(int d=0; d<BS->dimensions; d++) NB[d] = BS->GetNbins(d);
  TString canvNameSuffix = "Canv_" + modN;
  TString canvName;
  for(int d=0; d<BS->dimensions; d++) {
    if(d==1) canvNameSuffix += "_bins_" + BS->GetIVname(d);
    else canvNameSuffix += "_" + BS->GetIVname(d);
  };
  Int_t canvX,canvY,divX,divY;
  Int_t canvModX,canvModY,divModX,divModY;
  Int_t canvSize = 800;
  switch(BS->dimensions) {
    case 1:
      canvX=canvSize; canvY=canvSize;
      divX=1; divY=1;
      canvModX=NB[0]*canvSize; canvModY=canvSize;
      divModX=NB[0]; divModY=1;
      break;
    case 2:
      canvX=NB[1]*canvSize; canvY=canvSize;
      divX=NB[1]; divY=1;
      canvModX=NB[1]*canvSize; canvModY=NB[0]*canvSize;
      divModX=NB[1]; divModY=NB[0];
      break;
    case 3:
      canvX=NB[2]*canvSize; canvY=NB[1]*canvSize;
      divX=NB[2]; divY=NB[1];
      canvModX=canvSize; canvModY=canvSize; // (not used) 
      divModX=1; divModY=1; // (not used) 
      break;
  };

  canvName = "kindepOA" + canvNameSuffix;
  TCanvas * kindepOAcanv = new TCanvas(canvName,canvName,canvX,canvY);
  kindepOAcanv->Divide(divX,divY);

  TCanvas * kindepMAcanv[Asymmetry::nAmp];
  for(int aa=0; aa<N_AMP; aa++) {
    canvName = "kindepMA_A"+TString::Itoa(aa,10) + canvNameSuffix;
    kindepMAcanv[aa] = new TCanvas(canvName,canvName,canvX,canvY);
    kindepMAcanv[aa]->Divide(divX,divY);
  };

  canvName = "chindf" + canvNameSuffix;
  TCanvas * chindfCanv = new TCanvas(canvName,canvName,canvX,canvY);
  chindfCanv->Divide(divX,divY);

  canvName = "rellum" + canvNameSuffix;
  TCanvas * rellumCanv = new TCanvas(canvName,canvName,canvX,canvY);
  rellumCanv->Divide(divX,divY);

  canvName = "asymMod" + canvNameSuffix;
  TCanvas * asymModCanv = new TCanvas(canvName,canvName,canvModX,canvModY); 
  asymModCanv->Divide(divModX,divModY);

  canvName = "asymModHist2" + canvNameSuffix;
  TCanvas * asymModHist2Canv = new TCanvas(canvName,canvName,canvModX,canvModY); 
  asymModHist2Canv->Divide(divModX,divModY);

  canvName = "modDist" + canvNameSuffix;
  TCanvas * modDistCanv = new TCanvas(canvName,canvName,canvModX,canvModY); 
  modDistCanv->Divide(divModX,divModY);

  // -- add objects to canvases and graphs to multigraphs
  Int_t pad;
  TGraphAsymmErrors * kindepGrMAclone[Asymmetry::nAmp];
  Int_t binNum;
  if(BS->dimensions==1) {
    binNum = BS->HashBinNum(0);

    kindepGrOA = kindepOAmap.at(binNum);
    kindepOAcanv->cd();
    DrawKinDepGraph(kindepGrOA,BS,0);

    multiGr = multiMap.at(binNum);
    if(includeOAonMultiGr) multiGr->Add(kindepGrOA);
    for(int aa=0; aa<N_AMP; aa++) {
      kindepGrMA[aa] = kindepMAmap[aa].at(binNum);
      kindepMAcanv[aa]->cd();
      DrawKinDepGraph(kindepGrMA[aa],BS,0);
      kindepGrMAclone[aa] = ShiftGraph(kindepGrMA[aa],aa+1);
      multiGr->Add(kindepGrMAclone[aa]);
    };

    chindfGrOA = chindfOAmap.at(binNum);
    chindfCanv->cd();
    DrawSimpleGraph(chindfGrOA,BS,0);

    rellumGr = rellumMap.at(binNum);
    rellumCanv->cd();
    DrawSimpleGraph(rellumGr,BS,0,false);

    for(int b0=0; b0<NB[0]; b0++) {
      binNum = BS->HashBinNum(b0);
      A = asymMap.at(binNum);
      asymModCanv->cd(b0+1);
      if(A->gridDim==1) DrawAsymGr(A->asymGr);
      else DrawAsymGr2(A->asymGr2);
      modDistCanv->cd(b0+1);
      if(A->gridDim==1) A->modDist->Draw();
      else A->modDist2->Draw("colz");
      if(A->gridDim==2) {
        asymModHist2Canv->cd(b0+1);
        A->asymGr2hist->Draw("colz");
      };
    };
  }
  else if(BS->dimensions==2) {
    for(int b1=0; b1<NB[1]; b1++) {
      pad = b1+1;
      binNum = BS->HashBinNum(0,b1);

      kindepGrOA = kindepOAmap.at(binNum);
      kindepOAcanv->cd(pad);
      DrawKinDepGraph(kindepGrOA,BS,0);

      multiGr = multiMap.at(binNum);
      if(includeOAonMultiGr) multiGr->Add(kindepGrOA);

      for(int aa=0; aa<N_AMP; aa++) {
        kindepGrMA[aa] = kindepMAmap[aa].at(binNum);
        kindepMAcanv[aa]->cd(pad);
        DrawKinDepGraph(kindepGrMA[aa],BS,0);
        kindepGrMAclone[aa] = ShiftGraph(kindepGrMA[aa],aa+1);
        multiGr->Add(kindepGrMAclone[aa]);
      };

      chindfGrOA = chindfOAmap.at(binNum);
      chindfCanv->cd(pad);
      DrawSimpleGraph(chindfGrOA,BS,0);

      rellumGr = rellumMap.at(binNum);
      rellumCanv->cd(pad);
      DrawSimpleGraph(rellumGr,BS,0,false);

      for(int b0=0; b0<NB[0]; b0++) {
        binNum = BS->HashBinNum(b0,b1);
        A = asymMap.at(binNum);
        asymModCanv->cd(b0*NB[1]+b1+1);
        if(A->gridDim==1) DrawAsymGr(A->asymGr);
        else DrawAsymGr2(A->asymGr2);
        modDistCanv->cd(b0*NB[1]+b1+1);
        if(A->gridDim==1) A->modDist->Draw();
        else A->modDist2->Draw();
        if(A->gridDim==2) {
          asymModHist2Canv->cd(b0*NB[1]+b1+1);
          A->asymGr2hist->Draw("colz");
        };
      };
    };
  }
  else if(BS->dimensions==3) {
    for(int b1=0; b1<NB[1]; b1++) {
      for(int b2=0; b2<NB[2]; b2++) {
        pad = b1*NB[2]+b2+1;
        binNum = BS->HashBinNum(0,b1,b2);

        kindepGrOA = kindepOAmap.at(binNum);
        kindepOAcanv->cd(pad);
        DrawKinDepGraph(kindepGrOA,BS,0);

        multiGr = multiMap.at(binNum);
        if(includeOAonMultiGr) multiGr->Add(kindepGrOA);

        for(int aa=0; aa<N_AMP; aa++) {
          kindepGrMA[aa] = kindepMAmap[aa].at(binNum);
          kindepMAcanv[aa]->cd(pad);
          DrawKinDepGraph(kindepGrMA[aa],BS,0);
          kindepGrMAclone[aa] = ShiftGraph(kindepGrMA[aa],aa+1);
          multiGr->Add(kindepGrMAclone[aa]);
        };

        chindfGrOA = chindfOAmap.at(binNum);
        chindfCanv->cd(pad);
        DrawSimpleGraph(chindfGrOA,BS,0);

        rellumGr = rellumMap.at(binNum);
        rellumCanv->cd(pad);
        DrawSimpleGraph(rellumGr,BS,0,false);
      };
    };
  };


  // build multiGr canvases
  TCanvas * multiGrCanv;
  TLegend * multiLeg;
  TString legText;
  TString multiGrCanvN;
  TObjArray * multiGrCanvArr;
  Int_t nRows = ( N_AMP + 1 - (includeOAonMultiGr?0:1) )/4 + 1;
  for(Int_t bn : BS->binVec) {
    A = asymMap.at(bn);
    if(A->B[0] == 0) {

      multiGr = multiMap.at(bn);
      multiGrCanvN = multiGr->GetName();
      multiGrCanvN.ReplaceAll("multiGr","multiGrCanv");
      multiGrCanv = new TCanvas(multiGrCanvN,multiGrCanvN,1600,nRows*400);
      multiGrCanv->Divide(4,nRows);

      multiLeg = new TLegend(0.1,0.1,0.9,0.9);

      for(int aa=0; aa<N_AMP; aa++) {
        multiGrCanv->cd(aa+1);
        multiGrCanv->GetPad(aa+1)->SetGrid(0,1);
        kindepGrMA[aa] = kindepMAmap[aa].at(bn);
        kindepGrMA[aa]->Draw("LAPE");
        legText = A->modu[aa]->StateTitle();
        legText += ": ";
        legText += A->modu[aa]->ModulationTitle();
        multiLeg->AddEntry(kindepGrMA[aa],legText,"PLE");
      };
      
      if(includeOAonMultiGr) {
        kindepGrOA = kindepOAmap.at(bn);
        multiGrCanv->cd(N_AMP+1);
        multiGrCanv->GetPad(N_AMP+1)->SetGrid(0,1);
        kindepGrOA->Draw("LAPE");
        legText = A->oaModulationTitle;
        legText += " one-amp result";
        multiLeg->AddEntry(kindepGrOA,legText,"PLE");
      };

      multiGrCanv->cd(4*nRows);
      multiLeg->Draw();

      multiGrCanvArr = new TObjArray();
      multiGrCanvArr->AddLast(multiGrCanv);
      multiGrCanv->Print(
        TString(spinrootDir+"/"+multiGrCanvN+DparamStr+".png"),
        "png");


    };
  };


  // sum distributions (for showing bin boundaries)
  TH1D * ivFullDist1;
  TH2D * ivFullDist2;
  TH3D * ivFullDist3;
  TH1D * modFullDist;
  TH2D * modFullDist2; // for 2d modulation
  TH2D * IVvsModFullDist;

  if(BS->dimensions==1) {
    for(int b0=0; b0<NB[0]; b0++) {
      binNum = BS->HashBinNum(b0);
      A = asymMap.at(binNum);
      if(b0==0) {
        ivFullDist1 = (TH1D*)(A->ivDist1)->Clone();
        SetCloneName(ivFullDist1);
        if(A->gridDim==1) {
          IVvsModFullDist = (TH2D*)(A->IVvsModDist)->Clone();
          SetCloneName(IVvsModFullDist);
          modFullDist = (TH1D*)(A->modDist)->Clone();
          SetCloneName(modFullDist);
        } else {
          modFullDist2 = (TH2D*)(A->modDist2)->Clone();
          SetCloneName(modFullDist2);
        };
      } else {
        ivFullDist1->Add(A->ivDist1);
        if(A->gridDim==1) {
          IVvsModFullDist->Add(A->IVvsModDist);
          modFullDist->Add(A->modDist);
        } else {
          modFullDist2->Add(A->modDist2);
        };
      };
    };
  }
  else if(BS->dimensions==2) {
    for(int b1=0; b1<NB[1]; b1++) {
      for(int b0=0; b0<NB[0]; b0++) {
        binNum = BS->HashBinNum(b0,b1);
        A = asymMap.at(binNum);
        if(b0==0 && b1==0) {
          ivFullDist2 = (TH2D*)(A->ivDist2)->Clone();
          SetCloneName(ivFullDist2);
          if(A->gridDim==1) {
            modFullDist = (TH1D*)(A->modDist)->Clone();
            SetCloneName(modFullDist);
          } else {
            modFullDist2 = (TH2D*)(A->modDist2)->Clone();
            SetCloneName(modFullDist2);
          };
        } else {
          ivFullDist2->Add(A->ivDist2);
          if(A->gridDim==1) modFullDist->Add(A->modDist);
          else modFullDist2->Add(A->modDist2);
        };
      };
    };
  }
  else if(BS->dimensions==3) {
    for(int b2=0; b2<NB[2]; b2++) {
      for(int b1=0; b1<NB[1]; b1++) {
        for(int b0=0; b0<NB[0]; b0++) {
          binNum = BS->HashBinNum(b0,b1,b2);
          A = asymMap.at(binNum);
          if(b0==0 && b1==0 && b2==0) {
            ivFullDist3 = (TH3D*)(A->ivDist3)->Clone();
            SetCloneName(ivFullDist3);
            if(A->gridDim==1) {
              modFullDist = (TH1D*)(A->modDist)->Clone();
              SetCloneName(modFullDist);
            } else {
              modFullDist2 = (TH2D*)(A->modDist2)->Clone();
              SetCloneName(modFullDist2);
            };
          } else {
            ivFullDist3->Add(A->ivDist3);
            if(A->gridDim==1) modFullDist->Add(A->modDist);
            else modFullDist2->Add(A->modDist2);
          };
        };
      };
    };
  };


  // write output to asymFile
  TFile * asymFile = new TFile(asymFileN,"RECREATE");
  // -- Asymmetry objects
  /*
  asymFile->cd();
  printf("--- write Asymmetry objects\n");
  for(Int_t bn : BS->binVec) {
    A = asymMap.at(bn);
    A->StreamData(asymFile);
  };
  */

  // -- "full" distributions
  if(BS->dimensions==1) {
    ivFullDist1->Write();
    if(A->gridDim==1) IVvsModFullDist->Write();
  } else if(BS->dimensions==2) {
    ivFullDist2->Write();
  } else if(BS->dimensions==3) {
    ivFullDist3->Write();
  };
  if(A->gridDim==1) modFullDist->Write();
  else modFullDist2->Write();

  // -- asymmetries and kindep graphs
  printf("--- write kinematic-dependent asymmetries\n");
  for(Int_t bn : BS->binVec) {
    A = asymMap.at(bn);
    A->PrintSettings();

    // first write out the asymmetry vs. modulation graphs
    if(A->gridDim==1) A->asymGr->Write();
    else A->asymGr2->Write();

    // then write out the kindepGrOA after writing out all the
    // relevant asymmetry vs. modulation graphs
    if(A->B[0] + 1 == NB[0]) {
      binNum = BS->HashBinNum(A->B[0], A->B[1], A->B[2]);
      kindepGrOA = kindepOAmap.at(binNum);
      kindepGrOA->Write();
      for(int aa=0; aa<N_AMP; aa++) {
        kindepGrMA[aa] = kindepMAmap[aa].at(binNum);
        kindepGrMA[aa]->Write();
      };
      multiGr = multiMap.at(binNum);
      multiGr->Write();
    };
  };

  // write multiGr canvases
  multiGrCanvArr->Write("multiGrCanvArr",TObject::kSingleKey);


  kindepOAcanv->Write();
  for(int aa=0; aa<N_AMP; aa++) kindepMAcanv[aa]->Write();
  chindfCanv->Write();
  chisqDist->Write();
  rellumCanv->Write();
  if(BS->dimensions==1 || BS->dimensions==2) {
    asymModCanv->Write();
    if(A->gridDim==2) asymModHist2Canv->Write();
    modDistCanv->Write();
  };


  // -- MLM results
  TCanvas * rfCanv[Asymmetry::nAmp];
  TString rfCanvName[Asymmetry::nAmp];

  if(fitAlgo==0) {
    for(Int_t bn : BS->binVec) {
      A = asymMap.at(bn);

      for(int aa=0; aa<N_AMP; aa++) {
        rfCanvName[aa] = "RF_A" + TString::Itoa(aa,10) + "_NLL_" + A->binN;
        rfCanv[aa] = new TCanvas(rfCanvName[aa],rfCanvName[aa],800,800);
        A->rfNLLplot[aa]->Draw();
        rfCanv[aa]->Write();
      };

      Tools::PrintTitleBox("roofit function");
      A->PrintSettings();
      printf("\n");
      for(int ss=0; ss<nSpin; ss++) {
        printf("%s: %s\n",SpinTitle(ss).Data(),A->rfPdfFormu[ss].Data());
      };
      printf("\n");

      printf("fit parameter results:\n");
      for(int aa=0; aa<N_AMP; aa++) {
        printf(" >> A%d = %.5f +/- %.5f  (MINOS: +%.5f %.5f)\n",
            aa, A->rfA[aa]->getVal(), A->rfA[aa]->getError(),
            A->rfA[aa]->getErrorHi(), A->rfA[aa]->getErrorLo());
      };
      for(int dd=0; dd<N_D; dd++) {
        printf(" >> D%d = %.5f +/- %.5f\n",
            dd, A->rfD[dd]->getVal(), A->rfD[dd]->getError() );
      };
      /*
         printf(" >> Y+ = %.3f +/- %.3f\n",
         A->rfYield[0]->getVal(), A->rfYield[0]->getError() );
         printf(" >> Y- = %.3f +/- %.3f\n",
         A->rfYield[1]->getVal(), A->rfYield[1]->getError() );
         */

      printf("\n");

    };
  };


  asymFile->Close();
  catFile->Close();
  printf("--- end %s\n",argv[0]);
  printf("FIT ALGORITHM = %d\n",fitAlgo);
  return 0;
};



void DrawKinDepGraph(TGraph * g_, Binning * B_, Int_t d_) {
  Int_t v_ = B_->ivVar[d_];

  g_->Draw("APE"); // draw once, so we can then format it

  g_->SetLineWidth(2);

  g_->SetMarkerStyle(kFullCircle);
  g_->SetMarkerColor(kBlack);
  g_->SetMarkerSize(1.3);

  // set vertical axis range (it is overridden if the plot's vertical range
  // is larger than the desired range)
  Float_t yMin = ASYM_PLOT_MIN;
  Float_t yMax = ASYM_PLOT_MAX;
  if(g_->GetYaxis()->GetXmin() < yMin) yMin = g_->GetYaxis()->GetXmin();
  if(g_->GetYaxis()->GetXmax() > yMax) yMax = g_->GetYaxis()->GetXmax();
  g_->GetYaxis()->SetRangeUser(yMin,yMax);

  // set horizontal range
  //g_->GetXaxis()->SetLimits(B_->minIV[v_],B_->maxIV[v_]);

  g_->Draw("APE"); // draw again to apply the formatting


  // zero line
  Float_t drawMin = g_->GetXaxis()->GetXmin();
  Float_t drawMax = g_->GetXaxis()->GetXmax();
  TLine * zeroLine = new TLine(drawMin,0,drawMax,0);
  zeroLine->SetLineColor(kBlack);
  zeroLine->SetLineWidth(1.5);
  zeroLine->SetLineStyle(kDashed);
  zeroLine->Draw();
};



void DrawSimpleGraph(TGraphErrors * g_, Binning * B_, Int_t d_, Bool_t setRange) {
  Int_t v_ = B_->ivVar[d_];

  g_->Draw("AP"); // draw once, so we can then format it

  //g_->SetLineWidth(2);

  g_->SetMarkerStyle(kFullCircle);
  g_->SetMarkerColor(kBlack);
  g_->SetMarkerSize(1.3);

  if(setRange) {
    // set vertical axis range (it is overridden if the plot's vertical range
    // is larger than the desired range)
    Float_t yMin = 0;
    Float_t yMax = 2;
    if(g_->GetYaxis()->GetXmin() < yMin) yMin = g_->GetYaxis()->GetXmin() - 0.2;
    if(g_->GetYaxis()->GetXmax() > yMax) yMax = g_->GetYaxis()->GetXmax() + 0.2;
    g_->GetYaxis()->SetRangeUser(yMin,yMax);

    // set horizontal range
    //g_->GetXaxis()->SetLimits(B_->minIV[v_],B_->maxIV[v_]);
  };

  g_->Draw("AP"); // draw again to apply the formatting


  // unity line
  Float_t drawMin = g_->GetXaxis()->GetXmin();
  Float_t drawMax = g_->GetXaxis()->GetXmax();
  TLine * unityLine = new TLine(drawMin,1,drawMax,1);
  unityLine->SetLineColor(kBlack);
  unityLine->SetLineWidth(1.5);
  unityLine->SetLineStyle(kDashed);
  unityLine->Draw();
};


void DrawAsymGr(TGraphErrors * g_) {

  TString titleTmp = g_->GetTitle();
  g_->SetTitle(TString(dihTitle+" "+titleTmp));

  g_->Draw("APE"); // draw once, so we can then format it

  g_->SetLineColor(kBlack);
  g_->SetLineWidth(2);

  g_->SetMarkerStyle(kFullCircle);
  g_->SetMarkerColor(kBlack);
  g_->SetMarkerSize(1.3);

  // set vertical axis range (it is overridden if the plot's vertical range
  // is larger than the desired range)
  Float_t yMin = ASYM_PLOT_MIN;
  Float_t yMax = ASYM_PLOT_MAX;
  if(g_->GetYaxis()->GetXmin() < yMin) yMin = g_->GetYaxis()->GetXmin() - 0.05;
  if(g_->GetYaxis()->GetXmax() > yMax) yMax = g_->GetYaxis()->GetXmax() + 0.05;
  g_->GetYaxis()->SetRangeUser(yMin,yMax);

  g_->Draw("APE"); // draw again to apply the formatting

};


void DrawAsymGr2(TGraph2DErrors * g_) {

  TString titleTmp = g_->GetTitle();
  g_->SetTitle(TString(dihTitle+" "+titleTmp));

  g_->Draw("ERR P"); // draw once, so we can then format it

  g_->SetLineColor(kBlack);
  g_->SetLineWidth(2);

  g_->SetMarkerStyle(kFullCircle);
  g_->SetMarkerColor(kBlack);
  g_->SetMarkerSize(1.3);

  // set vertical axis range (it is overridden if the plot's vertical range
  // is larger than the desired range)
  /*
     Float_t yMin = -0.2;
     Float_t yMax = 0.2;
     if(g_->GetYaxis()->GetXmin() < yMin) yMin = g_->GetYaxis()->GetXmin() - 0.05;
     if(g_->GetYaxis()->GetXmax() > yMax) yMax = g_->GetYaxis()->GetXmax() + 0.05;
     g_->GetYaxis()->SetRangeUser(yMin,yMax);
     */

  g_->Draw("ERR P"); // draw again to apply the formatting

};


void SetCloneName(TH1 * clone_) {
  TString cloneName,cloneTitle;
  cloneName = clone_->GetName();
  cloneName.ReplaceAll("Dist","FullDist");
  cloneName.ReplaceAll("0","");
  cloneTitle = clone_->GetTitle();
  cloneTitle(TRegexp("::.*$")) = "";
  clone_->SetName(cloneName);
  clone_->SetTitle(cloneTitle);
};

// shift a graph's points to the right slightly (a clone of the graph, with shifted
// points, is returned)
TGraphAsymmErrors * ShiftGraph(TGraphAsymmErrors * gr, Int_t nShift) {
  TGraphAsymmErrors * retGr = new TGraphAsymmErrors();
  Double_t * grX = gr->GetX();
  Double_t * grY = gr->GetY();
  Double_t * grEXhi = gr->GetEXhigh();
  Double_t * grEXlo = gr->GetEXlow();
  Double_t * grEYhi = gr->GetEYhigh();
  Double_t * grEYlo = gr->GetEYlow();
  for(int nn=0; nn<gr->GetN(); nn++) {
    //retGr->SetPoint(nn, grX[nn]+nShift*0.01, grY[nn]); // shifting enabled
    retGr->SetPoint(nn, grX[nn], grY[nn]); // shifting disabled
    retGr->SetPointError(nn, grEXlo[nn], grEXhi[nn], grEYlo[nn], grEYhi[nn]);
  };

  switch(nShift) {
    case 1:
      retGr->SetLineColor(N_AMP==1?kGray+3:kGray+1); 
      //retGr->SetLineStyle(2);
      retGr->SetMarkerStyle(kFullCircle);
      break;
    case 2:
      retGr->SetLineColor(kRed); 
      //retGr->SetLineStyle(1);
      retGr->SetMarkerStyle(kFullCircle);
      break;
    case 3:
      retGr->SetLineColor(kAzure+1);
      //retGr->SetLineStyle(3);
      retGr->SetMarkerStyle(kFullCircle);
      break;
    case 4:
      retGr->SetLineColor(kViolet+1);
      retGr->SetMarkerStyle(kFullCircle);
      break;
    case 5:
      retGr->SetLineColor(kGreen+1);
      retGr->SetMarkerStyle(kFullCircle);
      break;
    case 6:
      retGr->SetLineColor(kCyan);
      retGr->SetMarkerStyle(kFullCircle);
      break;
    case 7:
      retGr->SetLineColor(kOrange);
      retGr->SetMarkerStyle(kFullCircle);
      break;
    case 8:
      retGr->SetLineColor(kOrange-7);
      retGr->SetMarkerStyle(kFullCircle);
      break;
    default: retGr->SetLineColor(kGray);
  };


  retGr->SetMarkerColor(kBlack);
  retGr->SetLineWidth(2);
  retGr->SetMarkerSize(1.3);

  gr->SetLineColor(retGr->GetLineColor());
  gr->SetLineStyle(retGr->GetLineStyle());
  gr->SetMarkerStyle(retGr->GetMarkerStyle());
  gr->SetMarkerColor(kBlack);
  gr->SetLineWidth(2);
  gr->SetMarkerSize(1.3);

  return retGr;
};
