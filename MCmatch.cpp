#include <cstdlib>
#include <iostream>

// ROOT
#include "TFile.h"
#include "TString.h"
#include "TMath.h"
#include "TRegexp.h"
#include "TH1.h"
#include "TH2.h"
#include "TProfile.h"
#include "TCanvas.h"

// DiSpin
#include "Constants.h"
#include "Tools.h"
#include "DIS.h"
#include "Dihadron.h"
#include "Trajectory.h"
#include "EventTree.h"

Float_t Delta(Float_t vGen, Float_t vRec, Bool_t adjAngle=false);

int main(int argc, char** argv) {

  // ARGUMENTS
  TString infiles = "outroot/*.root";
  Int_t whichPair = EncodePairType(kPip,kPim);
  if(argc>1) infiles = TString(argv[1]);
  if(argc>2) whichPair = (Int_t)strtof(argv[2],NULL);


  // open input files and define output file
  EventTree * ev = new EventTree(infiles,whichPair);
  TFile * outfile = new TFile("match.root","RECREATE");

  Int_t whichHad[2];
  TString hadName[2];
  TString hadTitle[2];
  int h;
  DecodePairType(whichPair,whichHad[qA],whichHad[qB]);
  for(int h=0; h<2; h++) {
    hadName[h] = PairHadName(whichHad[qA],whichHad[qB],h);
    hadTitle[h] = PairHadTitle(whichHad[qA],whichHad[qB],h);
  };


  // define histograms
  // - notation `D` represents a matching criterium metric, to allow for
  //   flexible study of its correlation with other variables or criteria;
  // - there must already be some base-levelmatching implemented however; in
  //   this way, `D` is interpreted as a matching criteria refinement
  // - you are also allowed to also cut on `D`, to see its effect on other
  //   correlations
  const Int_t NBINS = 100;
  const Float_t Dlim = 1;
  TH1F * Ddist = new TH1F("Ddist","D distribution",NBINS,0,10);
  TH1F * DdistZoom = new TH1F("DdistZoom","D distribution (zoom)",NBINS,0,Dlim);

  TH2F * hadEDeltaVsD[2];
  TH2F * hadPtDeltaVsD[2];
  TH2F * hadThetaDiffVsD[2];
  TH2F * hadPhiDiffVsD[2];
  for(h=0; h<2; h++) {
    hadEDeltaVsD[h] = new TH2F(
      TString(hadName[h]+"EDiffVsD"),
      TString(hadTitle[h]+" #DeltaE vs. D (no D cut)"),
      NBINS,0,Dlim,NBINS,-1,1);
    hadPtDeltaVsD[h] = new TH2F(
      TString(hadName[h]+"PtDiffVsD"),
      TString(hadTitle[h]+" #Deltap_{T} vs. D (no D cut)"),
      NBINS,0,Dlim,NBINS,-1,1);
    hadThetaDiffVsD[h] = new TH2F(
      TString(hadName[h]+"ThetaDiffVsD"),
      TString(hadTitle[h]+" #theta^{gen}-#theta^{rec} vs. D (no D cut)"),
      NBINS,0,Dlim,NBINS,-10,10);
    hadPhiDiffVsD[h] = new TH2F(
      TString(hadName[h]+"PhiDiffVsD"),
      TString(hadTitle[h]+" #phi^{gen}-#phi^{rec} vs. D (no D cut)"),
      NBINS,0,Dlim,NBINS,-0.2,0.2);
  };

  TH2F * hadECorr[2];
  TH2F * hadPtCorr[2];
  TH2F * hadPhiCorr[2];
  TH2F * hadThetaCorr[2];
  TH1F * hadEDelta[2];
  TH1F * hadPtDelta[2];
  for(h=0; h<2; h++) {

    hadECorr[h] = new TH2F(
      TString(hadName[h]+"ECorr"),
      TString(hadTitle[h]+" E^{rec} vs. E^{gen}"),
      NBINS,0,10,NBINS,0,10);
    hadPtCorr[h] = new TH2F(
      TString(hadName[h]+"PtCorr"),
      TString(hadTitle[h]+" p_{T}^{rec} vs. p_{T}^{gen}"),
      NBINS,0,2.5,NBINS,0,2.5);
    hadPhiCorr[h] = new TH2F(
      TString(hadName[h]+"PhiCorr"),
      TString(hadTitle[h]+" #phi^{rec} vs. #phi^{gen}"),
      NBINS,-PIe,PIe,NBINS,-PIe,PIe);
    hadThetaCorr[h] = new TH2F(
      TString(hadName[h]+"ThetaCorr"),
      TString(hadTitle[h]+" #theta^{rec} vs. #theta^{gen}"),
      NBINS,0,45,NBINS,0,45);
    hadEDelta[h] = new TH1F(
      TString(hadName[h]+"hadEDelta"),
      TString(hadTitle[h]+" #DeltaE distribution"),
      NBINS,-1,1);
    hadPtDelta[h] = new TH1F(
      TString(hadName[h]+"hadPtDelta"),
      TString(hadTitle[h]+" #Deltap_{T} distribution"),
      NBINS,-1,1);
  };


  // event loop
  Double_t D;
  printf("begin loop through %lld events...\n",ev->ENT);
  for(int i=0; i<ev->ENT; i++) {

    ev->GetEvent(i);

    // event must be "valid" (passing all cuts), and must have recon/gen 
    // matching (D is cut later)
    if(ev->Valid() && ev->gen_eleIsMatch && 
       ev->gen_hadIsMatch[qA] && ev->gen_hadIsMatch[qB]) {

    // define `D` //////////////////////////////
    D = ev->gen_eleMatchDist;
    ////////////////////////////////////////////

      Ddist->Fill(D);
      DdistZoom->Fill(D);

      for(h=0; h<2; h++) {
        hadEDeltaVsD[h]->Fill(D,Delta(ev->gen_hadE[h],ev->hadE[h]));
        hadPtDeltaVsD[h]->Fill(D,Delta(ev->gen_hadPt[h],ev->hadPt[h]));

        hadThetaDiffVsD[h]->Fill(D,ev->gen_hadTheta[h]-ev->hadTheta[h]);
        hadPhiDiffVsD[h]->Fill(D,
          Tools::AdjAngle(ev->gen_hadPhi[h]-ev->hadPhi[h]));
      }

      // D cut
      if(true) {
        for(h=0; h<2; h++) {
          hadECorr[h]->Fill(ev->gen_hadE[h],ev->hadE[h]);
          hadPtCorr[h]->Fill(ev->gen_hadPt[h],ev->hadPt[h]);
          hadPhiCorr[h]->Fill(ev->gen_hadPhi[h],ev->hadPhi[h],1);
          hadThetaCorr[h]->Fill(ev->gen_hadTheta[h],ev->hadTheta[h]);
          hadEDelta[h]->Fill(Delta(ev->gen_hadE[h],ev->hadE[h]));
          hadPtDelta[h]->Fill(Delta(ev->gen_hadPt[h],ev->hadPt[h]));
        };
      };
    };
  };

  //write output
  Ddist->Write();
  DdistZoom->Write();
  for(h=0; h<2; h++) {
    hadEDeltaVsD[h]->Write();
    hadPtDeltaVsD[h]->Write();
    hadPhiDiffVsD[h]->Write();
    hadThetaDiffVsD[h]->Write();
  };
  for(h=0; h<2; h++) {
    hadECorr[h]->Write();
    hadPtCorr[h]->Write();
    hadPhiCorr[h]->Write();
    hadThetaCorr[h]->Write();
    hadEDelta[h]->Write();
    hadPtDelta[h]->Write();
  };

  outfile->Close();
};

Float_t Delta(Float_t vGen, Float_t vRec, Bool_t adjAngle) {
  if(vRec==0) {
    fprintf(stderr,"ERROR: vRec==0\n");
    return UNDEF;
  }
  if(adjAngle) return Tools::AdjAngle(vGen-vRec)/vRec;
  else return (vGen-vRec)/vRec;
};
