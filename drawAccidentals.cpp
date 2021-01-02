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

// DihBsa
#include "Constants.h"
#include "Tools.h"
#include "EventTree.h"


int main(int argc, char** argv) {


  // ARGUMENTS
  Int_t species=1;
  TString infileN="outroot/*.root";
  if(argc>1) species = (Int_t)strtof(argv[1],NULL);
  if(argc>2) infileN = TString(argv[2]);
  
  EventTree * ev = new EventTree(infileN,0x34); // whichPair doesn't matter, see cutSpecies below

  TFile * outfile = new TFile("acc.root","RECREATE");
  TH2D * betaVsP[2];
  TString specStr[2];
  switch(species) { 
    case 1: // pi+h-
      specStr[0]="#pi^{+}"; specStr[1]="h^{-}"; break;
    case 2: // h+pi-
      specStr[0]="h^{+}"; specStr[1]="#pi^{-}"; break;
    case 3: // h+h-
      specStr[0]="h^{+}"; specStr[1]="h^{-}"; break;
    default: fprintf(stderr,"bad species\n"); return 0;
  };
  TString plotN,plotT;
  for(int h=0; h<2; h++) {
    plotN = Form("betaVsP%d",h);
    plotT = "e"+specStr[0]+specStr[1]+"X production, #beta("+specStr[h]+") vs. p("+specStr[h]+");p;#beta";
    betaVsP[h] = new TH2D(plotN,plotT,700,0,11,700,0,3);
  };


  Bool_t loc_Valid;
  Bool_t cutSpecies;
  Bool_t loc_cutFiducial;
  Bool_t loc_cutDihadron;
  Bool_t loc_cutPID;

  for(int i=0; i<ev->ENT; i++) {
    //if(i>30000) break; // limiter
    ev->GetEvent(i);

    // cut overrides
    switch(species) {
      case 1: // pi+h-
        cutSpecies = ev->hadIdx[qA]==kPip && PartCharge(ev->hadIdx[qB])<0;
        loc_cutFiducial = ev->eleFiduCut && ev->hadFiduCut[qA];
        loc_cutPID =
          ev->cutElePID && ev->cutHadPID[qA] &&
          ev->hadTheta[qB]>5 && ev->hadTheta[qB]<35 && ev->hadP[qB]>1.25 && TMath::Abs(ev->hadChi2pid[qB])<3;
        break;
      case 2: // h+pi-
        cutSpecies = PartCharge(ev->hadIdx[qA])>0 && ev->hadIdx[qB]==kPim;
        loc_cutFiducial = ev->eleFiduCut && ev->hadFiduCut[qB];
        loc_cutPID =
          ev->cutElePID && ev->cutHadPID[qB] &&
          ev->hadTheta[qA]>5 && ev->hadTheta[qA]<35 && ev->hadP[qA]>1.25 && TMath::Abs(ev->hadChi2pid[qA])<3;
        break;
      case 3: // h+h-
        cutSpecies = PartCharge(ev->hadIdx[qA])*PartCharge(ev->hadIdx[qB]) < 0;
        loc_cutFiducial = ev->eleFiduCut;
        loc_cutPID =
          ev->cutElePID &&
          ev->hadTheta[qA]>5 && ev->hadTheta[qA]<35 && ev->hadP[qA]>1.25 && TMath::Abs(ev->hadChi2pid[qA])<3 &&
          ev->hadTheta[qB]>5 && ev->hadTheta[qB]<35 && ev->hadP[qB]>1.25 && TMath::Abs(ev->hadChi2pid[qB])<3;
        break;
    };

    loc_cutDihadron = 
      cutSpecies &&
      ev->Zpair<0.95 &&
      ev->Mmiss>1.5 &&
      ev->hadXF[qA]>0 && ev->hadXF[qB]>0;
    

    loc_Valid = 
      ev->cutDIS &&
      loc_cutDihadron &&
      ev->cutHelicity &&
      loc_cutFiducial &&
      loc_cutPID &&
      ev->cutVertex;

    if(loc_Valid) {
      for(int h=0; h<2; h++) {
        betaVsP[h]->Fill(ev->hadP[h],ev->hadBeta[h]);
      };
    };
  };

  for(int h=0; h<2; h++) {
    betaVsP[h]->Write();
  };

  outfile->Close();
  return 1;
};
