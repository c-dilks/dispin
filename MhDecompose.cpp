#include <cstdlib>
#include <iostream>
#include <map>
#include <vector>

// ROOT
#include "TFile.h"
#include "TTree.h"
#include "TChain.h"
#include "TString.h"
#include "TMath.h"
#include "TSystem.h"
#include "TRegexp.h"
#include "TH1.h"
#include "TH2.h"
#include "TCanvas.h"
#include "TLegend.h"

// Dispin
#include "Constants.h"
#include "Tools.h"
#include "EventTree.h"

using namespace std;

TString infiles;
Int_t whichPair;
Int_t whichHad[2];
TString hadName[2];
TString hadTitle[2];

const Int_t kpAll = 1000000;
const Int_t kpUnknown = 1000001;


// Parent class
// -- store histograms etc. for each dihadron parent
class Parent {
  public:
    Int_t KF;
    TString title;
    Color_t col;
    Style_t sty;
    map<TString,TH1D*> dists;
    Parent(
      Int_t KF_, TString title_, Color_t col_, Style_t sty_
    ) { 
      KF=KF_;
      title=title_;
      col=col_;
      sty=sty_;
      TString kn = Form("_%d",KF);
      dists.insert(pair<TString,TH1D*>("Mh",new TH1D(TString("MhDist"+kn),"M_{h} distribution",150,0,3)));
      dists.insert(pair<TString,TH1D*>("x",new TH1D(TString("xDist"+kn),"x distribution",150,0,1)));
      dists.insert(pair<TString,TH1D*>("z",new TH1D(TString("zDist"+kn),"z distribution",150,0,1)));
      for(auto const & kv : dists) {
        kv.second->SetLineColor(col);
        kv.second->SetLineStyle(sty);
        kv.second->SetLineWidth(KF==kpAll?4:2);
      };
    };
};

map<Int_t,Parent> parMap;

// subroutines ////////////////////////////////////
TCanvas * BuildCanvas(TString distName) {
  TString canvName = distName + "Canv";
  TCanvas * canv = new TCanvas(canvName,canvName,1200,600);
  canv->Divide(2,1);
  canv->cd(1);
  TLegend * leg = new TLegend(0.1,0.1,0.9,0.9);
  parMap.at(kpAll).dists.at(distName)->Draw();
  for(auto const & kv : parMap) {
    auto par = kv.second;
    par.dists.at(distName)->Draw("SAME");
    leg->AddEntry(par.dists.at(distName),par.title,"LE");
  };
  canv->cd(2);
  leg->Draw();
  return canv;
};


////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////

int main(int argc, char** argv) {

  // ARGUMENTS
  infiles = "outroot.mcrec.injgen/*.root";
  whichPair = EncodePairType(kPip,kPim);
  if(argc>1) infiles = TString(argv[1]);
  if(argc>2) whichPair = (Int_t)strtof(argv[2],NULL);

  // get hadron pair from whichPair; note that in the print out, the 
  // order of hadron 0 and 1 is set by Constants::dihHadIdx
  printf("whichPair = 0x%x\n",whichPair);
  DecodePairType(whichPair,whichHad[qA],whichHad[qB]);
  for(int h=0; h<2; h++) {
    hadName[h] = PairHadName(whichHad[qA],whichHad[qB],h);
    hadTitle[h] = PairHadTitle(whichHad[qA],whichHad[qB],h);
    printf("hadron %d:  idx=%d  name=%s  title=%s\n",
        h,dihHadIdx(whichHad[qA],whichHad[qB],h),hadName[h].Data(),hadTitle[h].Data());
  };

  EventTree * ev = new EventTree(infiles,whichPair);


  // define output file
  TFile * outfile = new TFile("mhdecomp.root","RECREATE");


  // parMap : KFcode -> Parent object
  parMap.insert(pair<Int_t,Parent>(-1,Parent(-1,"undefined",kYellow-3,kSolid)));
  parMap.insert(pair<Int_t,Parent>(91,Parent(91,"cluster",kGray,kSolid)));
  parMap.insert(pair<Int_t,Parent>(92,Parent(92,"string",kGray,kDashed)));
  parMap.insert(pair<Int_t,Parent>(113,Parent(113,"#rho^{0}",kBlue,kSolid)));
  parMap.insert(pair<Int_t,Parent>(221,Parent(221,"#eta",kCyan,kSolid)));
  parMap.insert(pair<Int_t,Parent>(223,Parent(223,"#omega",kMagenta,kSolid)));
  parMap.insert(pair<Int_t,Parent>(310,Parent(310,"K_{S}^{0}",kGreen+1,kSolid)));
  parMap.insert(pair<Int_t,Parent>(331,Parent(331,"#eta'",kCyan,kDashed)));
  parMap.insert(pair<Int_t,Parent>(333,Parent(333,"#phi",kRed,kSolid)));
  parMap.insert(pair<Int_t,Parent>(kpAll,Parent(kpAll,"all",kBlack,kSolid)));
  parMap.insert(pair<Int_t,Parent>(kpUnknown,Parent(kpUnknown,"unknown",kYellow-3,kDashed)));




  // event loop -------------------------
  Int_t parPid;
  for(int i=0; i<ev->ENT; i++) {
    //if(i>10000) break; // limiter
    ev->GetEvent(i);

    ev->cutHelicity=true; // override requirement for having defined helicity
    if(ev->Valid()) {

      // hadrons are siblings (have same parent)
      if(ev->gen_hadParentIdx[qA] == ev->gen_hadParentIdx[qB]) {

        for(int i=0; i<2; i++) {
          if(i==0) {
            parPid = parMap.find(ev->gen_hadParentPid[qA]) != parMap.end() ?
                     ev->gen_hadParentPid[qA] : kpUnknown;
          } else parPid = kpAll;
          auto par = parMap.at(parPid);
          par.dists.at("Mh")->Fill(ev->Mh);
          par.dists.at("x")->Fill(ev->x);
          par.dists.at("z")->Fill(ev->Zpair);
        };

      };
    };
  };


  // draw
  vector<TCanvas*> canvVec;
  canvVec.push_back(BuildCanvas("Mh"));
  canvVec.push_back(BuildCanvas("x"));
  canvVec.push_back(BuildCanvas("z"));
  for(TCanvas * c : canvVec) c->Write();


  // close
  outfile->Close();
  return 0;
};

