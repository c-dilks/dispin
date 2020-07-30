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
enum parEnum {kEle,kHadA,kHadB,nPar};

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
  const Int_t NBINS = 150;
  const Float_t Dlim = 10;
  TH1D * Ddist = new TH1D("Ddist","D distribution",NBINS,0,10);
  TH1D * DdistZoom = new TH1D("DdistZoom","D distribution (zoom)",NBINS,0,Dlim);
  TH2D * DvsD = new TH2D("DvsD","D correlation",NBINS,0,Dlim,NBINS,0,Dlim);

  // match fractions
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

  // deltaTheta vs. deltaPhi
  TH2D * deltaThetaVsDeltaPhi[nPar];
  TString parName[nPar];
  TString parTitle[nPar];
  parName[kEle] = "electron"; parTitle[kEle] = "e^{-}";
  parName[kHadA] = hadName[qA];
  parName[kHadB] = hadName[qB];
  for(int p=0; p<nPar; p++) {
    deltaThetaVsDeltaPhi[p] = new TH2D(
      TString("deltaThetaVsDeltaPhi_"+parName[p]),
      TString(parTitle[p]+" #Delta#theta vs. #Delta#phi [deg];#Delta#phi [deg];#Delta#theta [deg]"),
      NBINS,-6,6,NBINS,-6,6);
  };

  // deltaKinematic vs. D
  TH2D * deltaEVsD[3];
  TH2D * deltaPtVsD[3];
  TH2D * deltaThetaVsD[3];
  TH2D * deltaPhiVsD[3];
  for(int p=0; p<nPar; p++) {
    deltaEVsD[p] = new TH2D(
      TString(parName[p]+"DeltaEVsD"),
      TString(parTitle[p]+" #DeltaE vs. D (no D cut)"),
      NBINS,0,Dlim,NBINS,-1,1);
    deltaPtVsD[p] = new TH2D(
      TString(parName[p]+"DeltaPtVsD"),
      TString(parTitle[p]+" #Deltap_{T} vs. D (no D cut)"),
      NBINS,0,Dlim,NBINS,-1,1);
    deltaThetaVsD[p] = new TH2D(
      TString(parName[p]+"DeltaThetaVsD"),
      TString(parTitle[p]+" #Delta#theta vs. D (no D cut)"),
      NBINS,0,Dlim,NBINS,-10,10);
    deltaPhiVsD[p] = new TH2D(
      TString(parName[p]+"DeltaPhiVsD"),
      TString(parTitle[p]+" #Delta#phi vs. D (no D cut)"),
      NBINS,0,Dlim,NBINS,-0.2,0.2);
  };

  // recon vs generated kinematics
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

  // dihadron plots
  TH2D * deltaPhiRVsRT = new TH2D("di_deltaPhiRVsRT",
    "#Delta#phi_{R} vs. recon R_{T};R_{T};#Delta#phi_{R}",
    NBINS,0,1.5,
    NBINS,-0.8,0.8);
  TH2D * deltaPhiHVsPhPerp = new TH2D("di_deltaPhiHVsPhPerp",
    "#Delta#phi_{h} vs. recon P_{h}^{perp};P_{h}^{perp};#Delta#phi_{h}",
    NBINS,0,2,
    NBINS,-PI,PI);
  TH2D * YHvsXF = new TH2D("di_YHvsXF",
    "Breit frame rapidity y_{H} vs. x_{F};x_{F};y_{H}",
    NBINS,-1,1,
    NBINS,-4,4);

  TH2D * deltaMhVsMh = new TH2D("di_deltaMhVsMh",
    "#Delta M_{h} vs. recon M_{h};M_{h};#Delta M_{h}",
    NBINS,0,3,
    NBINS,-0.5,0.5);
  TH2D * deltaXVsX = new TH2D("di_deltaXVsX",
    "#Delta x vs. recon x;x;#Delta x",
    NBINS,0,1,
    NBINS,-1,1);
  TH2D * deltaZVsZ = new TH2D("di_deltaZVsZ",
    "#Delta z vs. recon z;z;#Delta z",
    NBINS,0,1,
    NBINS,-1,1);
    

  
  //------------------------
  // MATCH CUT SETTINGS
  //------------------------

  // matching cuts
  Double_t deltaThetaMax[3];
  Double_t deltaPhiMax[3];
  deltaThetaMax[kEle] = 1.0*TMath::DegToRad();
  deltaThetaMax[kHadA] = 1.0*TMath::DegToRad();
  deltaThetaMax[kHadB] = 1.0*TMath::DegToRad();
  deltaPhiMax[kEle] = 3.0*TMath::DegToRad();
  deltaPhiMax[kHadA] = 3.0*TMath::DegToRad();
  deltaPhiMax[kHadB] = 3.0*TMath::DegToRad();

  // tilt angle for ellipse cut
  Double_t tilt[3];
  for(int p=0; p<nPar; p++) {
    tilt[p] = TMath::Tan(deltaThetaMax[p]/deltaPhiMax[p]);
  };
  tilt[kEle] *= 0.9;
  if(PartCharge(whichHad[qA])==1) tilt[kHadA] *= -0.3;
  if(PartCharge(whichHad[qB])==1) tilt[kHadB] *= -0.3;


  // define variables
  Double_t deltaE[3];
  Double_t deltaPt[3];
  Double_t deltaTheta[3];
  Double_t deltaPhi[3];
  Double_t deltaMh;
  Double_t deltaX;
  Double_t deltaZ;
  Double_t deltaPhiH;
  Double_t deltaPhiR;
  Double_t E[3];
  Double_t D;
  Bool_t cutD;
  Bool_t cutEllipse;
  Bool_t cutRectangle;


  //---------------------
  // event loop
  //---------------------
  printf("begin loop through %lld events...\n",ev->ENT);
  for(int i=0; i<ev->ENT; i++) {

    ev->GetEvent(i);

    // convert theta to radians (for calculations)
    ev->eleTheta *= TMath::DegToRad();
    ev->hadTheta[qA] *= TMath::DegToRad();
    ev->hadTheta[qB] *= TMath::DegToRad();
    ev->gen_eleTheta *= TMath::DegToRad();
    ev->gen_hadTheta[qA] *= TMath::DegToRad();
    ev->gen_hadTheta[qB] *= TMath::DegToRad();

    // event must be "valid" (passing all cuts)
    if(ev->Valid()) {

      // fill denominator distributions for matchFrac
      matchFracVsMh_den->Fill(ev->Mh);
      matchFracVsX_den->Fill(ev->x);
      matchFracVsZ_den->Fill(ev->Zpair);


      // event must have base-level recon/gen matching
      if(ev->gen_eleIsMatch && 
         ev->gen_hadIsMatch[qA] && ev->gen_hadIsMatch[qB]) {


        // compute deltas
        deltaE[kEle] = ev->gen_eleE - ev->eleE;
        deltaE[kHadA] = ev->gen_hadE[qA] - ev->hadE[qA];
        deltaE[kHadB] = ev->gen_hadE[qB] - ev->hadE[qB];
        deltaPt[kEle] = ev->gen_elePt - ev->elePt;
        deltaPt[kHadA] = ev->gen_hadPt[qA] - ev->hadPt[qA];
        deltaPt[kHadB] = ev->gen_hadPt[qB] - ev->hadPt[qB];
        deltaTheta[kEle] = Tools::AdjAngle(ev->gen_eleTheta - ev->eleTheta);
        deltaTheta[kHadA] = Tools::AdjAngle(ev->gen_hadTheta[qA] - ev->hadTheta[qA]);
        deltaTheta[kHadB] = Tools::AdjAngle(ev->gen_hadTheta[qB] - ev->hadTheta[qB]);
        deltaPhi[kEle] = Tools::AdjAngle(ev->gen_elePhi - ev->elePhi);
        deltaPhi[kHadA] = Tools::AdjAngle(ev->gen_hadPhi[qA] - ev->hadPhi[qA]);
        deltaPhi[kHadB] = Tools::AdjAngle(ev->gen_hadPhi[qB] - ev->hadPhi[qB]);

        deltaX = ev->gen_x - ev->x;
        deltaZ = ev->gen_Zpair - ev->Zpair;
        deltaMh = ev->gen_Mh - ev->Mh;
        deltaPhiR = Tools::AdjAngle( ev->gen_PhiRp - ev->PhiRp );
        deltaPhiH = Tools::AdjAngle( ev->gen_PhiH - ev->PhiH );



        // tilted ellipse
        // - E is a "radius", where E<1 satisfies the tilted ellipse cut
        for(int p=0; p<nPar; p++) {
          E[p] =
              TMath::Power(
                (deltaTheta[p]*TMath::Cos(tilt[p])+deltaPhi[p]*TMath::Sin(tilt[p])) /
                deltaThetaMax[p],2) + 
              TMath::Power(
                (deltaTheta[p]*TMath::Sin(tilt[p])-deltaPhi[p]*TMath::Cos(tilt[p])) /
                deltaPhiMax[p],2); 
        };


        // define custom cut metric `D` //////////////////////////////
        D = E[kEle] + E[kHadA] + E[kHadB]; // total elliptic radius

        DvsD->Fill(E[kHadB],E[kHadA]);
        //DvsD->Fill(ev->gen_hadMatchDist[qB],ev->gen_hadMatchDist[qA]);

        Ddist->Fill(D);
        DdistZoom->Fill(D);

        for(int p=0; p<nPar; p++) {
          deltaEVsD[p]->Fill(D,deltaE[p]);
          deltaPtVsD[p]->Fill(D,deltaPt[p]);
          deltaThetaVsD[p]->Fill(D,deltaTheta[p]*TMath::RadToDeg());
          deltaPhiVsD[p]->Fill(D,deltaPhi[p]);
        }
        //////////////////////////////////////////////////////////////////



        //-----------------------------------
        // refined matching cut
        //-----------------------------------

        cutD = D<1;
        cutEllipse = E[kEle]<1 && E[kHadA]<1 && E[kHadB]<1;
        cutRectangle = true;
        for(int p=0; p<nPar; p++) {
          if(TMath::Abs(deltaTheta[p]) >= deltaThetaMax[p]) cutRectangle=false;
          if(TMath::Abs(deltaPhi[p]) >= deltaPhiMax[p]) cutRectangle=false;
        };

        //if(true)
        //if(cutD)
        if(cutEllipse)
        //if(cutRectangle)
        {

          for(int p=0; p<nPar; p++) {
            deltaThetaVsDeltaPhi[p]->Fill(
              deltaPhi[p]*TMath::RadToDeg(),
              deltaTheta[p]*TMath::RadToDeg()
            );
          };

          // fill numerator distributions for matchFrac
          matchFracVsMh->Fill(ev->Mh);
          matchFracVsX->Fill(ev->x);
          matchFracVsZ->Fill(ev->Zpair);

          // fill other distributions
          for(h=0; h<2; h++) {
            hadECorr[h]->Fill(ev->gen_hadE[h],ev->hadE[h]);
            hadPtCorr[h]->Fill(ev->gen_hadPt[h],ev->hadPt[h]);
            hadPhiCorr[h]->Fill(ev->gen_hadPhi[h],ev->hadPhi[h],1);
            hadThetaCorr[h]->Fill(ev->gen_hadTheta[h]*TMath::RadToDeg(),ev->hadTheta[h]*TMath::RadToDeg());
            hadEDelta[h]->Fill(Delta(ev->gen_hadE[h],ev->hadE[h]));
            hadPtDelta[h]->Fill(Delta(ev->gen_hadPt[h],ev->hadPt[h]));
          };

          deltaPhiRVsRT->Fill(ev->RT,deltaPhiR);
          deltaPhiHVsPhPerp->Fill(ev->PhPerp,deltaPhiH);
          YHvsXF->Fill(ev->hadXF[qA],ev->GetBreitRapidity(qA)); // pi+

          deltaMhVsMh->Fill(ev->Mh,deltaMh);
          deltaXVsX->Fill(ev->x,deltaX);
          deltaZVsZ->Fill(ev->Zpair,deltaZ);
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
  for(int p=0; p<nPar; p++) {
    deltaThetaVsDeltaPhi[p]->Write();
    deltaEVsD[p]->Write();
    deltaPtVsD[p]->Write();
    deltaPhiVsD[p]->Write();
    deltaThetaVsD[p]->Write();
  };
  for(h=0; h<2; h++) {
    hadECorr[h]->Write();
    hadPtCorr[h]->Write();
    hadPhiCorr[h]->Write();
    hadThetaCorr[h]->Write();
    hadEDelta[h]->Write();
    hadPtDelta[h]->Write();
  };

  deltaPhiRVsRT->Write();
  deltaPhiHVsPhPerp->Write();
  YHvsXF->Write();

  deltaMhVsMh->Write();
  deltaXVsX->Write();
  deltaZVsZ->Write();

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
