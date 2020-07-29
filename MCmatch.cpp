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
  const Float_t Dlim = 10;
  TH1D * Ddist = new TH1D("Ddist","D distribution",NBINS,0,10);
  TH1D * DdistZoom = new TH1D("DdistZoom","D distribution (zoom)",NBINS,0,Dlim);
  TH2D * DvsD = new TH2D("DvsD","D correlation",NBINS,0,Dlim,NBINS,0,Dlim);

  Int_t nMbins = 5;
  TH1D * matchFracVsMh = new TH1D("matchFracVsMh","F vs recon M_{h}",nMbins,0,3);
  TH1D * matchFracVsMh_den = new TH1D("matchFracVsMh_den","F vs recon M_{h}",nMbins,0,3);
  TH1D * matchFracVsX = new TH1D("matchFracVsX","F vs recon x",nMbins,0,1);
  TH1D * matchFracVsX_den = new TH1D("matchFracVsX_den","F vs recon x",nMbins,0,1);
  TH1D * matchFracVsZ = new TH1D("matchFracVsZ","F vs recon z",nMbins,0,1);
  TH1D * matchFracVsZ_den = new TH1D("matchFracVsZ_den","F vs recon z",nMbins,0,1);
  matchFracVsMh->Sumw2();
  matchFracVsMh_den->Sumw2();
  matchFracVsX->Sumw2();
  matchFracVsX_den->Sumw2();
  matchFracVsZ->Sumw2();
  matchFracVsZ_den->Sumw2();

  TH2D * hadEDeltaVsD[2];
  TH2D * hadPtDeltaVsD[2];
  TH2D * hadThetaDiffVsD[2];
  TH2D * hadPhiDiffVsD[2];
  for(h=0; h<2; h++) {
    hadEDeltaVsD[h] = new TH2D(
      TString(hadName[h]+"EDiffVsD"),
      TString(hadTitle[h]+" #DeltaE vs. D (no D cut)"),
      NBINS,0,Dlim,NBINS,-1,1);
    hadPtDeltaVsD[h] = new TH2D(
      TString(hadName[h]+"PtDiffVsD"),
      TString(hadTitle[h]+" #Deltap_{T} vs. D (no D cut)"),
      NBINS,0,Dlim,NBINS,-1,1);
    hadThetaDiffVsD[h] = new TH2D(
      TString(hadName[h]+"ThetaDiffVsD"),
      TString(hadTitle[h]+" #theta^{gen}-#theta^{rec} vs. D (no D cut)"),
      NBINS,0,Dlim,NBINS,-10,10);
    hadPhiDiffVsD[h] = new TH2D(
      TString(hadName[h]+"PhiDiffVsD"),
      TString(hadTitle[h]+" #phi^{gen}-#phi^{rec} vs. D (no D cut)"),
      NBINS,0,Dlim,NBINS,-0.2,0.2);
  };

  TH2D * hadECorr[2];
  TH2D * hadPtCorr[2];
  TH2D * hadPhiCorr[2];
  TH2D * hadThetaCorr[2];
  TH1D * hadEDelta[2];
  TH1D * hadPtDelta[2];
  for(h=0; h<2; h++) {

    hadECorr[h] = new TH2D(
      TString(hadName[h]+"ECorr"),
      TString(hadTitle[h]+" E^{rec} vs. E^{gen}"),
      NBINS,0,10,NBINS,0,10);
    hadPtCorr[h] = new TH2D(
      TString(hadName[h]+"PtCorr"),
      TString(hadTitle[h]+" p_{T}^{rec} vs. p_{T}^{gen}"),
      NBINS,0,2.5,NBINS,0,2.5);
    hadPhiCorr[h] = new TH2D(
      TString(hadName[h]+"PhiCorr"),
      TString(hadTitle[h]+" #phi^{rec} vs. #phi^{gen}"),
      NBINS,-PIe,PIe,NBINS,-PIe,PIe);
    hadThetaCorr[h] = new TH2D(
      TString(hadName[h]+"ThetaCorr"),
      TString(hadTitle[h]+" #theta^{rec} vs. #theta^{gen}"),
      NBINS,0,45,NBINS,0,45);
    hadEDelta[h] = new TH1D(
      TString(hadName[h]+"hadEDelta"),
      TString(hadTitle[h]+" #DeltaE distribution"),
      NBINS,-1,1);
    hadPtDelta[h] = new TH1D(
      TString(hadName[h]+"hadPtDelta"),
      TString(hadTitle[h]+" #Deltap_{T} distribution"),
      NBINS,-1,1);
  };


  // event loop
  Double_t D,eleD;
  Double_t hadD[2];
  printf("begin loop through %lld events...\n",ev->ENT);
  for(int i=0; i<ev->ENT; i++) {

    ev->GetEvent(i);

    // event must be "valid" (passing all cuts)
    if(ev->Valid()) {

      // fill denominator distributions for matchFrac
      matchFracVsMh_den->Fill(ev->Mh);
      matchFracVsX_den->Fill(ev->x);
      matchFracVsZ_den->Fill(ev->Zpair);


      // event must have recon/gen matching (D is cut later)
      if(ev->gen_eleIsMatch && 
         ev->gen_hadIsMatch[qA] && ev->gen_hadIsMatch[qB]) {

        // define `D` //////////////////////////////
        ///*
        eleD =
            TMath::Power( 
              ev->gen_eleTheta - ev->eleTheta / 
              (1.0), 2) +
            TMath::Power( 
              Tools::AdjAngle(ev->gen_elePhi - ev->elePhi) / 
              (3.0*TMath::DegToRad()), 2);
        for(h=0; h<2; h++) {
          hadD[h] =
              TMath::Power( 
                ev->gen_hadTheta[h] - ev->hadTheta[h] / 
                (1.0), 2) +
              TMath::Power( 
                Tools::AdjAngle(ev->gen_hadPhi[h] - ev->hadPhi[h]) / 
                (3.0*TMath::DegToRad()), 2);
        };
        D = eleD + hadD[qA] + hadD[qB];
        //*/
        /*
        D = TMath::Sqrt(
          TMath::Power(ev->gen_eleMatchDist,2)+
          TMath::Power(ev->gen_hadMatchDist[qA],2)+
          TMath::Power(ev->gen_hadMatchDist[qB],2));
        */
        //D = TMath::Abs(ev->gen_hadTheta[qA]-ev->hadTheta[qA]);
        //D = TMath::Abs(Tools::AdjAngle(ev->gen_hadPhi[qA]-ev->hadPhi[qA]))*TMath::RadToDeg();
        ////////////////////////////////////////////
        //
        //DvsD->Fill(hadD[qB],hadD[qA]);
        DvsD->Fill(ev->gen_hadMatchDist[qB],ev->gen_hadMatchDist[qA]);
        //

        Ddist->Fill(D);
        DdistZoom->Fill(D);

        for(h=0; h<2; h++) {
          hadEDeltaVsD[h]->Fill(D,Delta(ev->gen_hadE[h],ev->hadE[h]));
          hadPtDeltaVsD[h]->Fill(D,Delta(ev->gen_hadPt[h],ev->hadPt[h]));

          hadThetaDiffVsD[h]->Fill(D,ev->gen_hadTheta[h]-ev->hadTheta[h]);
          hadPhiDiffVsD[h]->Fill(D,
            Tools::AdjAngle(ev->gen_hadPhi[h]-ev->hadPhi[h]));
        }

        /////////////////////////////////
        // refined matching cut ("D cut")
        /////////////////////////////////

        // (deltaPhi, deltaTheta) ellipse cut
        if(D<1) {
        // (deltaPhi, deltaTheta) box cut
        /*
        if( 
          TMath::Abs(ev->gen_eleTheta-ev->eleTheta) < 1.0 &&
          TMath::Abs(Tools::AdjAngle(ev->gen_elePhi-ev->elePhi)) < 3.0*TMath::DegToRad() &&
          TMath::Abs(ev->gen_hadTheta[qA]-ev->hadTheta[qA]) < 1.0 &&
          TMath::Abs(Tools::AdjAngle(ev->gen_hadPhi[qA]-ev->hadPhi[qA])) < 3.0*TMath::DegToRad() &&
          TMath::Abs(ev->gen_hadTheta[qB]-ev->hadTheta[qB]) < 1.0 &&
          TMath::Abs(Tools::AdjAngle(ev->gen_hadPhi[qB]-ev->hadPhi[qB])) < 3.0*TMath::DegToRad() ) {
          */



          // fill numerator distributions for matchFrac
          matchFracVsMh->Fill(ev->Mh);
          matchFracVsX->Fill(ev->x);
          matchFracVsZ->Fill(ev->Zpair);

          // fill other distributions
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
  };

  
  // compute matching fractions
  ///*
  matchFracVsMh->Divide(matchFracVsMh_den);
  matchFracVsX->Divide(matchFracVsX_den);
  matchFracVsZ->Divide(matchFracVsZ_den);
  //*/

  //write output
  matchFracVsMh->Write();
  matchFracVsX->Write();
  matchFracVsZ->Write();
  ///*
  matchFracVsMh_den->Write();
  matchFracVsX_den->Write();
  matchFracVsZ_den->Write();
  DvsD->Write();
  //*/
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
