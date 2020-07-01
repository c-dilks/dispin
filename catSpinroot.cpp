// spinroot file concatenator
// - combines spinroot files into a single file, referred to the 'catFile'
// - the asymmetry fitting code will be called on the catFile

#include <cstdlib>
#include <iostream>
#include <vector>
#include <map>
//#include <mcheck.h> // (memory tracing)

// DihBsa
#include "Constants.h"
#include "Binning.h"
#include "Asymmetry.h"

// ROOT
#include "TFile.h"
#include "TString.h"
#include "TSystem.h"
#include "TRegexp.h"
#include "TSystemDirectory.h"
#include "TSystemFile.h"
#include "TList.h"
#include "TCollection.h"



int main(int argc, char** argv) {
  //mtrace(); // (memory tracing)

  // ARGUMENTS
  TString spinrootDir = "spinroot";
  if(argc>1) spinrootDir = TString(argv[1]);

  // instantiate catFile
  TFile * catFile = new TFile(TString(spinrootDir+"/cat.root"),"RECREATE");

  // define class objects which will be read/written from/to spinroot file
  Binning * BS = new Binning();
  Asymmetry * A;


  
  // get list of spinroot files
  TString spinrootFileName;
  std::vector<TString> spinrootFileList;
  TSystemDirectory * sysDir = new TSystemDirectory(spinrootDir,spinrootDir);
  TList * sysFileList = sysDir->GetListOfFiles();
  TIter nxt(sysFileList);
  TSystemFile * sysFile;
  while(( sysFile = (TSystemFile*) nxt() )) {
    spinrootFileName = spinrootDir + "/" + TString(sysFile->GetName());
    if( !sysFile->IsDirectory() && 
        spinrootFileName.EndsWith(".root") &&
        spinrootFileName.Contains("spin.") &&
        !spinrootFileName.Contains("/cat.root") ){
      spinrootFileList.push_back(spinrootFileName);
      printf("add %s\n",spinrootFileName.Data());
    };
  };

  // extract binning scheme from a spinroot file, 
  TFile * spinrootFileBS = new TFile(spinrootFileList.at(0),"READ");
  spinrootFileBS->GetObject("BS",BS);
  spinrootFileBS->Close();
  

  // instantiate catFile Asymmetry objects, following the extracted binning scheme
  std::map<Int_t, Asymmetry*> asymMap;
  for(Int_t bn : BS->binVec) {
    A = new Asymmetry(BS,bn);
    if(A->success) asymMap.insert(std::pair<Int_t, Asymmetry*>(bn,A));
    else return 0;
  };


  // concatenate spinroot files data into catFile
  // - the extracted binning scheme from above will be the one written to the catFile
  // - the Asymmetry objects are combined together using Asymmetry::AppendData, which
  //   hadds histograms and concatenates RooFit data structures
  TFile * spinrootFile;
  for(TString spinrootFileN : spinrootFileList) {
    Tools::PrintSeparator(40,".");
    printf("concatenating data from %s\n",spinrootFileN.Data());
    spinrootFile = new TFile(spinrootFileN,"READ");
    for(Int_t bn : BS->binVec) {
      A = asymMap.at(bn);
      A->AppendData(spinrootFile);
    };
    spinrootFile->Close("R");
    printf("done reading %s\n",spinrootFileN.Data());
  };
  Tools::PrintSeparator(40,".");
  printf("spinroot files concatenated\n\n");


  // write out to catFile
  catFile->cd();
  BS->Write("BS");
  for(Int_t bn : BS->binVec) {
    A = asymMap.at(bn);
    A->StreamData(catFile);
  };
  catFile->Close();
  for(Int_t bn : BS->binVec) {
    A = asymMap.at(bn);
    delete A;
  };

  //muntrace(); // (memory tracing)
};
