// builds ortho.root, which contains yield distributions needed for modulation
// orthogonalty studies (see Orthogonality*.C)

#include <cstdlib>
#include <iostream>
#include <vector>
#include <map>

// ROOT
#include "TFile.h"
#include "TString.h"
#include "TMath.h"
#include "TRegexp.h"
#include "TH1.h"
#include "TH2.h"
#include "TF1.h"
#include "TSystem.h"
#include "TObjArray.h"

// dispin
#include "Constants.h"
#include "Binning.h"
#include "EventTree.h"
#include "Tools.h"

TString inputData;
TString outputFileN;
Int_t pairType;
Int_t ivType;
Binning * BS;
EventTree * ev;


int PrintUsage();
void SetDefaultArgs();


int main(int argc, char** argv) {

  // OPTION
  Bool_t usePhiD = 0; // if true, replace theta with phiD in z-axis; must match Orthogonality.C

  // read options
  SetDefaultArgs();
  int opt;
  enum inputType_enum {iFile,iDir};
  Int_t inputType = -1;
  while( (opt=getopt(argc,argv,"f:d:o:p:i:")) != -1 ) {
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
        outputFileN = optarg;
        break;
      case 'p': /* pair type (hexadecimal number) */
        pairType = (Int_t) strtof(optarg,NULL);
        break;
      case 'i': /* independent variables */
        ivType = (Int_t) strtof(optarg,NULL);
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
  ev = new EventTree(inputData+(inputType==iDir?"/*.root":""),pairType);

  // print arguments' values
  Tools::PrintSeparator(40,"=");
  printf("inputData = %s\n",inputData.Data());
  printf("outputFileN = %s\n",outputFileN.Data());
  printf("pairType = 0x%x\n",pairType);
  printf("ivType = %d\n",ivType);

  // set binning scheme
  BS = new Binning();
  Bool_t schemeSuccess = BS->SetScheme(ivType);
  if(!schemeSuccess) {
    fprintf(stderr,"ERROR: Binning::SetScheme failed\n");
    return 0;
  };


  // set up output file and distributions
  TFile * outfile = new TFile(outputFileN,"RECREATE");

  // yield distributions
  const Int_t NBINS = 50;
  std::map<Int_t,TH2D*> d2;
  std::map<Int_t,TH3D*> d3;
  TString d2n,d3n;
  for(Int_t b : BS->binVec) {
    d2n = Form("d2_bin%d",b);
    d3n = Form("d3_bin%d",b);
    d2.insert(std::pair<Int_t,TH2D*>(b,
      new TH2D(d2n,d2n,NBINS,-PI,PI,NBINS,-PI,PI)
    ));
    d3.insert(std::pair<Int_t,TH3D*>(b,
      new TH3D(d3n,d3n,NBINS,-PI,PI,NBINS,-PI,PI,NBINS,usePhiD?-PI:0,PI)
    ));
  };


  // event loop
  Float_t pH,pR;
  Int_t bn;
  for(int i=0; i<ev->ENT; i++) {
    ev->GetEvent(i);
    if(ev->Valid()) {
      pH = Tools::AdjAngle(ev->PhiH); // shift to [-pi,+pi]
      pR = Tools::AdjAngle(ev->PhiR); // shift to [-pi,+pi]
      bn = BS->FindBin(ev);
      d2.at(bn)->Fill(pR,pH);
      if(usePhiD) d3.at(bn)->Fill(pR,pH,ev->PhiD);
      else        d3.at(bn)->Fill(pR,pH,ev->theta);
    };
  };

  // write
  for(Int_t b : BS->binVec) d2.at(b)->Write();
  for(Int_t b : BS->binVec) d3.at(b)->Write();
};

//////////////////////////////////////


// set default arguments
void SetDefaultArgs() {
  inputData = "";
  outputFileN = "ortho.root";
  pairType = EncodePairType(kPip,kPim);
  ivType = Binning::vM + 1;
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

  return 0;
};
