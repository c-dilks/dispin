#include <iostream>
#include <vector>
#include <memory>

#include <TFile.h>
#include <TString.h>
#include <TH1.h>
#include <TH2.h>
#include <TCanvas.h>

#include "src/EventTree.h"

using std::cout;
using std::cerr;
using std::endl;

int main(int argc, char** argv) {

  const Long64_t LIMITER = 100000; // set to 0 to disable
  if(LIMITER>0)
    cerr << endl << endl << endl << endl << endl << "WARNING: LIMITER is set to " << LIMITER << endl << endl << endl << endl << endl << endl;

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
    cerr << endl;
    cerr << " - each argument should be an outroot dir" << endl;
    cerr << " - the binning schemes are hard-coded; this will run all of them at once" << endl;
    return 2;
  }
  for(int a=1; a<5; a++)
    outrootDirs.push_back(TString(argv[a]));
  auto whichPair = EncodePairType(kPip,kPim); // FIXME

  // set binning schemes
  std::map<std::string, std::unique_ptr<Binning>> BSmap;
  BSmap.insert({"x", std::make_unique<Binning>()});
  BSmap.insert({"m", std::make_unique<Binning>()});
  BSmap.insert({"zm", std::make_unique<Binning>()});
  BSmap.insert({"ptm", std::make_unique<Binning>()});
  BSmap.at("x")->SetScheme(whichPair, 1, 6);
  BSmap.at("m")->SetScheme(whichPair, 2, 6);
  BSmap.at("zm")->SetScheme(whichPair, 32, 3, 2);
  BSmap.at("ptm")->SetScheme(whichPair, 42, 3, 2);

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
  std::vector<TH2D*> leptoQ2vsX;
  std::vector<TH1D*> dihX;
  std::vector<TH1D*> dihMh;
  std::vector<TH2D*> dihZvsMh;
  std::vector<TH2D*> dihPhPerpVsMh;
  std::vector<long> leptoCounts;
  for(const auto& [leptoName, leptoTitle] : leptos) {
    leptoTheta.push_back(new TH1D(leptoName+"_theta", leptoTitle+" #theta;#theta [deg]", NBINS, 0, 40));
    leptoP.push_back(new TH1D(leptoName+"_p", leptoTitle+" p;p [GeV]", NBINS, 0, 11));
    leptoThetaVsP.push_back(new TH2D(leptoName+"_theta_vs_p", leptoTitle+" #theta vs. p;p [GeV];#theta [deg]", NBINS, 0, 11, NBINS, 0, 40));
    leptoQ2vsX.push_back(new TH2D(leptoName+"_Q2_vs_x", leptoTitle+" Q^{2} vs. x;x;Q^{2} [GeV^{2}]", NBINS, 0, 1, NBINS, 0, 12));
    dihX.push_back(new TH1D(
          leptoName+"_dihadron_x",
          leptoTitle+" x;x",
          BSmap.at("x")->GetBinArray(0)->GetArray()
          ));
    dihMh.push_back(new TH1D(
          leptoName+"_dihadron_Mh",
          leptoTitle+" dihadron M_{h};M_{h} [GeV]",
          BSMap.at("m")->GetBinArray(0)->GetArray()
          ));
    dihZvsMh.push_back(new TH2D(leptoName+"_dihadron_z_vs_Mh",
          leptoTitle+" dihadron z vs. M_{h};M_{h} [GeV];z",
          BSMap.at("zm")->GetBinArray(1)->GetArray(),
          BSMap.at("zm")->GetBinArray(0)->GetArray()
          ));
    dihPhPerpVsMh.push_back(new TH2D(
          leptoName+"_dihadron_PhPerp_vs_Mh",
          leptoTitle+" dihadron P_{h}^{perp} vs. M_{h};M_{h} [GeV];P_{h}^{perp} [GeV]",
          BSMap.at("ptm")->GetBinArray(1)->GetArray(),
          BSMap.at("ptm")->GetBinArray(0)->GetArray()
          ));
    leptoCounts.push_back(0);
  }

  // fill lepton histograms
  auto fillLepto = [&] (std::size_t idx) {
    const auto& ev = evTrees.at(idx);
    for(Long64_t e=0; e<ev->ENT; e++) {
      if(LIMITER>0 && e+1>LIMITER) break;
      ev->GetEvent(e);
      if(ev->Valid()) {
        leptoTheta.at(idx)->Fill(ev->eleTheta);
        leptoP.at(idx)->Fill(ev->eleP);
        leptoThetaVsP.at(idx)->Fill(ev->eleP, ev->eleTheta);
        leptoQ2vsX.at(idx)->Fill(ev->x, ev->Q2);
        dihX.at(idx)->Fill(ev->x);
        dihMh.at(idx)->Fill(ev->Mh);
        dihZvsMh.at(idx)->Fill(ev->Mh, ev->Zpair);
        dihPhPerpVsMh.at(idx)->Fill(ev->Mh, ev->PhPerp);
        // CUT: equal acceptance
        if(ev->eleP >= 3.0 && ev->eleP <= 6.0 && ev->eleTheta >= 12.0 && ev->eleTheta <= 25.0)
          leptoCounts.at(idx)++;
      }
    }
    cout << "COUNTS in equal-acceptance range for " << leptos.at(idx).first << ": " << leptoCounts.at(idx) << endl;
  };
  for(std::size_t idx=0; idx<leptoCounts.size(); idx++)
    fillLepto(idx);

  // normalization
  auto normalize = [&] (std::size_t idx, Long64_t norm) {
    if(norm==0) {
      cerr << "ERROR: normalization factor is zero for idx=" << idx << endl;
      return;
    }
    leptoTheta.at(idx)->Scale(1.0 / norm);
    leptoP.at(idx)->Scale(1.0 / norm);
    leptoThetaVsP.at(idx)->Scale(1.0 / norm);
    leptoQ2vsX.at(idx)->Scale(1.0 / norm);
    dihX.at(idx)->Scale(1.0 / norm);
    dihMh.at(idx)->Scale(1.0 / norm);
    dihZvsMh.at(idx)->Scale(1.0 / norm);
    dihPhPerpVsMh.at(idx)->Scale(1.0 / norm);
  };
  normalize(cInbendingElectron,  leptoCounts.at(cInbendingElectron));
  normalize(cOutbendingElectron, leptoCounts.at(cOutbendingElectron));
  normalize(cInbendingPositron,  leptoCounts.at(cInbendingElectron));
  normalize(cOutbendingPositron, leptoCounts.at(cOutbendingElectron));

  // compute nonDIS electron contamination
  auto compute_systematic = [&] (auto hist_vec) {
    hist_vec.at(cInbendingPositron)->Divide(hist_vec.at(cOutbendingElectron));
    hist_vec.at(cOutbendingPositron)->Divide(hist_vec.at(cInbendingElectron));
    hist_vec.at(cInbendingPositron)->SetTitle("non-DIS contamination of outbending data");
    hist_vec.at(cOutbendingPositron)->SetTitle("non-DIS contamination of inbending data");
  };
  compute_systematic(dihX);
  compute_systematic(dihMh);
  compute_systematic(dihZvsMh);
  compute_systematic(dihPhPerpVsMh);

  // draw
  auto canvNorm = new TCanvas("canvNorm", "canvNorm", 2*800, 600);
  canvNorm->Divide(2,1);
  canvNorm->cd(1);
  leptoTheta.at(cInbendingElectron)->SetLineColor(kBlue);
  leptoTheta.at(cOutbendingElectron)->SetLineColor(kRed);
  leptoTheta.at(cInbendingElectron)->Draw();
  leptoTheta.at(cOutbendingElectron)->Draw("same");
  canvNorm->cd(2);
  leptoP.at(cInbendingElectron)->SetLineColor(kBlue);
  leptoP.at(cOutbendingElectron)->SetLineColor(kRed);
  leptoP.at(cInbendingElectron)->Draw();
  leptoP.at(cOutbendingElectron)->Draw("same");

  int nCols = 2;
  int nRows = 2;
  std::map<file_classes, TCanvas*> canvSystematic;
  canvSystematic.insert({cOutbendingPositron, new TCanvas("canvSystematicInbending",  "canvSystematicInbending",  nCols*800, nRows*600)});
  canvSystematic.insert({cInbendingPositron,  new TCanvas("canvSystematicOutbending", "canvSystematicOutbending", nCols*800, nRows*600)});
  for(auto& [idx, canv] : canvSystematic) {
    canv->Divide(nCols, nRows);
    for(int padNum=1; padNum<=nCols*nRows; padNum++) {
      auto pad = canv->GetPad(padNum);
      pad->SetGrid(1,1);
      pad->SetLeftMargin(1.2);
      pad->SetRightMargin(1.2);
      pad->SetBottomMargin(1.2);
    }
    canv->cd(1); dihX.at(idx)->Draw();
    canv->cd(2); dihMh.at(idx)->Draw();
    canv->cd(3); dihZvsMh.at(idx)->Draw("colztext");
    canv->cd(4); dihPhPerpVsMh.at(idx)->Draw("colztext");
  }

  // write
  canvNorm->Write();
  for(std::size_t idx=0; idx<leptoCounts.size(); idx++) {
    leptoTheta.at(idx)->Write();
    leptoP.at(idx)->Write();
    leptoThetaVsP.at(idx)->Write();
    leptoQ2vsX.at(idx)->Write();
    dihX.at(idx)->Write();
    dihMh.at(idx)->Write();
    dihZvsMh.at(idx)->Write();
    dihPhPerpVsMh.at(idx)->Write();
  }

  outFile->Close();
  return 0;
}
