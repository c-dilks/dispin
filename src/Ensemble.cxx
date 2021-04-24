#include "Ensemble.h"

ClassImp(Ensemble)

Ensemble::Ensemble(TString infiles)
{
  printf("reading tree chain from %s\n",infiles.Data());
  tr = new TChain("tree");
  tr->Add(infiles);
  ENT = tr->GetEntries(); 
  tr->SetBranchAddress("evnum",&evnum);
  tr->SetBranchAddress("runnum",&runnum);
  tr->SetBranchAddress("hadRow",hadRow);
  tr->SetBranchAddress("hadIdx",hadIdx);
  this->StartOver();
};


void Ensemble::StartOver() {
  e=-1;
  ub=-1;
  lb=-1;
  this->ResetLists();
};


void Ensemble::Build(Long64_t i) {

  // reset lists
  this->ResetLists();

  // set `e`, and check if in range
  e = i;
  if(e<0 || e>=ENT) {
    fprintf(stderr,"ERROR: e out of range\n");
    return;
  };

  // get unique event identifiers, and set them to the `*Ens` variables,
  // which will be used to compare other tree entries to check if they
  // should belong to this ensemble
  tr->GetEntry(e);
  runnumEns = runnum;
  evnumEns = evnum;

  // add dihadron/diphoton at entry `e` to the lists
  this->AppendLists(e);

  // find the other dihadrons/diphotons of this event, and add them
  // to this ensemble
  // -- read the tree forward, until we cross into the next event
  readForward = true;
  eF = e;
  ub = -1;
  while(readForward) {
    eF++;
    if(eF<ENT) {
      tr->GetEntry(eF);
      readForward = SameEvent();
    } else readForward = false;
    if(readForward) AppendLists(eF);
    else ub = eF-1;
  };
  // -- read the tree backward, until we cross into the previous event
  readBackward = true;
  eB = e;
  lb = -1;
  while(readBackward) {
    eB--;
    if(eB>=0) {
      tr->GetEntry(eB);
      readBackward = SameEvent();
    } else readBackward = false;
    if(readBackward) AppendLists(eB);
    else lb = eB+1;
  };
};


Bool_t Ensemble::NextEvent() {
  if(e!=lb) {
    fprintf(stderr,"WARNING: e or lb has changed, you may be missing an event\n");
  };
  eNext = ub+1;
  if(eNext>=ENT || eNext<0) return false;
  this->Build(eNext);
  return true;
};


void Ensemble::ResetLists() {
  diphotonList.clear();
  dihadronList.clear();
  hadronList.clear();
  rowList.clear();
};


Bool_t Ensemble::SameEvent() {
  return runnum==runnumEns && evnum==evnumEns;
};


void Ensemble::AppendLists(Long64_t j) {
  // if diphoton, add to diphotonList
  if(hadIdx[qA]==kPhoton && hadIdx[qB]==kPhoton) {
    diphotonList.push_back(j);
    for(int h=0; h<2; h++) rowList.push_back(hadRow[h]);
  }
  // if dihadron, add to dihadronList
  // also add hadrons to hadronList, maintaining uniqueness
  else {
    dihadronList.push_back(j);
    for(int h=0; h<2; h++) {
      // hadrons should not be paired with single photons
      if(hadIdx[h]==kPhoton) {
        fprintf(stderr,"WARNING: photon paired with hadron, skipping\n");
        return;
      };
      // if this hadron has not been added to rowList, add it to
      // the hadronList and its row to rowList
      if(find(rowList.begin(),rowList.end(),hadRow[h])==rowList.end()) {
        hadronList.push_back(pair<Long64_t,Int_t>(j,h));
        rowList.push_back(hadRow[h]);
      };
    };
  };
};
