// check polarization LUT for RGB runs
R__LOAD_LIBRARY(DiSpin)
#include <Constants.h>
void polarizationLUTcheck(TString runlist="tmp/rgbRunlist.txt") {
  TTree * tr = new TTree();
  tr->ReadFile(runlist,"runnum/I");
  Int_t runnum;
  tr->SetBranchAddress("runnum",&runnum);
  for(Long64_t i=0; i<tr->GetEntries(); i++) {
    tr->GetEntry(i);
    cout << runnum << " " << RundepPolarization(runnum) << endl;
  };
};
