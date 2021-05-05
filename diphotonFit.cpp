#include <cstdlib>
#include <iostream>
#include <map>

// ROOT
#include "TFile.h"
#include "TString.h"
#include "TMath.h"
#include "TSystem.h"
#include "TROOT.h"
#include "TRegexp.h"
#include "TH1.h"
#include "TTree.h"

// dispin
#include <Binning.h>


using std::cout;
using std::cerr;
using std::endl;
using std::map;
using std::pair;

TString infileN, outfileN;
TFile *infile, *outfile;
TTree *tr;
Int_t ivType;
Int_t numBin[3];

Binning *BS;


// class for histograms, fit results, etc., for one bin of Binning scheme
class FitBin {
  public:
    TH1D *Mdist;

    // constructor
    FitBin(Int_t binn) {
      Mdist = new TH1D(
        Form("Mdist_%d",binn),Form("Mdist_%d",binn),
        100,0.04,0.45);
    };
      
};



////////////////////////////////////////
int main(int argc, char** argv) {

  // ARGUMENTS
  if(argc<=2) {
    cerr << "USAGE: " << argv[0] << " [tree file] [IV type] [num bins]" << endl;
    BS = new Binning();
    cout << " - IV types:" << endl
         << "    1, 2, or 3 digit number specifying binning scheme;" << endl
         << "    available vars:" << endl;
    for(int i=0; i<Binning::nIV; i++)
      cout << "     " << i+1 << " = " << BS->IVtitle[i] << endl;
    cout << " - num bins: number of bins, listed for each IV," << endl
         << "   separated by spaces (default is single bin)" << endl;
    return 1;
  };
  infileN = TString(argv[1]);
  ivType = (Int_t)strtof(argv[2],NULL);
  for(int i=0; i<3; i++) numBin[i]=-1;
  if(argc>3) {
    int ab=0;
    for(int ar=3; ar<argc; ar++)
      numBin[ab++] = (Int_t)strtof(argv[ar],NULL);
  };
  cout << "infileN = " << infileN << endl
       << "ivType = " << ivType << endl
       << "numBin = [" << numBin[0] << ","
                       << numBin[1] << ","
                       << numBin[2] << "]" << endl;


  // set binning scheme
  BS = new Binning();
  BS->SetScheme(ivType,numBin[0],numBin[1],numBin[2]);


  // open TFiles
  infile = new TFile(infileN,"READ");
  outfileN = infileN;
  outfileN(TRegexp("cat__")) = "fit__";
  outfile = new TFile(outfileN,"RECREATE");


  // open tree
  Double_t ivVal[3] = {0,0,0};
  Double_t Mgg;
  tr = (TTree*)infile->Get("diphTr");
  for(int d=0; d<BS->dimensions; d++) {
    cout << "IV " << d << " is " << BS->GetIVname(d) << endl;
    tr->SetBranchAddress(BS->GetIVname(d),&ivVal[d]);
  };
  tr->SetBranchAddress("diphM",&Mgg);


  // create FitBin object for each bin
  map<Int_t,FitBin*> FitBinList;
  for(Int_t bn : BS->binVec) {
    FitBinList.insert(pair<Int_t,FitBin*>(bn,new FitBin(bn)));
  };


  // loop through tree
  FitBin *fb;
  Long64_t ENT = tr->GetEntries();
  for(Long64_t e=0; e<ENT; e++) {
    if(e%10000==0) printf("[+] %.2f%%\n",100*(float)e/((float)ENT));
    tr->GetEntry(e);

    // find bin, and set FitBin pointer
    try {
      fb = FitBinList.at(BS->FindBin(ivVal[0],ivVal[1],ivVal[2]));
    } catch (const std::out_of_range &ex) {
      cerr << "ERROR: cannot find bin" << endl;
      continue;
    };

    // fill Mgg dist
    fb->Mdist->Fill(Mgg);
  };


  // write histograms
  outfile->cd();
  for(Int_t bn : BS->binVec) {
    FitBinList.at(bn)->Mdist->Write();
  };


  // cleanup
  outfile->Close();
  infile->Close(); 
  return 0;
};
