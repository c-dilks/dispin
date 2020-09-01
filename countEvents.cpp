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
EventTree * ev;
void PrintEvent();
Bool_t first;

int main(int argc, char** argv) {

   // ARGUMENTS
   infiles = "outroot/*.root";
   whichPair = EncodePairType(kPip,kPim);
   if(argc>1) infiles = TString(argv[1]);
   if(argc>2) whichPair = (Int_t)strtof(argv[2],NULL);

   // OPTIONS
   Bool_t printEvents = true;

   Long_t nTotal=0;
   Long_t nValid=0;
   Long_t nCutDIS=0;
   Long_t nCutDihadron=0;
   Long_t nCutHelicity=0;
   Long_t nCutFiducial=0;
   Long_t nCutPID=0;
   Long_t nCutVertex=0;
   
   ev = new EventTree(infiles,whichPair);
   first = true;

   // event loop
   for(int i=0; i<ev->ENT; i++) {
     ev->GetEvent(i);

     // full cut set
     if(ev->Valid()) {
       nValid++;
       if(printEvents && nValid<=10000) PrintEvent();
     };

     // counts for each cut
     if(Tools::PairSame(ev->hadIdx[qA],ev->hadIdx[qB],kPip,kPim)) {
       nTotal++;
       if(ev->cutDIS) nCutDIS++;
       if(ev->cutDihadron) nCutDihadron++;
       if(ev->cutHelicity) nCutHelicity++;
       if(ev->cutFiducial) nCutFiducial++;
       if(ev->cutPID) nCutPID++;
       if(ev->cutVertex) nCutVertex++;
     };

   };

   printf("total number of pi+pi- pairs = %ld\n",nTotal);
   printf("total number which satisfies all cuts = %ld\n",nValid);
   printf("nCutDIS = %ld\n",nCutDIS);
   printf("nCutDihadron = %ld\n",nCutDihadron);
   printf("nCutHelicity = %ld\n",nCutHelicity);
   printf("nCutFiducial = %ld\n",nCutFiducial);
   printf("nCutPID = %ld\n",nCutPID);
   printf("nCutVertex = %ld\n",nCutVertex);

   if(printEvents) 
     printf("\n!! events printed to eventTable.txt (no more than 10000 printed) !!\n\n");
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
