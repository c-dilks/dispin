#include <iostream>
#include <vector>
#include <memory>

#include <TFile.h>
#include <TTree.h>
#include <TChain.h>
#include <TString.h>
#include <TH1.h>
#include <TH2.h>

#include "src/Constants.h"
#include "src/Tools.h"
#include "src/EventTree.h"

using std::cout;
using std::cerr;
using std::endl;

int main(int argc, char** argv) {

  enum file_classes {
    cInbendingElectron,
    cInbendingPositron,
    cOutbendingElectron,
    cOutbendingPositron
  };
  std::vector<std::pair<TString,TString>> leptos{
    { "inbending_electron",  "Inbending e^{-}"  },
    { "inbending_positron",  "Inbending e^{+}"  },
    { "outbending_electron", "Outbending e^{-}" },
    { "outbending_positron", "Outbending e^{+}" }
  };

  // parse arguments
  std::vector<TString> outrootDirs;
  if(argc<5) {
    cerr << "USAGE: " << argv[0];
    for(const auto& [leptoName, _] : leptos)
      cerr << " [" << leptoName << "]";
    cerr << endl << " - each argument should be an outroot dir" << endl;
    return 2;
  }
  for(int a=1; a<5; a++)
    outrootDirs.push_back(TString(argv[a]));
  auto whichPair = EncodePairType(kPip,kPim); // FIXME

  // read event trees
  std::vector<std::unique_ptr<EventTree>> evTrees;
  cout << "READING DATA FROM:" << endl;
  for(const auto& outrootDir : outrootDirs) {
    cout << " - " << outrootDir << endl;
    evTrees.push_back(std::move(std::make_unique<EventTree>(outrootDir + "/*.root", whichPair)));
  }

  // create output file
  TString outFileN = "non_dis.root";
  auto outFile = new TFile(outFileN, "RECREATE");
  const int NBINS = 100;

  // lepton histograms and counters
  std::vector<TH1D*> leptoTheta;
  std::vector<TH1D*> leptoP;
  std::vector<TH2D*> leptoThetaVsP;
  std::vector<long> leptoCounts;
  for(const auto& [leptoName, leptoTitle] : leptos) {
    leptoTheta.push_back(new TH1D(leptoName+"_theta", leptoTitle+" #theta;#theta [deg]", NBINS, 0, 40));
    leptoP.push_back(new TH1D(leptoName+"_p", leptoTitle+" p;p [GeV]", NBINS, 0, 11));
    leptoThetaVsP.push_back(new TH2D(leptoName+"_theta_vs_p", leptoTitle+" #theta vs. p;p [GeV];#theta [deg]", NBINS, 0, 11, NBINS, 0, 40));
    leptoCounts.push_back(0);
  }

  // fill lepton histograms
  auto fillLepto = [&evTrees, &leptoTheta, &leptoP, &leptoThetaVsP, &leptoCounts] (std::size_t idx) {
    const auto& ev = evTrees.at(idx);
    for(Long64_t e=0; e<ev->ENT; e++) {
      if(e>10000) break; // limiter
      ev->GetEvent(e);
      if(ev->Valid()) {
        leptoTheta.at(idx)->Fill(ev->eleTheta);
        leptoP.at(idx)->Fill(ev->eleP);
        leptoThetaVsP.at(idx)->Fill(ev->eleP, ev->eleTheta);
        leptoCounts.at(idx)++;
      }
    }
  };
  fillLepto(cInbendingElectron);
  fillLepto(cOutbendingElectron);

  // write histograms
  for(std::size_t idx=0; idx<leptoCounts.size(); idx++) {
    leptoTheta.at(idx)->Write();
    leptoP.at(idx)->Write();
    leptoThetaVsP.at(idx)->Write();
    cout << leptos.at(idx).first << " COUNTS: " << leptoCounts.at(idx) << endl;
  }

  outFile->Close();
  return 0;
}
