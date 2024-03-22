/* step 1: build generated vs. reconstructed 2D histograms
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
    gROOT->ProcessLine(".! mkdir -p migration");
    if(inputType==iDir) {
      outFileN = "migration/hist";
      outFileN += "_" + PairName(pairType);
      for(int d=0; d<BS->dimensions; d++) outFileN += "_" + BS->GetIVname(d);
      outFileN += ".root";
    } else if(inputType==iFile) {
      outFileN = inputData;
      outFileN(TRegexp("^.*/")) = "migration/hist.";
    };
  }
  printf("\nCREATING TREE FILE = %s\n\n",outFileN.Data());
  outFile = new TFile(outFileN,"RECREATE");


  //-----------------------------------------------------
  // output data structures
  //-----------------------------------------------------


  // 2d hist: Generated Bin vs. Reconstructed Bin
  Int_t nBinsTotal = BS->GetNbinsTotal();
  TH2D *genVrec = new TH2D(
      "genVrec",
      "Generated Bin vs. Reconstructed Bin;Reconstructed Bin;Generated Bin",
      nBinsTotal,0,nBinsTotal,
      nBinsTotal,0,nBinsTotal
      );
  // label bins with 3-digit bin numbers from Binning
  Int_t b=1;
  for(Int_t bn : BS->binVec) {
    TString bStr = Form("%d",bn);
    genVrec->GetXaxis()->SetBinLabel(b,bStr.Data());
    genVrec->GetYaxis()->SetBinLabel(b,bStr.Data());
    b++;
  }

  // tree, for iv means
  TTree *ivTr = new TTree("ivTr","ivTr");
  Int_t binNumRec,binNumGen;
  Float_t ivVal[3] = {0.0,0.0,0.0};
  ivTr->Branch("binNumRec",&binNumRec,"binNumRec/I");
  ivTr->Branch("binNumGen",&binNumGen,"binNumGen/I");
  ivTr->Branch("ivVal",ivVal,"ivVal[3]/F");


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

      // find the reconstructed and generated bin
      binNumRec = BS->FindBin(ev);
      binNumGen = BS->FindBinGen(ev);
      if( binNumRec<0 || binNumGen<0 ) continue; // skip not-found bins (result of (rare) bad generated kinematics calculations)

      // fill output data structures
      TString binStrRec = Form("%d",binNumRec);
      TString binStrGen = Form("%d",binNumGen);
      genVrec->Fill(
          binStrRec.Data(),
          binStrGen.Data(),
          1.0
          );

      for(int d=0; d<BS->dimensions; d++) ivVal[d]=BS->GetIVval(d);
      ivTr->Fill();

    };
  }; // eo EVENT LOOP


  // write
  genVrec->Write();
  ivTr->Write();
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
