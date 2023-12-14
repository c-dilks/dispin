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
TString dataPlotsFile;
Int_t whichPair;
Int_t whichHad[2];
TString hadName[2];
TString hadTitle[2];

const Int_t kpAll = 1000000;
const Int_t kpUnknown = 1000001;
const Int_t kpComb = 1000002;
const Int_t kpCombUnk = 1000003;

Double_t electronCntData, electronCntMC;
map<TString,TH1D*> dataDists;
vector<Int_t> kfList;


// Parent class
// -- store histograms etc. for each dihadron parent type
class Parent {
  public:
    Int_t KF;
    TString title;
    Color_t col;
    Style_t sty;
    map<TString,TH1D*> dists;

    // constructor
    Parent(
      Int_t KF_, TString title_, Color_t col_, Style_t sty_
    ) { 

      // set vars
      KF=KF_;
      title=title_;
      col=col_;
      sty=sty_;

      // clone data histograms
      TString kn = Form("_%d",KF);
      for(auto const & kv : dataDists) {
        TString distN = kv.first;
        TH1D * dataDist = kv.second;
        dists.insert(pair<TString,TH1D*>(
          distN,
          new TH1D(
            distN+kn,
            dataDist->GetTitle(),
            dataDist->GetNbinsX(),
            dataDist->GetXaxis()->GetXmin(),
            dataDist->GetXaxis()->GetXmax()
          )
        ));
      };

      // format
      for(auto const & kv : dists) {
        kv.second->SetLineColor(col);
        kv.second->SetLineStyle(sty);
        kv.second->SetLineWidth(KF==kpAll?3:2);
      };

      // add to list of kf codes
      kfList.push_back(KF);
    };
};

map<Int_t,Parent> parMap;

// subroutines ////////////////////////////////////
// build a canvas for specified distribution; also normalizes
// distributions to the # of electrons
TCanvas * BuildCanvas(TString distName) {
  TString canvName = distName + "Canv";
  TCanvas * canv = new TCanvas(canvName,canvName,1200,600);
  TLegend * leg = new TLegend(0.1,0.1,0.9,0.9);
  Double_t max=0;
  Double_t maxTmp;
  canv->Divide(2,1);

  // normalize and draw data distribution
  canv->cd(1);
  dataDists.at(distName)->Scale(1/electronCntData);
  dataDists.at(distName)->Draw("PE");
  max=dataDists.at(distName)->GetMaximum();
  leg->AddEntry(dataDists.at(distName),"data","PE");

  // normalize and draw the MC distributions
  // - loops through sorted list of parents, `kfList` so that the legend is
  //   sorted in the same order we defined when calling `parMap.insert()`
  for(Int_t kfVal : kfList) {
    auto par = parMap.at(kfVal);
    par.dists.at(distName)->Scale(1/electronCntMC);
    par.dists.at(distName)->Draw("HIST SAME");
    maxTmp = par.dists.at(distName)->GetMaximum();
    max = maxTmp>max ? maxTmp:max;
    leg->AddEntry(par.dists.at(distName),par.title,"LE");
  };
  dataDists.at(distName)->GetYaxis()->SetRangeUser(0,1.1*max);

  // draw legend
  canv->cd(2);
  leg->Draw();

  return canv;
};


////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////

int main(int argc, char** argv) {

  // ARGUMENTS
  whichPair = EncodePairType(kPip,kPim);
  if(argc<3) {
    fprintf(stderr,"USAGE: %s [outroot file(s)] [diagnostics plots.root file] [pairType]\n",argv[0]);
    return 1;
  }
  infiles = TString(argv[1]);
  dataPlotsFile = TString(argv[2]);
  if(argc>3) whichPair = (Int_t)strtof(argv[3],NULL);


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


  // read dists from plots.root, produced from data
  TFile * plotsFile = new TFile(dataPlotsFile,"READ");
  TString plotN,plotK,plotT;
  electronCntData = ((TH1D*)plotsFile->Get("dihadronCntDist"))->GetEntries();
  dataDists.insert(pair<TString,TH1D*>("Mh",(TH1D*)plotsFile->Get("MhDist")));
  dataDists.insert(pair<TString,TH1D*>("Q2",(TH1D*)plotsFile->Get("Q2Dist")));
  dataDists.insert(pair<TString,TH1D*>("x",(TH1D*)plotsFile->Get("XDist")));
  dataDists.insert(pair<TString,TH1D*>("z",(TH1D*)plotsFile->Get("ZpairDist")));
  dataDists.insert(pair<TString,TH1D*>("PhPerp",(TH1D*)plotsFile->Get("PhPerpDist")));
  dataDists.insert(pair<TString,TH1D*>("xF",(TH1D*)plotsFile->Get("xFDist")));
  dataDists.insert(pair<TString,TH1D*>("YH",(TH1D*)plotsFile->Get("YHDist")));
  for(int h=0; h<2; h++) {
    plotK=Form("YH%d",h); plotN=hadName[h]+"hadYHDist";
    dataDists.insert(pair<TString,TH1D*>(plotK,(TH1D*)plotsFile->Get(plotN)));
    plotT = dataDists.at(plotK)->GetTitle();
    plotT(TRegexp("(.*)")) = "from "+hadTitle[h];
    dataDists.at(plotK)->SetTitle(plotT);
  };
  dataDists.insert(pair<TString,TH1D*>("PhiH",(TH1D*)plotsFile->Get("PhiHDist")));
  dataDists.insert(pair<TString,TH1D*>("PhiR",(TH1D*)plotsFile->Get("PhiRDist")));
  dataDists.insert(pair<TString,TH1D*>("theta",(TH1D*)plotsFile->Get("thetaDist")));
  dataDists.insert(pair<TString,TH1D*>("diphM",(TH1D*)plotsFile->Get("diphMdist")));

  // format data dists
  for(auto const & kv : dataDists) {
    kv.second->SetMarkerStyle(kFullCircle);
    kv.second->SetMarkerColor(kBlack);
    kv.second->SetMarkerSize(1);
    kv.second->SetLineWidth(2);
  };

  // define output file
  TFile * outfile = new TFile("mhdecomp.root","RECREATE");


  // define Parent objects, and store them in a map
  //    parMap : KFcode -> Parent object
  parMap.insert(pair<Int_t,Parent>(kpAll,Parent(kpAll,"all",kBlack,kSolid)));
  parMap.insert(pair<Int_t,Parent>(113,Parent(113,"#rho^{0}",kBlue,kSolid)));
  parMap.insert(pair<Int_t,Parent>(223,Parent(223,"#omega",kMagenta,kSolid)));
  parMap.insert(pair<Int_t,Parent>(310,Parent(310,"K_{S}^{0}",kGreen+1,kSolid)));
  parMap.insert(pair<Int_t,Parent>(221,Parent(221,"#eta",kCyan,kSolid)));
  parMap.insert(pair<Int_t,Parent>(331,Parent(331,"#eta'",kCyan,kDashed)));
  parMap.insert(pair<Int_t,Parent>(333,Parent(333,"#phi",kMagenta,kDashed)));
  parMap.insert(pair<Int_t,Parent>(92,Parent(92,"string fragment",kBlack,kDashed)));
  parMap.insert(pair<Int_t,Parent>(91,Parent(91,"cluster fragment",kGray,kDashed)));
  parMap.insert(pair<Int_t,Parent>(-1,Parent(-1,"undefined single parent",kGreen+1,kDashed)));
  
  // combinatorial BG contributions, where the parents are different:
  // - both parents are defined:
  parMap.insert(pair<Int_t,Parent>(kpComb,Parent(kpComb,"combinatorial BG (defined)",kRed,kDashed)));
  // - one or both parents are undefined:
  parMap.insert(pair<Int_t,Parent>(kpCombUnk,Parent(kpCombUnk,"combinatorial BG (undefined)",kBlue,kDashed)));





  // event loop -------------------------
  Int_t parPid;
  Int_t evnumTmp = -10000;
  for(int i=0; i<ev->ENT; i++) {
    //if(i>100000) break; // limiter
    ev->GetEvent(i);

    // event selection
    if(ev->Valid()) {

      // increment electron counter
      if(evnumTmp<0) evnumTmp=ev->evnum;
      if(ev->evnum!=evnumTmp) {
        //printf("event %d\n",evnumTmp);
        electronCntMC++;
        evnumTmp=ev->evnum;
      };

      // decide which histograms to fill
      for(int i=0; i<=1; i++) {
        // determine parPid, which will decide the histogram
        switch(i) {
          case 0:
            if(ev->gen_hadParentIdx[qA] == ev->gen_hadParentIdx[qB]) {
              // hadrons are siblings (have same parent)
              parPid = parMap.find(ev->gen_hadParentPid[qA]) != parMap.end() ?
                                   ev->gen_hadParentPid[qA] : kpUnknown;
            } else {
              // hadrons are cousins (have different parents): combinatorial bg
              if(ev->gen_hadParentPid[qA]==-1 || ev->gen_hadParentPid[qB]==-1)
                parPid = kpCombUnk; // at least one parent is undefined
              else
                parPid = kpComb; // both parents are defined
            };
            break;
          case 1:
            // all hadrons inclusively
            parPid = kpAll;
            break;
        }

        if(parPid==kpUnknown) {
          fprintf(stderr,"WARNING: could not find common parent in event %d\n",ev->evnum);
          continue;
        };

        // fill histograms
        auto par = parMap.at(parPid);
        par.dists.at("Mh")->Fill(ev->Mh);
        par.dists.at("Q2")->Fill(ev->Q2);
        par.dists.at("x")->Fill(ev->x);
        par.dists.at("z")->Fill(ev->Zpair);
        par.dists.at("PhPerp")->Fill(ev->PhPerp);
        par.dists.at("xF")->Fill(ev->xF);
        par.dists.at("YH")->Fill(ev->YH);
        for(int h=0; h<2; h++) {
          plotK=Form("YH%d",h); par.dists.at(plotK)->Fill(ev->hadYH[h]);
        };
        par.dists.at("PhiH")->Fill(ev->PhiH);
        par.dists.at("PhiR")->Fill(ev->PhiR);
        par.dists.at("theta")->Fill(ev->theta);
        par.dists.at("diphM")->Fill(ev->objDiphoton->M);
      };

    };
  };
  electronCntMC++; // (count the last one)

  printf("num. electrons in MC: %f\n",electronCntMC);
  printf("num. electrons in data: %f\n",electronCntData);


  // draw (also normalizes by number of electrons)
  vector<TCanvas*> canvVec;
  for(auto const & dataKV : dataDists) {
    canvVec.push_back(
      BuildCanvas(dataKV.first)
    );
  };


  // write
  for(TCanvas * c : canvVec) c->Write();
  for(auto const & parKV : parMap) {
    for(auto distKV : parKV.second.dists) {
      distKV.second->Write();
    };
  };
  for(auto distKV : dataDists) distKV.second->Write();


  // close
  outfile->Close();
  return 0;
};

