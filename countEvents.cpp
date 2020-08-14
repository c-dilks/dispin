#include <cstdlib>
#include <iostream>

// ROOT
#include "TFile.h"
#include "TTree.h"
#include "TChain.h"
#include "TString.h"

// DihBsa
#include "Constants.h"
#include "Tools.h"
#include "EventTree.h"

TString infiles;
Int_t whichPair;

int main(int argc, char** argv) {

   // ARGUMENTS
   infiles = "outroot/*.root";
   whichPair = EncodePairType(kPip,kPim);
   if(argc>1) infiles = TString(argv[1]);
   if(argc>2) whichPair = (Int_t)strtof(argv[2],NULL);

   Long_t nTotal=0;
   Long_t nValid=0;
   Long_t nCutDIS=0;
   Long_t nCutDihadron=0;
   Long_t nCutHelicity=0;
   Long_t nCutFiducial=0;
   Long_t nCutPID=0;
   Long_t nCutVertex=0;
   
   EventTree * ev = new EventTree(infiles,whichPair);
   for(int i=0; i<ev->ENT; i++) {
     ev->GetEvent(i);
     if(ev->pairType == 0x34) nTotal++;
     if(ev->Valid()) nValid++;
     if(ev->cutDIS) nCutDIS++;
     if(ev->cutDihadron) nCutDihadron++;
     if(ev->cutHelicity) nCutHelicity++;
     if(ev->cutFiducial) nCutFiducial++;
     if(ev->cutPID) nCutPID++;
     if(ev->cutVertex) nCutVertex++;
   };

   printf("total number of pi+pi- pairs = %ld\n",nTotal);
   printf("nValid = %ld\n",nValid);
   Long_t nCutDIS=0;
   Long_t nCutDihadron=0;
   Long_t nCutHelicity=0;
   Long_t nCutFiducial=0;
   Long_t nCutPID=0;
   Long_t nCutVertex=0;
};
