// reads outroot files (which must have MC data), and injects asymmetries
// by assigning helicities

#include <cstdlib>
#include <iostream>

// ROOT
#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TMath.h"
#include "TSystem.h"
#include "TRandom.h"
#include "TRandomGen.h"
#include "TRegexp.h"

// dispin
#include "Constants.h"
#include "EventTree.h"
#include "Modulation.h"

TFile * inFile;
TFile * outFile;
TTree * inTr;
TTree * outTr;


int main(int argc, char** argv) {

  // ARGUMENTS
  TString infileN;
  TString outdirN;
  if(argc<=2) {
    printf("USAGE: %s [outroot file] [output directory]\n",argv[0]);
    exit(0);
  };
  infileN = TString(argv[1]);
  outdirN = TString(argv[2]);

  // open input outroot file
  inFile = new TFile(infileN,"READ");
  inTr = (TTree*) inFile->Get("tree");
  Float_t phiH,phiR,theta;
  Float_t Mh;
  inTr->SetBranchAddress("gen_PhiH",&phiH);
  inTr->SetBranchAddress("gen_PhiRp",&phiR);
  inTr->SetBranchAddress("gen_theta",&theta);
  inTr->SetBranchAddress("gen_Mh",&Mh);


  // create output outroot file
  TString outfileN = infileN;
  outfileN(TRegexp("^.*/")) = TString(outdirN+"/");
  outfileN(TRegexp("//")) = "/";
  printf("infileN = %s\n",infileN.Data());
  printf("outfileN = %s\n",outfileN.Data());
  outFile = new TFile(outfileN,"RECREATE");


  // clone the tree, and create new helicityMC branch
  outTr = inTr->CloneTree(0);
  Int_t helicityMC[EventTree::NhelicityMC];
  TString brN = "helicityMC2"; // for now, append "2" to the new branch,
                               // since it's difficult to replace old branch
  TString brF = Form("%s[%d]/I",brN.Data(),EventTree::NhelicityMC);
  outTr->Branch(brN,helicityMC,brF);


  // instantiate modulations for MC
  enum modEnum {modH,modHR,modR,mod2HR,nMod};
  Modulation * modu[nMod];
  Double_t moduVal[nMod];
  Float_t amp = 0.04; // injected amplitude
  Float_t asymInject[EventTree::NhelicityMC];
  Float_t denomInject;
  TRandom * RNG;
  Float_t rn;
  modu[modH] =   new Modulation(3,0,0,0,false,Modulation::kLU);
  modu[modHR] =  new Modulation(2,1,1,0,false,Modulation::kLU);
  modu[modR] =   new Modulation(3,1,1,0,false,Modulation::kLU);
  modu[mod2HR] = new Modulation(3,1,-1,0,false,Modulation::kLU);
  RNG = new TRandomMixMax(14972); // seed



  // event loop
  Long64_t ENT = inTr->GetEntries();
  //ENT = 100; // limiter
  for(Long64_t i=0; i<ENT; i++) {
    if(i%10000==0) printf("[+] %.2f%%\n",100*(float)i/((float)ENT));
    inTr->GetEntry(i);
    for(int h=0; h<EventTree::NhelicityMC; h++) helicityMC[h]=0;


    // calculate modulations, generated values of phi and theta
    for(int m=0; m<nMod; m++) {
      moduVal[m] = modu[m]->Evaluate(phiR,phiH,theta);
    };


    // - constant asymmetry
    asymInject[0]=0;
    asymInject[1] = amp*moduVal[modH];
    asymInject[2] = amp*moduVal[modHR];
    asymInject[3] = amp*moduVal[modR];
    asymInject[4] = amp*moduVal[mod2HR];
    // - mimic physics Mh dependence
    asymInject[5] = amp*Mh/(2*0.77) * moduVal[modR]; // linear, pos. slope
    asymInject[5] += ( amp - amp*Mh/(2*0.77) ) * moduVal[modH]; // linear, neg. slope
    asymInject[5] += amp * TMath::Sin(PI*Mh/0.77) * moduVal[modHR]; // sign change
    // - test inclusion of F_UU modulations
    // --- effect on mimic
    asymInject[6] = asymInject[5] / (1+0.2*TMath::Cos(phiH));
    asymInject[7] = asymInject[5] / (1+0.2*TMath::Cos(phiR));
    asymInject[8] = asymInject[5] / (1+0.2*TMath::Cos(phiH-phiR));
    asymInject[9] = asymInject[5] / (1+0.2*TMath::Cos(phiH)+0.2*TMath::Cos(phiR)+0.2*TMath::Cos(phiH-phiR));
    // --- effect on 100% asym
    asymInject[10] = 1 / (1+0.2*TMath::Cos(phiH)); // 100% asym
    // --- effect on single asym amps, 20% cosPhiH
    asymInject[11] = asymInject[1] / (1+0.2*TMath::Cos(phiH));
    asymInject[12] = asymInject[2] / (1+0.2*TMath::Cos(phiH));
    asymInject[13] = asymInject[3] / (1+0.2*TMath::Cos(phiH));
    asymInject[14] = asymInject[4] / (1+0.2*TMath::Cos(phiH));
    // --- 10%
    asymInject[15] = asymInject[1] / (1+0.1*TMath::Cos(phiH));
    asymInject[16] = asymInject[2] / (1+0.1*TMath::Cos(phiH));
    asymInject[17] = asymInject[3] / (1+0.1*TMath::Cos(phiH));
    asymInject[18] = asymInject[4] / (1+0.1*TMath::Cos(phiH));
    // --- 40%
    asymInject[19] = asymInject[1] / (1+0.4*TMath::Cos(phiH));
    asymInject[20] = asymInject[2] / (1+0.4*TMath::Cos(phiH));
    asymInject[21] = asymInject[3] / (1+0.4*TMath::Cos(phiH));
    asymInject[22] = asymInject[4] / (1+0.4*TMath::Cos(phiH));
    // --- -10%
    asymInject[23] = asymInject[1] / (1-0.1*TMath::Cos(phiH));
    asymInject[24] = asymInject[2] / (1-0.1*TMath::Cos(phiH));
    asymInject[25] = asymInject[3] / (1-0.1*TMath::Cos(phiH));
    asymInject[26] = asymInject[4] / (1-0.1*TMath::Cos(phiH));

    // ------------------------------------------theta
    //denomInject = 0.5*(3*TMath::Power(TMath::Cos(theta),2)-1);
    denomInject = TMath::Sin(theta);

    // --- effect on numerators
    asymInject[27] = asymInject[1] / (1+0.2*denomInject);
    asymInject[28] = asymInject[2] / (1+0.2*denomInject);
    asymInject[29] = asymInject[3] / (1+0.2*denomInject);
    asymInject[30] = asymInject[4] / (1+0.2*denomInject);


    // fix B=0.2, vary A
    // 0.04
    asymInject[31] = 1*asymInject[1];
    asymInject[32] = 1*asymInject[1] / (1+0.2*denomInject);
    // 0.08
    asymInject[33] = 2*asymInject[1];
    asymInject[34] = 2*asymInject[1] / (1+0.2*denomInject);
    // 0.16
    asymInject[35] = 4*asymInject[1];
    asymInject[36] = 4*asymInject[1] / (1+0.2*denomInject);
    // 0.32
    asymInject[37] = 8*asymInject[1];
    asymInject[38] = 8*asymInject[1] / (1+0.2*denomInject);
    // -0.04
    asymInject[39] = -1*asymInject[1];
    asymInject[40] = -1*asymInject[1] / (1+0.2*denomInject);
    // -0.08
    asymInject[41] = -2*asymInject[1];
    asymInject[42] = -2*asymInject[1] / (1+0.2*denomInject);
    // -0.16
    asymInject[43] = -4*asymInject[1];
    asymInject[44] = -4*asymInject[1] / (1+0.2*denomInject);
    // -0.32
    asymInject[45] = -8*asymInject[1];
    asymInject[46] = -8*asymInject[1] / (1+0.2*denomInject);

    // fix A = amp, vary B
    asymInject[47] = asymInject[1] / 
                     (1+0.1*denomInject);
    asymInject[48] = asymInject[1] /
                     (1+0.2*denomInject);
    asymInject[49] = asymInject[1] /
                     (1+0.4*denomInject);
    asymInject[50] = asymInject[1] /
                     (1+0.8*denomInject);
    asymInject[51] = asymInject[1] /
                     (1-0.1*denomInject);
    asymInject[52] = asymInject[1] /
                     (1-0.2*denomInject);
    asymInject[53] = asymInject[1] /
                     (1-0.4*denomInject);
    asymInject[54] = asymInject[1] /
                     (1-0.8*denomInject);

    // fix A = 4*amp, vary B
    asymInject[55] = 4*asymInject[1] / 
                     (1+0.1*denomInject);
    asymInject[56] = 4*asymInject[1] /
                     (1+0.2*denomInject);
    asymInject[57] = 4*asymInject[1] /
                     (1+0.4*denomInject);
    asymInject[58] = 4*asymInject[1] /
                     (1+0.8*denomInject);
    asymInject[59] = 4*asymInject[1] /
                     (1-0.1*denomInject);
    asymInject[60] = 4*asymInject[1] /
                     (1-0.2*denomInject);
    asymInject[61] = 4*asymInject[1] /
                     (1-0.4*denomInject);
    asymInject[62] = 4*asymInject[1] /
                     (1-0.8*denomInject);


    // calculate injected helicity: 2=spin-, 3=spin+
    rn = RNG->Uniform(); // generate random number within [0,1]
    for(int f=0; f<EventTree::NhelicityMC; f++) {
      asymInject[f] *= 0.863; // polarization (cf EventTree::Polarization())
      helicityMC[f] = (rn<0.5*(1+asymInject[f])) ? 3 : 2;
    };

    outTr->Fill();
  };


  // write and close
  outTr->AutoSave();
  inFile->Close();
  outFile->Close();
};
