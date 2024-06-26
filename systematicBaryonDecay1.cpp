/* step 1: obtain parent information for all dihadrons
 * - for each bin: makes a TTree and a histogram:
 *   - histogram: used for bin means (can be 1D,2D,3D)
 */

#include <cstdlib>
#include <iostream>
#include <map>

// ROOT
#include "TFile.h"
#include "TString.h"
#include "TRegexp.h"
#include "TROOT.h"

// Dispin
#include "Constants.h"
#include "EventTree.h"
#include "Binning.h"

using std::cout;
using std::cerr;
using std::endl;
using std::map;
using std::pair;

// argument variables
TString inputData;
TString outFileN;
Int_t pairType;
Int_t nBins[3];
Int_t ivType;
Long64_t limiter;

// subroutines
void SetDefaultArgs();
int PrintUsage();

// global variables
Binning *BS;
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
  while( (opt=getopt(argc,argv,"f:d:o:p:i:n:l:")) != -1 ) {
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
      case 'i': /* independent variables */
        ivType = (Int_t) strtof(optarg,NULL);
        break;
      case 'n': /* number of bins for each independent variable */
        optind--;
        for( ; optind<argc && *argv[optind]!='-'; optind++) {
          if(nd<3) nBins[nd++] = (Int_t) strtof(argv[optind],NULL);
        };
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
  printf("ivType = %d\n",ivType);
  printf("nBins = ( %d, %d, %d )\n",nBins[0],nBins[1],nBins[2]);
  Tools::PrintSeparator(40,"=");

  // set binning scheme
  BS = new Binning();
  Bool_t schemeSuccess = BS->SetScheme(pairType,ivType,nBins[0],nBins[1],nBins[2]);
  if(!schemeSuccess) {
    fprintf(stderr,"ERROR: Binning::SetScheme failed\n");
    return 0;
  };

  // instantiate EventTree 
  // (use 1 file if inputType==iFile, or all root files in inputData if inputType==iDir)
  ev = new EventTree(inputData+(inputType==iDir?"/*.root":""),pairType);

  // define output file
  if(outFileN=="") {
    gROOT->ProcessLine(".! mkdir -p sysunc/baryonDecay");
    if(inputType==iDir) {
      outFileN = "sysunc/baryonDecay/tree";
      outFileN += "_" + PairName(pairType);
      for(int d=0; d<BS->dimensions; d++) outFileN += "_" + BS->GetIVname(d);
      outFileN += ".root";
    } else if(inputType==iFile) {
      outFileN = inputData;
      outFileN(TRegexp("^.*/")) = "sysunc/baryonDecay/tree.";
    };
  }
  printf("\nCREATING TREE FILE = %s\n\n",outFileN.Data());
  outFile = new TFile(outFileN,"RECREATE");


  //-----------------------------------------------------
  // output data structures
  //-----------------------------------------------------

  // build map of 3-digit bin number -> object
  map<Int_t,TTree*> treeHash;
  map<Int_t,TH1*> histHash;
  Float_t ivVal[3] = {-10000.0,-10000.0,-10000.0};
  for(Int_t bn : BS->binVec) {

    // tree; each tree will have the same branch addresses, but only
    // one of them will be filled per each dihadron
    TString treeN = Form("tree_%d",bn);
    treeHash.insert(pair<Int_t,TTree*>(bn,new TTree(treeN,treeN)));
    treeHash.at(bn)->Branch("evnum",&(ev->evnum),"evnum/I");
    for(int d=0; d<BS->dimensions; d++) treeHash.at(bn)->Branch(BS->GetIVname(d),&(ivVal[d]),BS->GetIVname(d)+"/F");
    treeHash.at(bn)->Branch("parentIdx",ev->gen_hadParentIdx,"parentIdx[2]/I");
    treeHash.at(bn)->Branch("parentPid",ev->gen_hadParentPid,"parentPid[2]/I");

    // histogram
    TString histN = Form("iv_%d",bn);
    TString histT = "iv dist";
    for(int d=0; d<BS->dimensions; d++) histT += ";" + BS->GetIVtitle(d);
    switch(BS->dimensions) {
      case 1:
        histHash.insert(pair<Int_t,TH1D*>( bn, new TH1D(histN, histT,
                500, BS->GetIVmin(0), BS->GetIVmax(0)
                )));
        break;
      case 2:
        histHash.insert(pair<Int_t,TH2D*>( bn, new TH2D(histN, histT,
                500, BS->GetIVmin(0), BS->GetIVmax(0),
                500, BS->GetIVmin(1), BS->GetIVmax(1)
                )));
        break;
      case 3:
        histHash.insert(pair<Int_t,TH3D*>( bn, new TH3D(histN, histT,
                500, BS->GetIVmin(0), BS->GetIVmax(0),
                500, BS->GetIVmin(1), BS->GetIVmax(1),
                500, BS->GetIVmin(2), BS->GetIVmax(2)
                )));
        break;
    };
  };

  //-----------------------------------------------------
  // EVENT LOOP  
  //-----------------------------------------------------
  Bool_t eventAdded;
  Double_t evCount = 0;
  Float_t deltaPhiDeg[2];
  Float_t deltaThetaDeg[2];

  printf("begin loop through %lld events...\n",ev->ENT);
  for(int i=0; i<ev->ENT; i++) {

    ev->GetEvent(i);
    if(ev->Valid()) {

      // truncate at limit
      if(limiter>0 && i>limiter) break;

      // MATCHING CUT
      for(int h=0; h<2; h++) {
        deltaPhiDeg[h] = Tools::AdjAngle( ev->hadPhi[h] - ev->gen_hadPhi[h] ) * TMath::RadToDeg();
        deltaThetaDeg[h] = Tools::AdjAngleDeg( ev->hadTheta[h] - ev->gen_hadTheta[h] );
      }
      if( deltaPhiDeg[qA]>3.0 || deltaThetaDeg[qA]>1.0 ||
          deltaPhiDeg[qB]>3.0 || deltaThetaDeg[qB]>1.0 )
        continue;

      // find the bin, and IV values
      Int_t binnum = BS->FindBin(ev);
      for(int d=0; d<BS->dimensions; d++) ivVal[d] = BS->GetIVval(d);

      // fill output data structures
      treeHash.at(binnum)->Fill();
      switch(BS->dimensions) {
        case 1: ((TH1D*)histHash.at(binnum))->Fill(ivVal[0]);                   break;
        case 2: ((TH2D*)histHash.at(binnum))->Fill(ivVal[0],ivVal[1]);          break;
        case 3: ((TH3D*)histHash.at(binnum))->Fill(ivVal[0],ivVal[1],ivVal[2]); break;
      };

    };
  }; // eo EVENT LOOP


  // write out to spinroot file
  for(Int_t bn : BS->binVec) treeHash.at(bn)->Write();
  for(Int_t bn : BS->binVec) histHash.at(bn)->Write();
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
  ivType = Binning::vM + 1;
  limiter = 0;
  for(int d=0; d<3; d++) nBins[d] = -1;
};


// help printout
int PrintUsage() {

  SetDefaultArgs();
  BS = new Binning();
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

  printf(" -i\tindependent variable specifier: 1, 2, or 3-digit number which\n");
  printf("   \tspecifies the independent variables that asymmetries will be\n");
  printf("   \tplotted against. The number of digits will be the number of\n");
  printf("   \tdimensions in the multi-dimensional binning\n");
  printf("   \t* the allowed digits are:\n");
  BS = new Binning();
  for(int i=0; i<Binning::nIV; i++) {
    printf("   \t  %d = %s\n",i+1,(BS->IVtitle[i]).Data());
  };
  printf("   \tdefault = %d\n\n",ivType);

  printf(" -n\tnumber of bins, listed for each independent variable,\n");
  printf("   \tseparated by spaces\n\n");

  printf(" -l\tlimiter, truncate to this many events (default take all)\n\n");

  return 0;
};
