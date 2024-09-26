/* - make a tree with runnum, sampling fraction, sector, etc.,
 *   to help determine sampling fraction epochs
 */

#include <cstdlib>
#include <iostream>

// ROOT
#include "TFile.h"
#include "TString.h"
#include "TRegexp.h"
#include "TROOT.h"

// Dispin
#include "Constants.h"
#include "EventTree.h"

using std::cout;
using std::cerr;
using std::endl;

// argument variables
TString inputData;
TString outFileN;
Int_t pairType;
Long64_t limiter;

// subroutines
void SetDefaultArgs();
int PrintUsage();

// global variables
EventTree *ev;
TFile *outFile;


//////////////////////////////////////


int main(int argc, char** argv) {

  // read options
  SetDefaultArgs();
  int opt;
  enum inputType_enum {iFile,iDir};
  Int_t inputType = -1;
  Int_t nd=0;
  while( (opt=getopt(argc,argv,"f:d:o:p:l:")) != -1 ) {
    switch(opt) {
      case 'f': /* input file */
        if(inputType>=0) return PrintUsage();
        inputData = optarg;
        inputType = iFile;
        break;
      case 'd': /* input directory */
        if(inputType>=0) return PrintUsage();
        inputData = optarg;
        inputType = iDir;
        break;
      case 'o': /* output file name */
        outFileN = optarg;
        break;
      case 'p': /* pair type (hexadecimal number) */
        pairType = (Int_t) strtof(optarg,NULL);
        break;
      case 'l': /* limiter */
        limiter = (Long64_t) strtof(optarg,NULL);
        break;
      default: return PrintUsage();
    };
  };
  if(inputType!=iFile && inputType!=iDir) {
    fprintf(stderr,"ERROR: must specify input file or directory\n");
    return PrintUsage();
  };
  if(pairType==0x99) {
    fprintf(stderr,"ERROR: pi0 pi0 channel is under construction\n");
    return 0;
  };

  // print arguments' values
  Tools::PrintSeparator(40,"=");
  printf("inputData = %s\n",inputData.Data());
  printf("pairType = 0x%x\n",pairType);
  Tools::PrintSeparator(40,"=");

  // instantiate EventTree 
  // (use 1 file if inputType==iFile, or all root files in inputData if inputType==iDir)
  ev = new EventTree(inputData+(inputType==iDir?"/*.root":""),pairType);

  // define output file
  if(outFileN=="") { // set default
    if(inputType==iDir) outFileN="out.root";
    else {
      outFileN = inputData;
      outFileN(TRegexp("^.*/")) = "sfPart.";
    };
  };
  printf("\nCREATING TREE FILE = %s\n\n",outFileN.Data());
  outFile = new TFile(outFileN,"RECREATE");


  //-----------------------------------------------------
  // output data structures
  //-----------------------------------------------------

  TTree *tr = new TTree("tree","tree");
  Bool_t CheckSampFrac;
  tr->Branch( "runnum",        &(ev->runnum),      "runnum/I"        );
  tr->Branch( "sector",        &(ev->eleSector),   "sector/I"        );
  tr->Branch( "sampFrac",      &(ev->eleSampFrac), "sampFrac/F"      );
  tr->Branch( "eleP",          &(ev->eleP),        "eleP/F"          );
  tr->Branch( "CheckSampFrac", &CheckSampFrac,     "CheckSampFrac/O" );

  //-----------------------------------------------------
  // EVENT LOOP  
  //-----------------------------------------------------
  printf("begin loop through %lld events...\n",ev->ENT);
  for(int i=0; i<ev->ENT; i++) {
    if(limiter>0 && i>limiter) break;

    ev->GetEvent(i);
    
    // CUT
    if( 
        ev->cutDIS &&
        ev->eleFiduCut &&
        ev->eleTheta>5 && ev->eleTheta<35 &&
        ev->eleP>2 &&
        ev->elePCALen>0.07
      ) {
      CheckSampFrac = ev->CheckSampFrac();
      tr->Fill();
    };

  }; // eo EVENT LOOP


  // write out to spinroot file
  tr->Write();
  outFile->Close();
  printf("wrote %s\n",outFileN.Data());
  return 0;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


// set default arguments
void SetDefaultArgs() {
  inputData = "";
  outFileN = "";
  pairType = EncodePairType(kPip,kPim);
  limiter = 0;
};


// help printout
int PrintUsage() {

  SetDefaultArgs();
  fprintf(stderr,"\nUSAGE: buildSpinroot.exe [-f or -d input_data ] [options...]\n\n");

  printf("INPUT DATA:\n");
  printf(" -f\tsingle ROOT file\n");
  printf(" -d\tdirectory of ROOT files\n");
  printf(" NOTE: specify input with either -f or -d, but not both\n");
  printf("\n");

  printf("OPTIONS:\n");

  printf(" -o\toutput file name\n");
  printf("   \tdefault is a name based on the input file or directory name\n\n");

  printf(" -p\tpair type, specified as a hexadecimal number\n");
  printf("   \trun PrintEnumerators.C for notation\n");
  printf("   \tdefault = 0x%x (%s)\n\n",pairType,PairTitle(pairType).Data());

  printf(" -l\tlimiter, truncate to this many events (default take all)\n\n");

  return 0;
};
