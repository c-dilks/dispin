// reads spinroot/cat.root file and makes distributions of the 
// denominator, assuming D values, specified by argument

#include <cstdlib>
#include <iostream>
#include <map>

#include "Asymmetry.h"
#include "Binning.h"

///////////////////////////
const int nD = 5; // number of sigma_uu amps
bool writeHists = true; // make root file with hists
///////////////////////////


Asymmetry * A;
Binning * BS;

int main(int argc, char** argv) {

  // read arguments, which must be D values
  double dval[nD];
  if(argc==nD+1) {
    for(int d=0; d<nD; d++) {
      dval[d] = (double)strtof(argv[d+1],NULL);
      printf("d%d = %f\n",d,dval[d]);
    };
  } else {
    fprintf(stderr,"USAGE: denomDist.exe");
    for(int d=0; d<nD; d++) fprintf(stderr," d%d",d); fprintf(stderr,"\n");
    return 0;
  };


  // obtain Asymmetry objects, and prepare them
  TFile * infile = new TFile("spinroot/cat.root","READ");
  std::map<Int_t, Asymmetry*> asymMap;
  infile->GetObject("BS",BS);
  for(Int_t bn : BS->binVec) {
    A = new Asymmetry(BS,bn);
    if(A->success) {

      // import data
      A->AppendData(infile);

      // set D parameter values, and define denominator modulation
      for(int d=0; d<nD; d++) A->DparamVal[d]=dval[d];
      A->SetFitMode(200);

      // add to asymMap
      asymMap.insert(std::pair<Int_t,Asymmetry*>(bn,A));
    }
    else return 0;
  };


  // fill distributions
  Double_t denom;
  double phiH,phiR,theta;
  bool complained=false;
  for(Int_t bn : BS->binVec) {
    A = asymMap.at(bn);
    printf("read bin %d...\n",bn);
    for(int i=0; i<A->rfData->numEntries(); i++) {
      // get (phiR,phiH,theta) value
      phiR = A->rfData->get(i)->getRealValue("rfPhiR");
      phiH = A->rfData->get(i)->getRealValue("rfPhiH");
      theta = A->rfData->get(i)->getRealValue("rfTheta");

      // compute denominator
      denom = A->DenomEval(phiR,phiH,theta);

      // check if denominator is within bin boundaries (print warning if not)
      if(!complained) {
        if(denom<A->denomLB || denom>A->denomUB) {
          fprintf(stderr,"WARNING WARNING WARNING: denomDist overflow!\n");
          complained=true;
        };
      };

      A->denomDist->Fill(denom);
    };
  };

  // write distributions
  TFile * outfile;
  if(writeHists) {
    outfile = new TFile("dplots.root","RECREATE");
    for(Int_t bn : BS->binVec) {
      A = asymMap.at(bn);
      A->denomDist->Write();
      printf("bin%d: <D>=%f +- %f\n",bn,A->denomDist->GetMean(),A->denomDist->GetRMS());
    };
  };

  // close cat file
  infile->Close();
  outfile->Close();
};
