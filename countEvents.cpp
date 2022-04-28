#include <cstdlib>
#include <iostream>

// ROOT
#include "TFile.h"
#include "TTree.h"
#include "TChain.h"
#include "TString.h"
#include "TSystem.h"

// DihBsa
#include "Constants.h"
#include "Tools.h"
#include "EventTree.h"

TString infiles;
Int_t whichPair;
Int_t whichHad[2];
TString tableFile;
EventTree * ev;
void PrintCount(TString cntName,Long64_t numer,Long64_t denom);
void PrintEvent();
void PrintEvent2();
void PrintEvent_xcheck();
Bool_t first;
Bool_t hasDiphoton;

int main(int argc, char** argv) {

   /////////////////////
   // ARGUMENTS
   infiles = "outroot/*.root";
   whichPair = EncodePairType(kPip,kPim);
   tableFile = "eventTable.txt";
   if(argc>1) infiles = TString(argv[1]);
   if(argc>2) whichPair = (Int_t)strtof(argv[2],NULL);
   if(argc>3) tableFile = TString(argv[3]);
   DecodePairType(whichPair,whichHad[qA],whichHad[qB]);

   // OPTIONS
   Bool_t printEvents = true;
   Long64_t numToPrint = 1e9;
   /////////////////////


   // yields for each cut
   Long_t nTotal=0;
   Long_t nValid=0;
   Long_t nCutDIS=0;
   Long_t nCutDihadron=0;
   Long_t nCutHelicity=0;
   Long_t nCutFiducial=0;
   Long_t nCutPID=0;
   Long_t nCutVertex=0;
   Long_t nCutFR=0;

   // yields including diphoton cuts
   hasDiphoton = false; // true if whichPair has diphoton
   for(int h=0; h<2; h++) {
     if(  whichHad[h]==kPio
       || whichHad[h]==kDiph
       || whichHad[h]==kDiphBasic
       || whichHad[h]==kPioBG
     ) hasDiphoton = true;
   };
   Long64_t nCutdiphPhotBeta = 0;
   Long64_t nCutdiphPhotEn = 0;
   Long64_t nCutdiphPhotEleAng = 0;
   Long64_t nCutdiphPhotTheta = 0;
   Long64_t nCutdiphMassPi0 = 0;
   Long64_t nCutdiphMassSB = 0;
   Long64_t nCutdiphBasic = 0;

   // extra yield counters for cross checking
   const Int_t nTests = 4;
   Long_t nCutTest[nTests];
   for(int j=0; j<nTests; j++) nCutTest[j]=0;
   TString cutTestName[nTests];
   Bool_t cutTest[nTests];

   
   // open tree
   ev = new EventTree(infiles,whichPair);
   first = true;

   // event loop
   for(Long64_t i=0; i<ev->ENT; i++) {
     if(printEvents && nValid>=numToPrint) { printf("--- limiter ---\n"); break; };
     ev->GetEvent(i);

     //// in case we are looking at an older SISIS skim file, make sure the SIDIS skim file cut matches
     // if( ! ev->Check_nSidis_skim_cut() ) continue; // <--------------------!!!!!!!!!!! XCHECK (RGA ONLY)

     // full cut set
     if(ev->Valid()) {
       nValid++;
       if(printEvents && nValid<=numToPrint) PrintEvent_xcheck(); // <-------------------------------- !!!!!!!!!!!!!!!!!!!!! XCHECK
     };

     // counts for each cut
     if(Tools::PairSame(ev->hadIdx[qA],ev->hadIdx[qB],whichHad[qA],whichHad[qB])) {
       nTotal++;

       // main cuts
       if(ev->cutDIS) nCutDIS++;
       if(ev->cutDihadron) nCutDihadron++;
       if(ev->cutHelicity) nCutHelicity++;
       if(ev->cutFiducial) nCutFiducial++;
       if(ev->cutPID) nCutPID++;
       if(ev->cutVertex) nCutVertex++;
       if(ev->cutFR) nCutFR++;
       // diphoton cuts
       if(hasDiphoton) {
         if(ev->objDiphoton->cutPhotBeta) nCutdiphPhotBeta++;
         if(ev->objDiphoton->cutPhotEn) nCutdiphPhotEn++;
         if(ev->objDiphoton->cutPhotEleAng) nCutdiphPhotEleAng++;
         if(ev->objDiphoton->cutPhotTheta) nCutdiphPhotTheta++;
         if(ev->objDiphoton->cutMassPi0) nCutdiphMassPi0++;
         if(ev->objDiphoton->cutMassSB) nCutdiphMassSB++;
         if(ev->objDiphoton->cutBasic) nCutdiphBasic++;
       };

       ///////////////////////////////////////////////////////// XCHECK TESTS for PID
       // Bool_t cut1  = ev->eleTheta>5 && ev->eleTheta<35;
       // Bool_t cut2  = ev->eleP > 2;
       // Bool_t cut3  = ev->elePCALen > 0.07;
       // Bool_t cut4  = ev->CheckSampFrac_diagonal();
       // Bool_t cut5  = ev->CheckSampFrac_vs_p();
       // Bool_t cut6  = ev->hadTheta[qA]>5 && ev->hadTheta[qA]<35; // pi+
       // Bool_t cut7  = ev->hadP[qA] > 1.25;                       // pi+
       // Bool_t cut8  = ev->CheckHadChi2pid(qA);                   // pi+
       // Bool_t cut9  = ev->hadTheta[qB]>5 && ev->hadTheta[qB]<35; // pi-
       // Bool_t cut10 = ev->hadP[qB] > 1.25;                       // pi-
       // Bool_t cut11 = ev->CheckHadChi2pid(qB);                   // pi-
       // cutTest[0] = cut1 && cut6 && cut9;  cutTestName[0] = "cut1 && cut6 && cut9";
       // cutTest[1] = cut7 && cut10;         cutTestName[1] = "cut7 && cut10";
       // cutTest[2] = cut8 && cut11;         cutTestName[2] = "cut8 && cut11";
       // cutTest[3] = cut4 && cut5;          cutTestName[3] = "cut4 && cut5";
       // for(int j=0; j<nTests; j++) { if(cutTest[j]) nCutTest[j]++; };
       ///////////////////////////////////////////////////////// XCHECK TESTS

     };

   };

   printf("--------------------- RUN %d\n",ev->runnum);
   PrintCount("nAllCuts",nValid,nTotal);
   PrintCount("nNoCuts",nTotal,nTotal);
   PrintCount("nCutDIS",nCutDIS,nTotal);
   PrintCount("nCutDihadron",nCutDihadron,nTotal);
   PrintCount("nCutHelicity",nCutHelicity,nTotal);
   PrintCount("nCutFiducial",nCutFiducial,nTotal);
   PrintCount("nCutPID",nCutPID,nTotal);
   PrintCount("nCutVertex",nCutVertex,nTotal);
   PrintCount("nCutFR",nCutFR,nTotal);
   if(hasDiphoton) {
     printf("diphoton cuts:\n");
     PrintCount("  nCutdiphPhotBeta",nCutdiphPhotBeta,nTotal);
     PrintCount("  nCutdiphPhotEn",nCutdiphPhotEn,nTotal);
     PrintCount("  nCutdiphPhotEleAng",nCutdiphPhotEleAng,nTotal);
     PrintCount("  nCutdiphPhotTheta",nCutdiphPhotTheta,nTotal);
     PrintCount("  nCutdiphMassPi0",nCutdiphMassPi0,nTotal);
     PrintCount("  nCutdiphMassSB",nCutdiphMassSB,nTotal);
     PrintCount("  nCutdiphBasic",nCutdiphBasic,nTotal);
   };

   // for(int j=0; j<nTests; j++) PrintCount("nCutTest ("+cutTestName[j]+") = ",nCutTest[j],nTotal); // XCHECK

   if(printEvents) 
     printf("\n!! events printed to eventTable.txt (no more than 10000 printed) !!\n\n");
};


// print counts
void PrintCount(TString cntName,Long64_t numer,Long64_t denom) {
  //// print with percentage
  // printf("%s = %lld  (%.3f%%)\n",
  //     cntName.Data(),
  //     numer,
  //     100*(Double_t)numer/denom
  //     );
  //// print without percentage
  printf("%s   %lld\n",
      cntName.Data(),
      numer
      );
};


// print variables from the event
void PrintEvent() {
  if(first) {
    gSystem->RedirectOutput("eventTable.txt","w");
    printf("evnum");
    printf(" eleP");
    printf(" piPlusP");
    printf(" piMinusP");
    printf(" Q2");
    printf(" W");
    printf(" Mmiss");
    printf(" piPlusXF");
    printf(" piMinusXF");
    printf(" y");
    printf(" x");
    printf(" Zpair");
    printf(" Mh");
    printf(" PhPerp");
    printf(" PhiH");
    printf(" PhiR");
    printf(" theta");
    printf("\n");
    first = false;
  } else gSystem->RedirectOutput("eventTable.txt","a");
  printf("%d",ev->evnum);
  printf(" %.3f",ev->eleP);
  printf(" %.3f",ev->hadP[qA]);
  printf(" %.3f",ev->hadP[qB]);
  printf(" %.3f",ev->Q2);
  printf(" %.3f",ev->W);
  printf(" %.3f",ev->Mmiss);
  printf(" %.3f",ev->hadXF[qA]);
  printf(" %.3f",ev->hadXF[qB]);
  printf(" %.3f",ev->y);
  printf(" %.3f",ev->x);
  printf(" %.3f",ev->Zpair);
  printf(" %.3f",ev->Mh);
  printf(" %.3f",ev->PhPerp);
  printf(" %.3f",ev->PhiH);
  printf(" %.3f",ev->PhiR);
  printf(" %.3f",ev->theta);
  printf("\n");
  gSystem->RedirectOutput(0);
};


void PrintEvent2() {
  if(first) {
    TString pname[2];
    for(int h=0; h<2; h++) 
      pname[h] = PairHadName(whichHad[qA],whichHad[qB],h);
    gSystem->RedirectOutput("eventTable.txt","w");
    printf("evntnum");
    for(int h=0; h<2; h++) {
      printf(" Lab_%s_z",pname[h].Data());
      printf(" gN_%s_z",pname[h].Data());
      printf(" Breit_%s_z",pname[h].Data());
      printf(" gN_%s_rapidity",pname[h].Data());
      printf(" Breit_%s_rapidity",pname[h].Data());
      printf(" xF_%s",pname[h].Data());
    };
    printf("\n");
    first = false;
  } else gSystem->RedirectOutput("eventTable.txt","a");
  printf("%d",ev->evnum);
  for(int h=0; h<2; h++) {
    printf(" %.5f",ev->hadPqLab[h]);
    printf(" %.5f",ev->hadPqCom[h]);
    printf(" %.5f",ev->hadPqBreit[h]);
    printf(" %.5f",ev->hadYCM[h]);
    printf(" %.5f",ev->hadYH[h]);
    printf(" %.5f",ev->hadXF[h]);
  };
  printf("\n");
  gSystem->RedirectOutput(0);
};


void PrintEvent_xcheck() {
  if(first) {
    gSystem->RedirectOutput(tableFile,"w");
    first = false;
  } else gSystem->RedirectOutput(tableFile,"a");
  std::cout <<
    ev->runnum                       << " " <<
    ev->evnum                        << " " <<
    ev->helicity                     << " " <<
    ev->Q2                           << " " <<
    ev->W                            << " " <<
    ev->x                            << " " <<
    ev->y                            << " " <<
    ev->Zpair                        << " " <<
    ev->PhPerp                       << " " <<
    ev->hadXF[qA]                    << " " <<
    ev->hadXF[qB]                    << " " <<
    ev->Mmiss                        << " " <<
    ev->PhiH                         << " " <<
    ev->PhiR                         << " " <<
    ev->theta                        << " " <<
    ev->GetDepolarizationFactor('A') << " " <<
    ev->GetDepolarizationFactor('C') << " " <<
    ev->GetDepolarizationFactor('W') <<
    std::endl;
  gSystem->RedirectOutput(0);
};
