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
    Modu(int key_, TString name_, func_t func_) : key(key_), name(name), func(func_) {}
    Double_t Eval(EventTree* e) const { return func(e); }
    int GetKey() const { return key; }
    TString GetName() const { return name; }
};


int main(int argc, char** argv) {

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


   // define modulations
   auto pwTitle = [](int twist, int m) { return Form("|L,%d>_{%d}", m, twist); };
   auto sdmeTitle = [](int alpha, int hel, int helPrime) { return Form("r^{%d}_{%d%d}", alpha, hel, helPrime); };
   std::vector<Modu> pwModus = {
     Modu(0, pwTitle(2,1), [](EventTree* ev) { return TMath::Sin(ev->PhiH - ev->PhiR); }),
     Modu(1, pwTitle(2,2), [](EventTree* ev) { return TMath::Sin(2*ev->PhiH - 2*ev->PhiR); })
   };
   std::vector<Modu> sdmeModus = {
     Modu(0, sdmeTitle(3,1,0), [](EventTree* ev) { return TMath::Sin(ev->sdmePhiL); }),
     Modu(1, sdmeTitle(3,1,-1), [](EventTree* ev) { return TMath::Sin(2*ev->sdmePhiL); })
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
     // if(i>10000) break; // limiter

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

   // write plots
   for(const auto& [pwKey, sdmeDists] : corrDists) {
     for(const auto& [sdmeKey, corrDist] : sdmeDists) {
       corrDist->Write();
     }
   }

   // draw
   int const numPlots = pwModus.size() * sdmeModus.size();
   auto corrCanv = new TCanvas("corrCanv", "corrCanv", 1000, 1000);
   corrCanv->Divide(pwModus.size() + 1, sdmeModus.size() + 1);
   for(const auto& [pwKey, sdmeDists] : corrDists) {
     for(const auto& [sdmeKey, corrDist] : sdmeDists) {
       int padNum = (pwKey + 1) + (pwModus.size() + 1) * (sdmeKey + 1) + 1;
       corrCanv->cd(padNum);
       corrDist->Draw("col");
     }
   }
   corrCanv->Write();
   outfile->Close();

}

