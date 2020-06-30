#include <cstdlib>
#include <iostream>

// ROOT
#include "TFile.h"
#include "TTree.h"
#include "TChain.h"
#include "TString.h"
#include "TMath.h"
#include "TSystem.h"
#include "TRegexp.h"
#include "TH1.h"
#include "TH2.h"
#include "TCanvas.h"
#include "TGraph.h"

// DihBsa
#include "Constants.h"
#include "Tools.h"
#include "DIS.h"
#include "Trajectory.h"
#include "Dihadron.h"
#include "EventTree.h"

TFile * outfile;
TString inDir;
Int_t whichPair;
Int_t whichHad[2];
TString hadName[2];
TString hadTitle[2];
TH1D * pr;

void normalize(TH2D * d);

int main(int argc, char** argv) {

  // RUN NUMBER RANGE
  const Int_t runnumMin = 5000;
  const Int_t runnumMax = 5300;
  Int_t runnumBins = runnumMax - runnumMin;


  // ARGUMENTS
  inDir = "outroot.fall18";
  whichPair = EncodePairType(kPip,kPim);
  if(argc>1) inDir = TString(argv[1]);
  if(argc>2) whichPair = (Int_t)strtof(argv[2],NULL);


  // get hadron pair from whichPair; note that in the print out, the 
  // order of hadron 0 and 1 is set by Constants::dihHadIdx
  printf("whichPair = 0x%x\n",whichPair);
  DecodePairType(whichPair,whichHad[qA],whichHad[qB]);
  for(int h=0; h<2; h++) {
    hadName[h] = PairHadName(whichHad[qA],whichHad[qB],h);
    hadTitle[h] = PairHadTitle(whichHad[qA],whichHad[qB],h);
    printf("hadron %d:  idx=%d  name=%s  title=%s\n",
        h,dihHadIdx(whichHad[qA],whichHad[qB],h),hadName[h].Data(),hadTitle[h].Data());
  };

  EventTree * ev = new EventTree(TString(inDir+"/*.root"),whichPair);


  outfile = new TFile("vsRun.root","RECREATE");

  const Int_t NBINS = 100; // number of bins for kinematic variable
  const Int_t NPHIBINS = 50; // number of bins for sector dependence
  Float_t deltaPhi;


  // DIS kinematics
  TH2D * WVsRun = new TH2D("WVsRun",
    "W distribution (w/o W cut) vs. runnum;runnum;W",
    runnumBins,runnumMin,runnumMax,NBINS,0,6);
  TH2D * XVsRun = new TH2D("XVsRun",
    "x distribution vs. runnum;runnum;x",
    runnumBins,runnumMin,runnumMax,NBINS,0,1);
  TH2D * YVsRun = new TH2D("YVsRun",
    "y distribution (w/o y cut) vs. runnum;runnum;y",
    runnumBins,runnumMin,runnumMax,NBINS,0,1);
  //
  TH2D * WVsEPhi = new TH2D("WVsEPhi",
    "W distribution (w/o W cut) vs. e^{-} #phi;e^{-} #phi;W",
    NPHIBINS,-PI,PI,NBINS,0,6);
  TH2D * XVsEPhi = new TH2D("XVsEPhi",
    "x distribution vs. e^{-} #phi;e^{-} #phi;x",
    NPHIBINS,-PI,PI,NBINS,0,1);
  TH2D * YVsEPhi = new TH2D("YVsEPhi",
    "y distribution (w/o y cut) vs. e^{-} #phi;e^{-} #phi;y",
    NPHIBINS,-PI,PI,NBINS,0,1);

  // electron kinematics
  TH2D * eleEVsRun = new TH2D("eleEVsRun",
    "e^{-} E distribution vs. runnum;runnum;E",
    runnumBins,runnumMin,runnumMax,NBINS,0,12);
  TH2D * elePtVsRun = new TH2D("elePtVsRun",
    "e^{-} p_{T} distribution vs. runnum;runnum;p_{T}",
    runnumBins,runnumMin,runnumMax,NBINS,0,4);
  TH2D * eleEtaVsRun = new TH2D("eleEtaVsRun",
    "e^{-} #eta distribution vs. runnum;runnum;#eta",
    runnumBins,runnumMin,runnumMax,NBINS,-3,6);
  TH2D * elePhiVsRun = new TH2D("elePhiVsRun",
    "e^{-} #phi distribution vs. runnum;runnum;#phi",
    runnumBins,runnumMin,runnumMax,NBINS,-PIe,PIe);
  //
  TH2D * eleEVsEPhi = new TH2D("eleEVsEPhi",
    "e^{-} E distribution vs. e^{-} #phi;e^{-} #phi;E",
    NPHIBINS,-PI,PI,NBINS,0,12);
  TH2D * elePtVsEPhi = new TH2D("elePtVsEPhi",
    "e^{-} p_{T} distribution vs. e^{-} #phi;e^{-} #phi;p_{T}",
    NPHIBINS,-PI,PI,NBINS,0,4);
  TH2D * eleEtaVsEPhi = new TH2D("eleEtaVsEPhi",
    "e^{-} #eta distribution vs. e^{-} #phi;e^{-} #phi;#eta",
    NPHIBINS,-PI,PI,NBINS,-3,6);


  // dihadron's hadron kinematics
  TH2D * hadEVsRun[2];
  TH2D * hadPVsRun[2];
  TH2D * hadPtVsRun[2];
  TH2D * hadEtaVsRun[2];
  TH2D * hadPhiVsRun[2];
  TH2D * hadZVsRun[2];
  //
  TH2D * hadEVsPhPhi[2];
  TH2D * hadPVsPhPhi[2];
  TH2D * hadPtVsPhPhi[2];
  TH2D * hadEtaVsPhPhi[2];
  TH2D * hadPhiVsPhPhi[2];
  TH2D * hadZVsPhPhi[2];
  //
  for(int h=0; h<2; h++) {
    hadEVsRun[h] = new TH2D(TString(hadName[h]+"hadEVsRun"),
        TString(hadTitle[h]+" E vs. runnum;runnum;E"),
        runnumBins,runnumMin,runnumMax,NBINS,0,10);
    hadPVsRun[h] = new TH2D(TString(hadName[h]+"hadPVsRun"),
        TString(hadTitle[h]+" p vs. runnum;runnum;p"),
        runnumBins,runnumMin,runnumMax,NBINS,0,10);
    hadPtVsRun[h] = new TH2D(TString(hadName[h]+"hadPtVsRun"),
        TString(hadTitle[h]+" p_{T} vs. runnum;runnum;p_{T}"),
        runnumBins,runnumMin,runnumMax,NBINS,0,4);
    hadEtaVsRun[h] = new TH2D(TString(hadName[h]+"hadEtaVsRun"),
        TString(hadTitle[h]+" #eta vs. runnum;runnum;#eta"),
        runnumBins,runnumMin,runnumMax,NBINS,0,5);
    hadPhiVsRun[h] = new TH2D(TString(hadName[h]+"hadPhiVsRun"),
        TString(hadTitle[h]+" #phi vs. runnum;runnum;#phi"),
        runnumBins,runnumMin,runnumMax,NBINS,-PIe,PIe);
    hadZVsRun[h] = new TH2D(TString(hadName[h]+"hadZVsRun"),
        TString(hadTitle[h]+" z vs. runnum;runnum;z"),
        runnumBins,runnumMin,runnumMax,NBINS,0,1);
    //
    hadEVsPhPhi[h] = new TH2D(TString(hadName[h]+"hadEVsPhPhi"),
        TString(hadTitle[h]+" E vs. dihadron #phi;dihadron #phi;E"),
        NPHIBINS,-PI,PI,NBINS,0,10);
    hadPVsPhPhi[h] = new TH2D(TString(hadName[h]+"hadPVsPhPhi"),
        TString(hadTitle[h]+" p vs. dihadron #phi;dihadron #phi;p"),
        NPHIBINS,-PI,PI,NBINS,0,10);
    hadPtVsPhPhi[h] = new TH2D(TString(hadName[h]+"hadPtVsPhPhi"),
        TString(hadTitle[h]+" p_{T} vs. dihadron #phi;dihadron #phi;p_{T}"),
        NPHIBINS,-PI,PI,NBINS,0,4);
    hadEtaVsPhPhi[h] = new TH2D(TString(hadName[h]+"hadEtaVsPhPhi"),
        TString(hadTitle[h]+" #eta vs. dihadron #phi;dihadron #phi;#eta"),
        NPHIBINS,-PI,PI,NBINS,0,5);
    hadPhiVsPhPhi[h] = new TH2D(TString(hadName[h]+"hadPhiVsPhPhi"),
        TString(hadTitle[h]+" #phi vs. dihadron #phi;dihadron #phi;#phi"),
        NPHIBINS,-PI,PI,NBINS,-PIe,PIe);
    hadZVsPhPhi[h] = new TH2D(TString(hadName[h]+"hadZVsPhPhi"),
        TString(hadTitle[h]+" z vs. dihadron #phi;dihadron #phi;z"),
        NPHIBINS,-PI,PI,NBINS,0,1);
  };


  // dihadron kinematics
  TString plotTitle = "#Delta#phi = #phi(" + hadTitle[qA] + ")" +
    " - #phi(" + hadTitle[qB] + 
    ") distribution vs runnum;runnum;#Delta#phi";
  TH2D * deltaPhiVsRun = new TH2D("deltaPhiVsRun",
    plotTitle,
    runnumBins,runnumMin,runnumMax,NBINS,-PIe,PIe);

  TH2D * MhVsRun = new TH2D("MhVsRun",
    "M_{h} distribution vs. runnum;runnum;M_{h}",
    runnumBins,runnumMin,runnumMax,2*NBINS,0,3);
  TH2D * ZpairVsRun = new TH2D("ZpairVsRun",
    "z_{pair} distribution vs. runnum;runnum;z_{pair}",
    runnumBins,runnumMin,runnumMax,NBINS,0,1);
  TH2D * zetaVsRun = new TH2D("zetaVsRun",
    "#zeta distribution vs. runnum;runnum;#zeta",
    runnumBins,runnumMin,runnumMax,NBINS,-1,1);
  TH2D * xFVsRun = new TH2D("xFVsRun",
    "x_{F} distribution vs. runnum;runnum;x_{F}",
    runnumBins,runnumMin,runnumMax,NBINS,-2,2);
  TH2D * MmissVsRun = new TH2D("MmissVsRun",
    "M_{X} distribution vs. runnum;runnum;M_{X}",
    runnumBins,runnumMin,runnumMax,NBINS,-2,6);
  TH2D * thetaVsRun = new TH2D("thetaVsRun",
    "#theta distribution vs. runnum;runnum;#theta",
    runnumBins,runnumMin,runnumMax,NBINS,0,PI);
  //
  TH2D * PhiHVsRun = new TH2D("PhiHVsRun",
    "#phi_{h} distribution vs. runnum;runnum;#phi_{h}",
    runnumBins,runnumMin,runnumMax,NBINS,-PIe,PIe);
  TH2D * PhiRVsRun = new TH2D("PhiRVsRun",
    "#phi_{R} distribution vs. runnum;runnum;#phi_{R}",
    runnumBins,runnumMin,runnumMax,NBINS,-PIe,PIe);
  TH2D * PhiHRVsRun = new TH2D("PhiHRVsRun",
    "#phi_{h}-#phi_{R} distribution vs. runnum;runnum;#phi_{h}-#phi_{R}",
    runnumBins,runnumMin,runnumMax,NBINS,-PIe,PIe);
  ////
  TH2D * MhVsPhPhi = new TH2D("MhVsPhPhi",
    "M_{h} distribution vs. dihadron #phi;dihadron #phi;M_{h}",
    NPHIBINS,-PI,PI,2*NBINS,0,3);
  TH2D * alphaVsPhPhi = new TH2D("alphVsPhPhi",
    "#alpha distribution vs. dihadron #phi;dihadron #phi;#alpha",
    NPHIBINS,-PI,PI,NBINS,0,1.3);
  TH2D * ZpairVsPhPhi = new TH2D("ZpairVsPhPhi",
    "z_{pair} distribution vs. dihadron #phi;dihadron #phi;z_{pair}",
    NPHIBINS,-PI,PI,NBINS,0,1);
  TH2D * zetaVsPhPhi = new TH2D("zetaVsPhPhi",
    "#zeta distribution vs. dihadron #phi;dihadron #phi;#zeta",
    NPHIBINS,-PI,PI,NBINS,-1,1);
  TH2D * xFVsPhPhi = new TH2D("xFVsPhPhi",
    "x_{F} distribution vs. dihadron #phi;dihadron #phi;x_{F}",
    NPHIBINS,-PI,PI,NBINS,-2,2);
  TH2D * MmissVsPhPhi = new TH2D("MmissVsPhPhi",
    "M_{X} distribution vs. dihadron #phi;dihadron #phi;M_{X}",
    NPHIBINS,-PI,PI,NBINS,-2,6);
  TH2D * thetaVsPhPhi = new TH2D("thetaVsPhPhi",
    "#theta distribution vs. dihadron #phi;dihadron #phi;#theta",
    NPHIBINS,-PI,PI,NBINS,0,PI);
  //
  TH2D * PhiHVsPhPhi = new TH2D("PhiHVsPhPhi",
    "#phi_{h} distribution vs. dihadron #phi;dihadron #phi;#phi_{h}",
    NPHIBINS,-PI,PI,NBINS,-PIe,PIe);
  TH2D * PhiRVsPhPhi = new TH2D("PhiRVsPhPhi",
    "#phi_{R} distribution vs. dihadron #phi;dihadron #phi;#phi_{R}",
    NPHIBINS,-PI,PI,NBINS,-PIe,PIe);
  TH2D * PhiHRVsPhPhi = new TH2D("PhiHRVsPhPhi",
    "#phi_{h}-#phi_{R} distribution vs. dihadron #phi;dihadron #phi;#phi_{h}-#phi_{R}",
    NPHIBINS,-PI,PI,NBINS,-PIe,PIe);


  // multiplicities
  /*
  TH2D * partMultiplicityVsRun = new TH2D("partMultiplicityVsRun",
    "overall particle multiplicities (DIS cuts only) vs. runnum;runnum;particle",
    runnumBins,runnumMin,runnumMax,nParticles,0,nParticles);
  TH2D * obsMultiplicityVsRun = new TH2D("obsMultiplicityVsRun",
    "dihadrons' particle multiplicities vs. runnum;runnum;particle",
    runnumBins,runnumMin,runnumMax,nObservables,0,nObservables);
  //
  TH2D * partMultiplicityVsEPhi = new TH2D("partMultiplicityVsEPhi",
    "overall particle multiplicities (DIS cuts only) vs. e^{-} #phi;e^{-} #phi;particle",
    NPHIBINS,-PI,PI,nParticles,0,nParticles);
  TH2D * obsMultiplicityVsEPhi = new TH2D("obsMultiplicityVsEPhi",
    "dihadrons' particle multiplicities vs. e^{-} #phi;e^{-} #phi;particle",
    NPHIBINS,-PI,PI,nObservables,0,nObservables);

  for(int p=0; p<nParticles; p++) {
    partMultiplicityVsRun->GetYaxis()->SetBinLabel(p+1,PartTitle(p));
    partMultiplicityVsEPhi->GetYaxis()->SetBinLabel(p+1,PartTitle(p));
  };
  for(int p=0; p<nObservables; p++) {
    obsMultiplicityVsRun->GetYaxis()->SetBinLabel(p+1,ObsTitle(p));
    obsMultiplicityVsEPhi->GetYaxis()->SetBinLabel(p+1,ObsTitle(p));
  };
  */


  // event-level distributions
  TH2D * helicityVsRun = new TH2D("helicityVsRun",
    "helicity distribution vs. runnum;runnum;h",
    runnumBins,runnumMin,runnumMax,5,-2,3);
  TH2D * helicityVsEPhi = new TH2D("helicityVsEPhi",
    "helicity distribution vs. e^{-} #phi;e^{-} #phi;h",
    NPHIBINS,-PI,PI,5,-2,3);




  // EVENT LOOP
  printf("begin loop through %lld events...\n",ev->ENT);
  Int_t hadI[2];
  for(int i=0; i<ev->ENT; i++) {

    ev->GetEvent(i);


    /*
    // fill multiplicity plots
    //------------------------
    // fill overall particle multiplicity
    if(ev->cutDIS) {
      for(int p=0; p<nParticles; p++) {
        if(ev->particleCnt[p]>0) {
          partMultiplicityVsRun->Fill(ev->runnum,p,ev->particleCnt[p]);
          partMultiplicityVsEPhi->Fill(ev->elePhi,p,ev->particleCnt[p]);
        };
      };
    };
    if(ev->cutDIS && ev->cutDihadronKinematics && ev->cutDiph[qA] && ev->cutDiph[qB]) {

      // fill observable multiplicity
      for(int h=0; h<2; h++) {
        hadI[h] = IO(ev->hadIdx[h]); // observable indices
        obsMultiplicityVsRun->Fill(ev->runnum,hadI[h]);
        obsMultiplicityVsEPhi->Fill(ev->elePhi,hadI[h]);
      };
    };
    */


    // fill DIS kinematic plots
    // ------------------------
    if(ev->cutDihadron) {

      //ev->PrintEvent();

      if(ev->cutQ2 && ev->cutY) {
        WVsRun->Fill(ev->runnum,ev->W);
        WVsEPhi->Fill(ev->elePhi,ev->W);
      };

      if(ev->cutQ2 && ev->cutW) {
        YVsRun->Fill(ev->runnum,ev->y);
        YVsEPhi->Fill(ev->elePhi,ev->y);
      };
    };


    // fill dihadron kinematics plots
    // ------------------------------
    if(ev->Valid()) {

      eleEVsRun->Fill(ev->runnum,ev->eleE);
      elePtVsRun->Fill(ev->runnum,ev->elePt);
      eleEtaVsRun->Fill(ev->runnum,ev->eleEta);
      elePhiVsRun->Fill(ev->runnum,ev->elePhi);
      //
      eleEVsEPhi->Fill(ev->elePhi,ev->eleE);
      elePtVsEPhi->Fill(ev->elePhi,ev->elePt);
      eleEtaVsEPhi->Fill(ev->elePhi,ev->eleEta);

      XVsRun->Fill(ev->runnum,ev->x);
      //
      XVsEPhi->Fill(ev->elePhi,ev->x);

      for(int h=0; h<2; h++) {
        hadEVsRun[h]->Fill(ev->runnum,ev->hadE[h]);
        hadPVsRun[h]->Fill(ev->runnum,ev->hadP[h]);
        hadPtVsRun[h]->Fill(ev->runnum,ev->hadPt[h]);
        hadEtaVsRun[h]->Fill(ev->runnum,ev->hadEta[h]);
        hadPhiVsRun[h]->Fill(ev->runnum,ev->hadPhi[h]);
        hadZVsRun[h]->Fill(ev->runnum,ev->Z[h]);
        //
        hadEVsPhPhi[h]->Fill(ev->PhPhi,ev->hadE[h]);
        hadPVsPhPhi[h]->Fill(ev->PhPhi,ev->hadP[h]);
        hadPtVsPhPhi[h]->Fill(ev->PhPhi,ev->hadPt[h]);
        hadEtaVsPhPhi[h]->Fill(ev->PhPhi,ev->hadEta[h]);
        hadPhiVsPhPhi[h]->Fill(ev->PhPhi,ev->hadPhi[h]);
        hadZVsPhPhi[h]->Fill(ev->PhPhi,ev->Z[h]);
      };

      deltaPhi = Tools::AdjAngle(ev->hadPhi[qA] - ev->hadPhi[qB]);
      deltaPhiVsRun->Fill(ev->runnum,deltaPhi);

      MhVsRun->Fill(ev->runnum,ev->Mh);
      ZpairVsRun->Fill(ev->runnum,ev->Zpair);
      zetaVsRun->Fill(ev->runnum,ev->zeta);
      xFVsRun->Fill(ev->runnum,ev->xF);
      MmissVsRun->Fill(ev->runnum,ev->Mmiss);
      thetaVsRun->Fill(ev->runnum,ev->theta);
      //
      MhVsPhPhi->Fill(ev->PhPhi,ev->Mh);
      alphaVsPhPhi->Fill(ev->PhPhi,ev->alpha);
      ZpairVsPhPhi->Fill(ev->PhPhi,ev->Zpair);
      zetaVsPhPhi->Fill(ev->PhPhi,ev->zeta);
      xFVsPhPhi->Fill(ev->PhPhi,ev->xF);
      MmissVsPhPhi->Fill(ev->PhPhi,ev->Mmiss);
      thetaVsPhPhi->Fill(ev->PhPhi,ev->theta);

      PhiHVsRun->Fill(ev->runnum,ev->PhiH);
      PhiRVsRun->Fill(ev->runnum,ev->PhiR);
      PhiHRVsRun->Fill(ev->runnum,ev->PhiHR);
      //
      PhiHVsPhPhi->Fill(ev->PhPhi,ev->PhiH);
      PhiRVsPhPhi->Fill(ev->PhPhi,ev->PhiR);
      PhiHRVsPhPhi->Fill(ev->PhPhi,ev->PhiHR);


      helicityVsRun->Fill(ev->runnum,ev->helicity);
      helicityVsEPhi->Fill(ev->elePhi,ev->helicity);
    };

  }; // eo event loop


  // normalize VsRun distributions
  outfile->mkdir("RunByRun");
  normalize(WVsRun);
  normalize(XVsRun);
  normalize(YVsRun);

  normalize(eleEVsRun);
  normalize(elePtVsRun);
  normalize(eleEtaVsRun);
  normalize(elePhiVsRun);

  /*
  normalize(partMultiplicityVsRun);
  normalize(obsMultiplicityVsRun);
  */

  for(int h=0; h<2; h++) {
    normalize(hadEVsRun[h]);
    normalize(hadPVsRun[h]);
    normalize(hadPtVsRun[h]);
    normalize(hadEtaVsRun[h]);
    normalize(hadPhiVsRun[h]);
    normalize(hadZVsRun[h]);
  };

  normalize(deltaPhiVsRun);
  normalize(MhVsRun);
  normalize(ZpairVsRun);
  normalize(zetaVsRun);
  normalize(xFVsRun);
  normalize(MmissVsRun);
  normalize(thetaVsRun);

  normalize(PhiHVsRun);
  normalize(PhiRVsRun);
  normalize(PhiHRVsRun);

  normalize(helicityVsRun);
  

  // write output
  outfile->cd("/");
  outfile->mkdir("vsRun");
  outfile->cd("vsRun");
  WVsRun->Write();
  XVsRun->Write();
  YVsRun->Write();

  eleEVsRun->Write();
  elePtVsRun->Write();
  eleEtaVsRun->Write();
  elePhiVsRun->Write();

  /*
  partMultiplicityVsRun->Write();
  obsMultiplicityVsRun->Write();
  */

  for(int h=0; h<2; h++) hadEVsRun[h]->Write();
  for(int h=0; h<2; h++) hadPVsRun[h]->Write();
  for(int h=0; h<2; h++) hadPtVsRun[h]->Write();
  for(int h=0; h<2; h++) hadEtaVsRun[h]->Write();
  for(int h=0; h<2; h++) hadPhiVsRun[h]->Write();
  for(int h=0; h<2; h++) hadZVsRun[h]->Write();

  deltaPhiVsRun->Write();
  MhVsRun->Write();
  ZpairVsRun->Write();
  zetaVsRun->Write();
  xFVsRun->Write();
  MmissVsRun->Write();
  thetaVsRun->Write();

  PhiHVsRun->Write();
  PhiRVsRun->Write();
  PhiHRVsRun->Write();

  helicityVsRun->Write();


  outfile->cd("/");
  outfile->mkdir("vsPhi");
  outfile->cd("vsPhi");

  WVsEPhi->Write();
  XVsEPhi->Write();
  YVsEPhi->Write();

  eleEVsEPhi->Write();
  elePtVsEPhi->Write();
  eleEtaVsEPhi->Write();

  /*
  partMultiplicityVsEPhi->Write();
  obsMultiplicityVsEPhi->Write();
  */

  for(int h=0; h<2; h++) hadEVsPhPhi[h]->Write();
  for(int h=0; h<2; h++) hadPVsPhPhi[h]->Write();
  for(int h=0; h<2; h++) hadPtVsPhPhi[h]->Write();
  for(int h=0; h<2; h++) hadEtaVsPhPhi[h]->Write();
  for(int h=0; h<2; h++) hadPhiVsPhPhi[h]->Write();
  for(int h=0; h<2; h++) hadZVsPhPhi[h]->Write();

  MhVsPhPhi->Write();
  alphaVsPhPhi->Write();
  ZpairVsPhPhi->Write();
  zetaVsPhPhi->Write();
  xFVsPhPhi->Write();
  MmissVsPhPhi->Write();
  thetaVsPhPhi->Write();

  PhiHVsPhPhi->Write();
  PhiRVsPhPhi->Write();
  PhiHRVsPhPhi->Write();

  helicityVsEPhi->Write();


  outfile->Close();
};


void normalize(TH2D * d) {
  
  outfile->cd("/");
  //outfile->cd("RunByRun");

  TString dirname,fulldirname;
  Bool_t writePr;
  dirname = d->GetName();
  dirname(TRegexp("Vs.*$")) = "";
  fulldirname = "RunByRun/"+dirname;
  outfile->mkdir(fulldirname);
  outfile->cd(fulldirname);

  printf("normalize %s\n",d->GetName());
  TH2D * c = (TH2D*)d->Clone();
  d->Reset();

  Int_t nx = c->GetNbinsX();
  Int_t ny = c->GetNbinsY();

  Double_t yl = c->GetYaxis()->GetXmin();
  Double_t yh = c->GetYaxis()->GetXmax();

  TString prN,prT,runnumStr;
  //TH1D * pr = new TH1D("pr","pr",ny,yl,yh);

  Double_t bc,norm;
  for(int bx=1; bx<=nx; bx++) {

    runnumStr = Form("run%d",(Int_t)d->GetXaxis()->GetBinLowEdge(bx));
    
    //pr->Reset();
    prN = d->GetName();
    prT = d->GetTitle();
    prN(TRegexp("Vs.*$")) = TString("_"+runnumStr);
    prT(TRegexp("vs\\..*$")) = TString("-- "+runnumStr);
    //printf("prN=%s\n",prN.Data());
    //printf("prT=%s\n",prT.Data());
    pr = new TH1D(prN,prT,ny,yl,yh);
    writePr = false;

    for(int by=1; by<=ny; by++) {
      bc = c->GetBinContent(bx,by);
      pr->SetBinContent(by,bc);
      if(bc>0) writePr = true;
    };
    if(writePr) pr->Write();

    norm = pr->Integral();
    if(norm>0) pr->Scale(1/norm);

    for(int by=1; by<=ny; by++) {
      bc = pr->GetBinContent(by);
      d->SetBinContent(bx,by,bc);
    };
  };

  //delete pr;
   
};
      

  
