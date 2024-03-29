// builds spinroot files, which contain the necessary data structures
// used by the asymmetry fitting code

#include <cstdlib>
#include <iostream>
#include <vector>
#include <map>

// ROOT
#include "TFile.h"
#include "TString.h"
#include "TRegexp.h"

// Dispin
#include "Constants.h"
#include "EventTree.h"
#include "Binning.h"
#include "Asymmetry.h"
#include "Modulation.h"
#include "InjectionModel.h"


// argument variables
TString inputData;
TString injectionFileN;
Int_t pairType;
Int_t nBins[3];
Int_t ivType;
Int_t oaTw,oaL,oaM;
Bool_t useWeighting;
Int_t gridDim;
Int_t whichHelicityMC;
Bool_t isMC;

// subroutines
void SetDefaultArgs();
int PrintUsage();

// global variables
Binning * BS;
Asymmetry * A;
EventTree * ev;
TFile * spinrootFile;
TFile * treeFile;
TFile * injectionFile;
InjectionModel *IM;


//////////////////////////////////////


int main(int argc, char** argv) {

  //gDebug = 2; // use to debug streaming problems
  
  // read options
  SetDefaultArgs();
  int opt;
  enum inputType_enum {iFile,iDir};
  Int_t inputType = -1;
  Int_t nd=0;
  while( (opt=getopt(argc,argv,"f:d:p:i:n:t:l:m:w|b|h:s|j:")) != -1 ) {
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
      case 't': /* one-amp fit modulation specifier Twist */
        oaTw = (Int_t) strtof(optarg,NULL);
        break;
      case 'l': /* one-amp fit modulation specifier L */
        oaL = (Int_t) strtof(optarg,NULL);
        break;
      case 'm': /* one-amp fit modulation specifier M */
        oaM = (Int_t) strtof(optarg,NULL);
        break;
      case 'w': /* enable weighting by PhPerp/Mh */
        useWeighting = true;
        break;
      case 'b': /* enable 2d asymmetry grids, instead of 1d default */
        gridDim = 2;
        break;
      case 'h': /* which helicityMC */
        whichHelicityMC = (Int_t) strtof(optarg,NULL);
        break;
      case 's': /* simulated (MC) data (see Asymmetry::ActivateTree()) */
        isMC = true;
        break;
      case 'j': /* asymmetry injection model file */
        injectionFileN = optarg;
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
  printf("enable PhPerp/Mh weighting: %s\n",useWeighting?"true":"false");
  Tools::PrintSeparator(40,"-");
  printf("one-amp fit modulation: |%d,%d>, twist-%d\n",oaL,oaM,oaTw);
  if(gridDim==1)
    printf("one-amp fit performed in 1D modulation space\n");
  else if(gridDim==2)
    printf("one-amp and multi-amp fits performed in 2D (PhiR,PhiH) space\n");
  Tools::PrintSeparator(40,"-");
  printf("whichHelicityMC = %d\n",whichHelicityMC);
  Tools::PrintSeparator(40,"=");


  // set binning scheme
  BS = new Binning();
  BS->SetOAnums(oaTw,oaL,oaM);
  BS->useWeighting = useWeighting;
  BS->gridDim = gridDim;
  Bool_t schemeSuccess = BS->SetScheme(pairType,ivType,nBins[0],nBins[1],nBins[2]);
  if(!schemeSuccess) {
    fprintf(stderr,"ERROR: Binning::SetScheme failed\n");
    return 0;
  };
  Bool_t singleBinMode = BS->GetNbinsTotal()==1;


  // instantiate EventTree 
  // (use 1 file if inputType==iFile, or all root files in inputData if inputType==iDir)
  ev = new EventTree(inputData+(inputType==iDir?"/*.root":""),pairType);


  // get modulation name for 1-amp fit
  Modulation * moduOA = new Modulation(oaTw,oaL,oaM);
  TString modN = moduOA->ModulationName();
  printf("--> 1-amp fit will be for %s modulation\n\n",modN.Data());


  // instantiate spinroot file
  TString spinrootFileN,treeFileN;
  if(inputType==iDir) {
    spinrootFileN = "spinroot/spin";
    spinrootFileN += "__" + PairName(pairType) + "_";
    for(int d=0; d<BS->dimensions; d++) spinrootFileN += "_" + BS->GetIVname(d);
    spinrootFileN += "__" + modN;
    spinrootFileN += ".root";
  } else if(inputType==iFile) {
    spinrootFileN = inputData;
    spinrootFileN(TRegexp("^.*/")) = "spinroot/spin.";
  };
  treeFileN = spinrootFileN;
  treeFileN(TRegexp("spinroot/spin")) = "spinroot/tree";
  printf("\nCREATING OUTPUT FILE = %s\n\n",spinrootFileN.Data());
  spinrootFile = new TFile(spinrootFileN,"RECREATE");


  // instantiate Asymmetry objects, and
  // build map of 3-digit bin number -> Asymmetry object
  std::map<Int_t, Asymmetry*> asymMap;
  for(Int_t bn : BS->binVec) {
    A = new Asymmetry(BS,bn);
    if(A->success) asymMap.insert(std::pair<Int_t, Asymmetry*>(bn,A));
    else return 0;
  };

  // define output tree in a separate file, if analysing a single bin
  if(singleBinMode) {
    printf("\nCREATING TREE FILE = %s\n\n",treeFileN.Data());
    treeFile = new TFile(treeFileN,"RECREATE");
    if(injectionFileN!="") {
      injectionFile = new TFile(injectionFileN,"READ");
      IM = (InjectionModel*) injectionFile->Get("IM");
      treeFile->cd();
    } else IM=nullptr;
    A->ActivateTree(isMC,IM); // tell the one and only A to fill its TTree
    spinrootFile->cd();
  };

        

  //-----------------------------------------------------
  // EVENT LOOP  
  //-----------------------------------------------------
  Bool_t eventAdded;
  Double_t evCount = 0;
  ev->whichHelicityMC = whichHelicityMC;

  printf("begin loop through %lld events...\n",ev->ENT);
  for(int i=0; i<ev->ENT; i++) {
    //if(i>1000000) break; // limiter

    ev->GetEvent(i);
    if(ev->Valid()) {

      // add event to Asymmetry, where Asymmetry::AddEvent() checks the bin,
      // and fills plots if it's the correct bin; if not, AddEvent() does nothing
      for(Int_t bn : BS->binVec) {
        A = asymMap.at(bn);
        eventAdded = A->AddEvent(ev);
        //if(eventAdded && A->debug) ev->PrintEvent();
        if(eventAdded) evCount+=1;
      };

    };
  }; // eo EVENT LOOP


  // write out to spinroot file
  spinrootFile->cd();
  BS->Write("BS");
  for(Int_t bn : BS->binVec) {
    A = asymMap.at(bn);
    A->StreamData(spinrootFile);
  };

  // write out to tree file
  if(singleBinMode) {
    treeFile->cd();
    A->tree->Write("tree");
    treeFile->Close();
    spinrootFile->cd();
  };


  // stream objects directly to spinroot file
  // - this doesn't work, unfortunately. The Asymmetry objects get written, and have a
  //   data size that indicates histograms/data structures are being written, however
  //   upon trying to access anything in the TFile, it immediately seg faults
  // - alternative implementation above writes out only pertinent members of Asymmetry
  //   instead of the full Asymmetry object
  /*
  TString Aname;
  spinrootFile->cd();
  BS->Write("BS");
  for(Int_t bn : BS->binVec) {
    A = asymMap.at(bn);
    Aname = "A" + A->binN;
    printf("write %s\n",Aname.Data());
    A->PrintSettings();
    //A->rfData->Write(Aname);
    A->Write(Aname);
  };
  */


  // close spinroot file and cleanup
  printf("\n%s written\n\n",spinrootFileN.Data());
  printf("evCount = %.0f\n",evCount);
  spinrootFile->Close();
  for(Int_t bn : BS->binVec) {
    A = asymMap.at(bn);
    delete A;
  };
  return 0;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


// set default arguments
void SetDefaultArgs() {
  inputData = "";
  pairType = EncodePairType(kPip,kPim);
  ivType = Binning::vM + 1;
  for(int d=0; d<3; d++) nBins[d] = -1;
  oaTw = 2;
  oaL = 1;
  oaM = 1;
  useWeighting = false;
  gridDim = 1;
  whichHelicityMC = 0;
  isMC = false;
  injectionFileN = "";
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
  printf(" -w\tenable PhPerp/Mh weighting (default is off)\n\n");

  printf(" -t, -l, -m   azimuthal modulation single-amplitude asymmetry linear fit\n");
  printf("   \texample: -t2 -l1 -m1 will fit the sin(phiH-phiR) modulation\n");
  printf("   \texample: -t3 -l1 -m1 will fit the sin(phiR) modulation\n");
  printf("   \tdefault = -t%d -l%d -m%d\n\n",oaTw,oaL,oaM);

  printf(" -b\tbin in 2D azimuthal space (for multi-amplitude chi2 minimization)\n\n");

  printf("extra options for MC usage (in single-bin mode):\n");
  printf(" -s\ttell Asymmetry::ActivateTree() this is MC data\n");
  printf(" -j\tinjection model file, for MC asymmetry injection studies\n");
  printf(" -h\tselect which helicityMC to use (DEPRECATED)\n");
  printf("\n");

  return 0;
};
