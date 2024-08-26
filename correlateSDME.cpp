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
    TString name2;
    func_t func;
  public:
    Modu(int key_, TString name_, TString name2_, func_t func_) : key(key_), name(name_), name2(name2_), func(func_) {}
    Double_t Eval(EventTree* e) const { return func(e); }
    int GetKey() const { return key; }
    TString GetName() const { return name; }
    TString GetName2() const { return name2; }
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
   auto makeDiFFs = [](int m, char const* ff) -> TString {
     switch(m) {
       case 0:
         return Form("%s_{UU}^{s+p}, %s_{UL}^{sp}, %s_{LL}^{pp}", ff, ff, ff);
         break;
       case 1:
       case -1:
         return Form("%s_{UT}^{sp}, %s_{LT}^{pp}", ff, ff);
         break;
       case 2:
       case -2:
         return Form("%s_{TT}^{pp}", ff);
         break;
     }
     return TString("unknown");
   };
   auto makeRho = [](char const* polGamma, char const* polRho) { return Form("#gamma_{%s}^{*} #rho_{%s}^{0}"); };
   std::vector<Modu> pwModus = {
     Modu(0, pwTitle(2,1),  makeDiFFs(1,"G"),  makeTw2pw(1)),
     Modu(1, pwTitle(2,2),  makeDiFFs(2,"G"),  makeTw2pw(2)),
     Modu(2, pwTitle(3,0),  makeDiFFs(0,"H"),  makeTw3pw(0)),
     Modu(3, pwTitle(3,1),  makeDiFFs(1,"H"),  makeTw3pw(1)),
     Modu(4, pwTitle(3,2),  makeDiFFs(2,"H"),  makeTw3pw(2)),
     Modu(5, pwTitle(3,-1), makeDiFFs(-1,"H"), makeTw3pw(-1)),
     Modu(6, pwTitle(3,-2), makeDiFFs(-2,"H"), makeTw3pw(-2)),
     Modu(7,  pwTitle(3,-1) + TString(" + ") + pwTitle(3,1), makeDiFFs(1,"H"), makeTw3pwSum(1)),
     Modu(8,  pwTitle(3,-2) + TString(" + ") + pwTitle(3,2), makeDiFFs(2,"H"), makeTw3pwSum(2)),
     Modu(9,  pwTitle(3,-1) + TString(" - ") + pwTitle(3,1), makeDiFFs(1,"H"), makeTw3pwDiff(1)),
     Modu(10, pwTitle(3,-2) + TString(" - ") + pwTitle(3,2), makeDiFFs(2,"H"), makeTw3pwDiff(2))
   };

   // define SDME modulations
   auto sdmeTitle = [](int alpha, int hel, int helPrime) { return Form("r^{%d}_{%d%d}", alpha, hel, helPrime); };
   std::vector<Modu> sdmeModus = {
     Modu(
         0,
         sdmeTitle(3,1,0),
         makeRho("T","L"),
         [](EventTree* ev) { return TMath::Sin(ev->sdmePhiL); }),
     Modu(
         1,
         sdmeTitle(3,1,-1),
         makeRho("T","-T"),
         [](EventTree* ev) { return TMath::Sin(2*ev->sdmePhiL); }),
     Modu(
         2,
         sdmeTitle(8,1,1) + TString(" and ") + sdmeTitle(8,0,0),
         makeRho("L","T") + TString(" and ") + makeRho("T","L"),
         [](EventTree* ev) { return TMath::Sin(ev->sdmePhiU); }),
     Modu(
         3,
         sdmeTitle(7,1,0),
         TString("int. ") + makeRho("L","L") + TString(" & ") + makeRho("T","T"),
         [](EventTree* ev) { return TMath::Cos(ev->sdmePhiU) * TMath::Sin(ev->sdmePhiL); }),
     Modu(
         4,
         sdmeTitle(7,1,-1),
         makeRho("L","T"),
         [](EventTree* ev) { return TMath::Cos(ev->sdmePhiU) * TMath::Sin(2*ev->sdmePhiL); }),
     Modu(
         5,
         sdmeTitle(8,1,0),
         TString("int. ") + makeRho("L","L") + TString("&") + makeRho("T","T"),
         [](EventTree* ev) { return TMath::Sin(ev->sdmePhiU) * TMath::Cos(ev->sdmePhiL); }),
     Modu(
         6,
         sdmeTitle(8,1,-1),
         makeRho("L","T"),
         [](EventTree* ev) { return TMath::Sin(ev->sdmePhiU) * TMath::Cos(2*ev->sdmePhiL); }),
     Modu(
         7,
         sdmeTitle(7,1,0) + TString("+") + sdmeTitle(8,1,0),
         TString("int. ") + makeRho("L","L") + TString(" & ") + makeRho("T","T"),
         [](EventTree* ev) { return TMath::Cos(ev->sdmePhiU) * TMath::Sin(ev->sdmePhiL) + TMath::Sin(ev->sdmePhiU) * TMath::Cos(ev->sdmePhiL); }),
     Modu(
         8,
         sdmeTitle(7,1,-1) + TString("+") + sdmeTitle(8,1,-1),
         makeRho("L","T"),
         [](EventTree* ev) { return TMath::Cos(ev->sdmePhiU) * TMath::Sin(2*ev->sdmePhiL) + TMath::Sin(ev->sdmePhiU) * TMath::Cos(2*ev->sdmePhiL); }),
     Modu(
         9,
         sdmeTitle(7,1,0) + TString("-") + sdmeTitle(8,1,0),
         TString("int. ") + makeRho("L","L") + TString(" & ") + makeRho("T","T"),
         [](EventTree* ev) { return TMath::Cos(ev->sdmePhiU) * TMath::Sin(ev->sdmePhiL) - TMath::Sin(ev->sdmePhiU) * TMath::Cos(ev->sdmePhiL); }),
     Modu(
         10,
         sdmeTitle(7,1,-1) + TString("-") + sdmeTitle(8,1,-1),
         makeRho("L","T"),
         [](EventTree* ev) { return TMath::Cos(ev->sdmePhiU) * TMath::Sin(2*ev->sdmePhiL) - TMath::Sin(ev->sdmePhiU) * TMath::Cos(2*ev->sdmePhiL); })

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
     if(i>50000) break; // limiter

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
   auto corrCanv = new TCanvas("corrCanv", "corrCanv", pwModus.size()*100, sdmeModus.size()*80);
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
   Double_t const texX = 0.03;
   Double_t const texY = 0.3;
   Double_t const tex2Y = 0.6;
   Double_t const texSize = 0.15;
   for(const auto& pwModu : pwModus) {
     auto corrPad = corrCanv->GetPad(pwModu.GetKey() + 2);
     corrPad->cd();
     // corrPad->DrawFrame(0,0,1,1);
     auto tex = new TLatex(texX, texY, pwModu.GetName());
     auto tex2 = new TLatex(texX, tex2Y, pwModu.GetName2());
     tex->SetTextSize(texSize);
     tex2->SetTextSize(texSize);
     tex->Draw();
     tex2->Draw();
   }
   for(const auto& sdmeModu : sdmeModus) {
     auto corrPad = corrCanv->GetPad((sdmeModu.GetKey() + 1) * (pwModus.size() + 1) + 1);
     corrPad->cd();
     // corrPad->DrawFrame(0,0,1,1);
     auto tex = new TLatex(texX, texY, sdmeModu.GetName());
     auto tex2 = new TLatex(texX, tex2Y, sdmeModu.GetName2());
     tex->SetTextSize(texSize);
     tex2->SetTextSize(texSize);
     tex->Draw();
     tex2->Draw();
   }

   // write plots
   corrCanv->SaveAs("sdme.pdf");
   corrCanv->Write();
   for(const auto& [pwKey, sdmeDists] : corrDists) {
     for(const auto& [sdmeKey, corrDist] : sdmeDists) {
       corrDist->Write();
     }
   }

   outfile->Close();

}

