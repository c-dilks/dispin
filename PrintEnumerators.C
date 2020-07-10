R__LOAD_LIBRARY(DiSpin)

#include "Constants.h"

void PrintEnumerators(Bool_t printAllPairs=false) {
  printf("\n");

  // print particles
  printf("particles\n");
  printf("---------\n");
  for(int p=0; p<nParticles; p++) {
    printf("%d = %s  (pid=%d)\n",p,PartName(p).Data(),PartPID(p));
  };
  printf("\n");

  // print pair types
  printf("dihadron pair types\n");
  printf("-------------------\n");
  Int_t pa,pb;
  TString outf = "pairs.list";
  TString outft = outf+".tmp";
  gSystem->RedirectOutput(outft,"w");
  for(int o1=0; o1<nObservables; o1++) {
    for(int o2=0; o2<nObservables; o2++) {
      if(IterPair(o1,o2,pa,pb) || printAllPairs) {
        printf("0x%x %s\n", EncodePairType(pa,pb), PairName(pa,pb).Data() );
      };
    };
  };
  gSystem->RedirectOutput(0);
  gROOT->ProcessLine(TString(".! sort " + outft + " | uniq > " + outf));
  gROOT->ProcessLine(TString(".! rm " + outft));
  gROOT->ProcessLine(TString(".! cat " + outf));


  printf("\nbasic dihadron cuts (useful for inspecting outroot files)\n");
  printf("---------------------------------------------------------\n");
  printf("pairType==0x34 && Q2>1 && W>2 && y<0.8 && xF>0 && Mmiss>1.5 && ");
  printf("Z[0]>0.1 && Z[1]>0.1 && hadP[0]>1.25 && hadP[1]>1.25\n");
};

