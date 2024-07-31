#include <cstdlib>

// ROOT
#include "TFile.h"
#include "TTree.h"
#include "TChain.h"
#include "TString.h"
#include "TMath.h"
#include "TRegexp.h"
#include "TH2.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TLatex.h"

// Dispin
#include "src/Constants.h"
#include "src/Tools.h"
#include "src/EventTree.h"

TString infiles;
TString outfileN;
Int_t whichPair;
Int_t whichHad[2];
TString hadName[2];
TString hadTitle[2];
TString dihTitle;

class Modu {
  using func_t = std::function<Double_t(EventTree*)>;
  private:
    int key;
    TString name;
    func_t func;
  public:
    Modu(int key_, TString name_, func_t func_) : key(key_), name(name_), func(func_) {}
    Double_t Eval(EventTree* e) const { return func(e); }
    int GetKey() const { return key; }
    TString GetName() const { return name; }
};


int main(int argc, char** argv) {

   gStyle->SetOptStat(0);

   // ARGUMENTS
   infiles = "outroot/*.root";
   outfileN = "sdme.root";
   whichPair = EncodePairType(kPip,kPim);
   if(argc==1) {
     fprintf(stderr,"USAGE: %s [outroot file(s)] [output sdme.root file] [pairType]\n",argv[0]);
     return 1;
   };
   if(argc>1) infiles = TString(argv[1]);
   if(argc>2) outfileN = TString(argv[2]);
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
   }
   dihTitle = PairTitle(whichPair);

   EventTree * ev = new EventTree(infiles,whichPair);


   // define PW modulations
   auto pwTitle = [](int twist, int m) { return Form("|L,%d>_{%d}", m, twist); };
   auto makeTw2pw = [](int m) { return [m](EventTree* ev) { return TMath::Sin(m*ev->PhiH - m*ev->PhiR); }; };
   auto makeTw3pw = [](int m) { return [m](EventTree* ev) { return TMath::Sin((1-m)*ev->PhiH + m*ev->PhiR); }; };
   auto makeTw3pwSum  = [](int m) { return [m](EventTree* ev) { return 0.5 * (TMath::Sin((1+m)*ev->PhiH - m*ev->PhiR) + TMath::Sin((1-m)*ev->PhiH + m*ev->PhiR)); }; };
   auto makeTw3pwDiff = [](int m) { return [m](EventTree* ev) { return 0.5 * (TMath::Sin((1+m)*ev->PhiH - m*ev->PhiR) - TMath::Sin((1-m)*ev->PhiH + m*ev->PhiR)); }; };
   std::vector<Modu> pwModus = {
     Modu(0, pwTitle(2,1),  makeTw2pw(1)),
     Modu(1, pwTitle(2,2),  makeTw2pw(2)),
     Modu(2, pwTitle(3,0),  makeTw3pw(0)),
     Modu(3, pwTitle(3,1),  makeTw3pw(1)),
     Modu(4, pwTitle(3,2),  makeTw3pw(2)),
     Modu(5, pwTitle(3,-1), makeTw3pw(-1)),
     Modu(6, pwTitle(3,-2), makeTw3pw(-2)),
     Modu(7, pwTitle(3,-1) + TString(" + ") + pwTitle(3,1), makeTw3pwSum(1)),
     Modu(8, pwTitle(3,-2) + TString(" + ") + pwTitle(3,2), makeTw3pwSum(2)),
     Modu(9, pwTitle(3,-1) + TString(" - ") + pwTitle(3,1), makeTw3pwDiff(1)),
     Modu(10, pwTitle(3,-2) + TString(" - ") + pwTitle(3,2), makeTw3pwDiff(2))
   };

   // define SDME modulations
   auto sdmeTitle = [](int alpha, int hel, int helPrime) { return Form("r^{%d}_{%d%d}", alpha, hel, helPrime); };
   std::vector<Modu> sdmeModus = {
     Modu(0, sdmeTitle(3,1,0), [](EventTree* ev) { return TMath::Sin(ev->sdmePhiL); }),
     Modu(1, sdmeTitle(3,1,-1), [](EventTree* ev) { return TMath::Sin(2*ev->sdmePhiL); }),
     Modu(2, sdmeTitle(7,1,0), [](EventTree* ev) { return TMath::Cos(ev->sdmePhiU) * TMath::Sin(ev->sdmePhiL); }),
     Modu(3, sdmeTitle(7,1,-1), [](EventTree* ev) { return TMath::Cos(ev->sdmePhiU) * TMath::Sin(2*ev->sdmePhiL); }),
     Modu(4, sdmeTitle(8,1,1) + TString(" and ") + sdmeTitle(8,0,0), [](EventTree* ev) { return TMath::Sin(ev->sdmePhiU); }),
     Modu(5, sdmeTitle(8,1,0), [](EventTree* ev) { return TMath::Sin(ev->sdmePhiU) * TMath::Cos(ev->sdmePhiL); }),
     Modu(6, sdmeTitle(8,1,-1), [](EventTree* ev) { return TMath::Sin(ev->sdmePhiU) * TMath::Cos(2*ev->sdmePhiL); })
   };


   // define correlation plots
   std::map<int, std::map<int, TH2D*>> corrDists;
   for(auto const& pwModu : pwModus) {
     corrDists.insert({pwModu.GetKey(), {}});
     for(auto const& sdmeModu : sdmeModus) {
       TString plotName = Form("corr_%d_%d", pwModu.GetKey(), sdmeModu.GetKey());
       corrDists.at(pwModu.GetKey()).insert(std::make_pair<int, TH2D*>(sdmeModu.GetKey(), new TH2D(plotName, plotName, 100, -1, 1, 100, -1, 1)));
     }
   }


   TFile * outfile = new TFile(outfileN,"RECREATE");


   ///////////////////////////
   //                       //
   //      EVENT LOOP       //
   //                       //
   ///////////////////////////

   printf("begin loop through %lld events...\n",ev->ENT);
   for(int i=0; i<ev->ENT; i++) {
     // if(i>50000) break; // limiter

     ev->GetEvent(i);

     if(ev->Valid()) {

       // calculate modulations
       std::map<int, Double_t> pwVals;
       std::map<int, Double_t> sdmeVals;
       for(auto const& pwModu : pwModus)
         pwVals.insert({pwModu.GetKey(), pwModu.Eval(ev)});
       for(auto const& sdmeModu : sdmeModus)
         sdmeVals.insert({sdmeModu.GetKey(), sdmeModu.Eval(ev)});

       // fill correlation plots
       for(const auto& [pwKey, pwVal] : pwVals) {
         for(const auto& [sdmeKey, sdmeVal] : sdmeVals) {
           corrDists.at(pwKey).at(sdmeKey)->Fill(pwVal, sdmeVal);
         }
       }

     }
   } // eo event loop

   // draw
   int const numPlots = pwModus.size() * sdmeModus.size();
   auto corrCanv = new TCanvas("corrCanv", "corrCanv", 1000, 1000);
   corrCanv->Divide(pwModus.size() + 1, sdmeModus.size() + 1);
   for(const auto& [pwKey, sdmeDists] : corrDists) {
     for(const auto& [sdmeKey, corrDist] : sdmeDists) {
       int padNum = (pwKey + 1) + (pwModus.size() + 1) * (sdmeKey + 1) + 1;
       auto corrPad = corrCanv->GetPad(padNum);
       corrPad->cd();
       corrPad->SetLogz();
       corrDist->Draw("col");
     }
   }
   for(const auto& pwModu : pwModus) {
     auto corrPad = corrCanv->GetPad(pwModu.GetKey() + 2);
     corrPad->cd();
     corrPad->DrawFrame(0,0,1,1);
     auto tex = new TLatex(0.1, 0.3, pwModu.GetName());
     tex->SetTextSize(0.14);
     tex->Draw();
   }
   for(const auto& sdmeModu : sdmeModus) {
     auto corrPad = corrCanv->GetPad((sdmeModu.GetKey() + 1) * (pwModus.size() + 1) + 1);
     corrPad->cd();
     corrPad->DrawFrame(0,0,1,1);
     auto tex = new TLatex(0.1, 0.3, sdmeModu.GetName());
     tex->SetTextSize(0.14);
     tex->Draw();
   }

   // write plots
   corrCanv->Write();
   for(const auto& [pwKey, sdmeDists] : corrDists) {
     for(const auto& [sdmeKey, corrDist] : sdmeDists) {
       corrDist->Write();
     }
   }

   outfile->Close();

}

