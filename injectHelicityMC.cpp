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
#include "TObjArray.h"

// dispin
#include "Constants.h"
#include "EventTree.h"
#include "Modulation.h"
#include "Binning.h"

TFile * inFile;
TFile * outFile;
TTree * inTr;
TTree * outTr;
TTree * asymTr;
Binning * BS;


int main(int argc, char** argv) {

  // ARGUMENTS ///////////////////////////////////
  TString infileN;
  TString outdirN;
  if(argc<=2) {
    printf("USAGE: %s [outroot file] [output directory]\n",argv[0]);
    exit(0);
  };
  infileN = TString(argv[1]);
  outdirN = TString(argv[2]);
  ///////////////////////////////////////////////


  // OPTIONS /////////////////////////////////////
  const Int_t NumInjections = 73;
  // - settings for lattice estimate of shift on LU amps from nonzero
  //   UU amps; here we control how to read the physics asymmetries
  //
  Bool_t injectLattice = true; // enable lattice estimate
  //
  Float_t latticeMax = 0.3; // maximum amplitude (lattice size)
  Int_t latticePoints = 7; // number of amplitudes (within +-latticeMax)
  //
  const Int_t nUUmods = 3; // number of UU modulations for lattice
  // - setup for injection of measuered physics asym
  BS = new Binning();
  BS->SetScheme(0x34,2); // pairType and ivType from `buildSpinroot.exe -i`
  TString asymTable = "spinroot_final_2/table_42_mlm.dat";
  TString iv0str = "gen_Mh"; // which vars to read
  TString iv1str = "";
  TString iv2str = "";
  ///////////////////////////////////////////////

  // prevent injection overflow
  if(NumInjections > EventTree::NumInjectionsMax) {
    fprintf(stderr,"ERROR: requested more than EventTree::NumInjectionsMax injections\n");
    return 1;
  };

  // open input outroot file
  inFile = new TFile(infileN,"READ");
  inTr = (TTree*) inFile->Get("tree");
  Float_t phiH,phiR,theta;
  Float_t Mh;
  Float_t iv0,iv1,iv2;
  Bool_t eleIsMatch;
  Bool_t hadIsMatch[2];
  inTr->SetBranchAddress("gen_PhiH",&phiH);
  inTr->SetBranchAddress("gen_PhiRp",&phiR);
  inTr->SetBranchAddress("gen_theta",&theta);
  inTr->SetBranchAddress("gen_eleIsMatch",&eleIsMatch);
  inTr->SetBranchAddress("gen_hadIsMatch",hadIsMatch);
  if(!injectLattice) inTr->SetBranchAddress("gen_Mh",&Mh);
  else {
    iv0=iv1=iv2=0;
    if(iv0str!="") inTr->SetBranchAddress(iv0str,&iv0);
    if(iv1str!="") inTr->SetBranchAddress(iv1str,&iv1);
    if(iv2str!="") inTr->SetBranchAddress(iv2str,&iv2);
  };


  // create output outroot file
  TString outfileN = infileN;
  outfileN(TRegexp("^.*/")) = TString(outdirN+"/");
  outfileN(TRegexp("//")) = "/";
  printf("infileN = %s\n",infileN.Data());
  printf("outfileN = %s\n",outfileN.Data());
  outFile = new TFile(outfileN,"RECREATE");


  // clone the tree, and create new helicityMC branch
  outTr = inTr->CloneTree(0);
  Int_t NhelicityMC;
  Int_t helicityMC[NumInjections];
  outTr->Branch("NhelicityMC",&NhelicityMC,"NhelicityMC/I");
  outTr->Branch("helicityMC",helicityMC,"helicityMC[NhelictyMC]/I");


  // instantiate modulations for MC
  enum modEnum {modH,modHR,modR,mod2HR,nMod};
  Modulation * modu[nMod];
  Double_t moduVal[nMod];
  Float_t amp = 0.04; // injected amplitude
  Float_t asymInject[NumInjections];
  Float_t numerInject,denomInject,denomInject2;
  TRandom * RNG;
  Float_t rn;
  modu[modH] =   new Modulation(3,0,0,0,false,Modulation::kLU);
  modu[modHR] =  new Modulation(2,1,1,0,false,Modulation::kLU);
  modu[modR] =   new Modulation(3,1,1,0,false,Modulation::kLU);
  modu[mod2HR] = new Modulation(3,1,-1,0,false,Modulation::kLU);
  RNG = new TRandomMixMax(0); // seed
  Int_t ii;
  Bool_t once = true;
  Bool_t success;


  // lattice estimate: read asymmetry measurement data
  Int_t binnum,ampnum;
  Float_t asymval;
  Char_t binnumStr[32];
  Int_t binnumBS;
  Float_t st,lb,ub;
  Modulation * moduLatLU[7];
  Modulation * moduLatUU[nUUmods];
  TObjArray * moduUUarr;
  Int_t levMax;
  Float_t bb[nUUmods];
  Bool_t enableLegendre = false; // theta-dependence for moduUUarr
  if(injectLattice) {
    asymTr = new TTree();
    asymTr->ReadFile(asymTable,"binnumStr/C:ampnum/I:kinval/F:asymval/F:asymerr/F:dpmean/F");
    asymTr->SetBranchAddress("binnumStr",binnumStr);
    asymTr->SetBranchAddress("ampnum",&ampnum);
    asymTr->SetBranchAddress("asymval",&asymval);
    // LU modulations
    moduLatLU[0] = new Modulation(3,0,0,0,false,Modulation::kLU);
    moduLatLU[1] = new Modulation(2,1,1,0,false,Modulation::kLU);
    moduLatLU[2] = new Modulation(3,1,1,0,false,Modulation::kLU);
    moduLatLU[3] = new Modulation(3,1,-1,0,false,Modulation::kLU);
    moduLatLU[4] = new Modulation(2,2,2,0,false,Modulation::kLU);
    moduLatLU[5] = new Modulation(3,2,2,0,false,Modulation::kLU);
    moduLatLU[6] = new Modulation(3,2,-2,0,false,Modulation::kLU);
    // UU modulations
    moduLatUU[0] = new Modulation(3,0,0,0,false,Modulation::kUU); // cosPhiH
    moduLatUU[1] = new Modulation(2,1,1,0,false,Modulation::kUU); // cos(PhiH-PhiR)
    moduLatUU[2] = new Modulation(3,1,1,0,false,Modulation::kUU); // cosPhiR
    // other
    st = 2*latticeMax/(latticePoints-1); // step size
    lb = -latticeMax; // minimum UU amp
    ub = latticeMax + 1e-5; // maximum UU amp (+small offset for float loop ranges)
    // moduUUarr: list of all UU modulations (used for 1D lattices)
    moduUUarr = new TObjArray();
    for(int l=0; l<=2; l++) {
      for(int m=0; m<=l; m++) {
        for(int twist=2; twist<=3; twist++) {
          levMax = twist==2 ? 1:0;
          for(int lev=0; lev<=levMax; lev++) {
            if(!enableLegendre && l<2) continue;
            if(twist==2 && lev==0 && m==0) continue; // skip constant term
            if((twist==2 && lev==0 && m>=0) || (twist==2 && lev==1) || twist==3) {
              moduUUarr->AddLast(new Modulation(twist,l,m,lev,enableLegendre,Modulation::kUU));
              if(((twist==2 && lev==1) || twist==3) && m>0) { // negative m states
                moduUUarr->AddLast(new Modulation(twist,l,-m,lev,enableLegendre,Modulation::kUU));
              };
            };
          };
        };
      };
    };
  };




  // event loop
  Long64_t ENT = inTr->GetEntries();
  //ENT = 1000; // limiter
  for(Long64_t i=0; i<ENT; i++) {
    if(i%10000==0) printf("[+] %.2f%%\n",100*(float)i/((float)ENT));
    inTr->GetEntry(i);
    for(int h=0; h<NumInjections; h++) helicityMC[h]=0;


    // lattice estimate
    if(injectLattice) {
      
      // get physics asymmetry
      if(iv0>UNDEF && iv1>UNDEF && iv2>UNDEF) {
        binnumBS = BS->FindBin(iv0,iv1,iv2);
        numerInject = 0;
        for(int r=0; r<asymTr->GetEntries(); r++) {
          asymTr->GetEntry(r);
          sscanf(binnumStr,"%x",&binnum);
          //printf("binnumStr=%s binnum=%d\n",binnumStr,binnum);
          if(binnum==binnumBS) {
            numerInject += asymval * moduLatLU[ampnum]->Evaluate(phiR,phiH,theta);
          };
        };

        /*
        // construct the lattice
        // - loop over points with a cartesian product
        // - calculate denominator
        // - calculate full asymmetry to inject
        ii=0;
        for(bb[0]=lb; bb[0]<ub; bb[0]+=st) {
          for(bb[1]=lb; bb[1]<ub; bb[1]+=st) {
            for(bb[2]=lb; bb[2]<ub; bb[2]+=st) {
              // denom = 1 + sum_i{B_i*g_i(phi)}
              denomInject = 1;
              for(int u=0; u<nUUmods; u++) {
                denomInject += bb[u] * moduLatLU[u]->Evaluate(phiR,phiH,theta);
              }
              // asymmetry to inject
              asymInject[ii] = numerInject / denomInject;
              // print out UU amplitude values
              if(once) {
                printf("ii B_i: %d",ii);
                for(int u=0; u<nUUmods; u++) printf(" %f",bb[u]);
                printf("\n");
              };
              ii++;
            };
          };
        };
        once = false;
        // number of injections = (num UU amps)^(num UU modulations) 
        // = latticePoints^nUUmods
        // = 27   // must match EventTree::NhelicityMC
        */

        // 1D lattice: one lattice for each UU modulation in moduUUarr
        ii=0;
        asymInject[ii] = numerInject;
        if(once) printf("ii moduUU B: %d 0 0\n",ii);
        ii++;
        TObjArrayIter nxt(moduUUarr);
        while(Modulation * moduUU = (Modulation*)nxt()) {
          for(float bbb=lb; bbb<ub; bbb+=st) {
            if(fabs(bbb)<1e-4) continue; // skip B=0 cases
            denomInject = 1 + bbb * moduUU->Evaluate(phiR,phiH,theta);
            asymInject[ii] = numerInject / denomInject;
            if(once) {
              printf("ii moduUU B: %d %s %f\n",
                ii,(moduUU->ModulationTitle()).Data(),bbb);
            };
            ii++;
          };
        };
        once = false;
        // number of injections: 73

        success = true;
      }
      else {
        success = false;
      };
    } 
    
    else {
      // sandbox for injection studies     


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

      // - test sp and pp linear independence for m=1 modulations
      // -- A_sp = `amp` (coefficient of P_11=sin(theta))
      // -- A_pp is varied (coefficient of P_21=sin(2theta))
      // --- twist 2:
      asymInject[6] =  ( amp*TMath::Sin(theta) + 0.04*TMath::Sin(2*theta) ) * moduVal[modHR];
      asymInject[7] =  ( amp*TMath::Sin(theta) + 0.02*TMath::Sin(2*theta) ) * moduVal[modHR];
      asymInject[8] =  ( amp*TMath::Sin(theta) ) * moduVal[modHR];
      asymInject[9] =  ( amp*TMath::Sin(theta) - 0.02*TMath::Sin(2*theta) ) * moduVal[modHR];
      asymInject[10] = ( amp*TMath::Sin(theta) - 0.04*TMath::Sin(2*theta) ) * moduVal[modHR];
      // --- twist 3:
      asymInject[11] = ( amp*TMath::Sin(theta) + 0.04*TMath::Sin(2*theta) ) * moduVal[modR];
      asymInject[12] = ( amp*TMath::Sin(theta) + 0.02*TMath::Sin(2*theta) ) * moduVal[modR];
      asymInject[13] = ( amp*TMath::Sin(theta) ) * moduVal[modR];
      asymInject[14] = ( amp*TMath::Sin(theta) - 0.02*TMath::Sin(2*theta) ) * moduVal[modR];
      asymInject[15] = ( amp*TMath::Sin(theta) - 0.04*TMath::Sin(2*theta) ) * moduVal[modR];
      // number of injections: 16
      




      /*
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
      */

      // ------------------------------------------theta
      /***/
      numerInject = moduVal[modR];
      //numerInject = 0.06*moduVal[modH]+0.08*moduVal[modHR];
      /***/
      //denomInject = 0.5*(3*TMath::Power(TMath::Cos(theta),2)-1);
      //denomInject = TMath::Sin(theta);
      //denomInject = TMath::Cos(phiH);
      denomInject = TMath::Cos(phiH+phiR);
      /***/
      //denomInject2 = TMath::Sin(theta);
      denomInject2 = 0;
      /***/


      // generalized injection over grid of (A,B)
      /*
      ii=27;
      for(float AA=-0.21; AA<=0.21; AA+=0.03) {
        for(float BB=-0.5; BB<=0.5; BB+=0.1) {
          asymInject[ii] = AA*numerInject / (1+BB*denomInject);
          if(once) printf("ii AA BB: %d %f %f\n",ii,AA,BB);
          ii++;
        };
      };
      once = false;
      // max ii=180
      */

      // smaller grid
      /*
      ii=27;
      for(float AA=-0.04; AA<=0.041; AA+=0.04) {
        for(float BB=-0.4; BB<=0.41; BB+=0.2) {
          asymInject[ii] = AA*numerInject / (1+BB*denomInject);
          if(once) printf("ii AA BB: %d %f %f\n",ii,AA,BB);
          ii++;
        };
      };
      once = false;
      */
      // max ii=41



      // --- effect on numerators
      /*
      asymInject[27] = asymInject[1] / (1+0.2*denomInject+0.2*denomInject2);
      asymInject[28] = asymInject[2] / (1+0.2*denomInject+0.2*denomInject2);
      asymInject[29] = asymInject[3] / (1+0.2*denomInject+0.2*denomInject2);
      asymInject[30] = asymInject[4] / (1+0.2*denomInject+0.2*denomInject2);


      // fix B=0.2, vary A
      // 0.04
      asymInject[31] = 1*numerInject;
      asymInject[32] = 1*numerInject / (1+0.2*denomInject+0.2*denomInject2);
      // 0.08
      asymInject[33] = 2*numerInject;
      asymInject[34] = 2*numerInject / (1+0.2*denomInject+0.2*denomInject2);
      // 0.16
      asymInject[35] = 4*numerInject;
      asymInject[36] = 4*numerInject / (1+0.2*denomInject+0.2*denomInject2);
      // 0.32
      asymInject[37] = 8*numerInject;
      asymInject[38] = 8*numerInject / (1+0.2*denomInject+0.2*denomInject2);
      // -0.04
      asymInject[39] = -1*numerInject;
      asymInject[40] = -1*numerInject / (1+0.2*denomInject+0.2*denomInject2);
      // -0.08
      asymInject[41] = -2*numerInject;
      asymInject[42] = -2*numerInject / (1+0.2*denomInject+0.2*denomInject2);
      // -0.16
      asymInject[43] = -4*numerInject;
      asymInject[44] = -4*numerInject / (1+0.2*denomInject+0.2*denomInject2);
      // -0.32
      asymInject[45] = -8*numerInject;
      asymInject[46] = -8*numerInject / (1+0.2*denomInject+0.2*denomInject2);

      // fix A = amp, vary B
      asymInject[47] = numerInject / 
                       (1+0.1*denomInject+0.2*denomInject2);
      asymInject[48] = numerInject /
                       (1+0.2*denomInject+0.2*denomInject2);
      asymInject[49] = numerInject /
                       (1+0.4*denomInject+0.2*denomInject2);
      asymInject[50] = numerInject /
                       (1+0.8*denomInject+0.2*denomInject2);
      asymInject[51] = numerInject /
                       (1-0.1*denomInject+0.2*denomInject2);
      asymInject[52] = numerInject /
                       (1-0.2*denomInject+0.2*denomInject2);
      asymInject[53] = numerInject /
                       (1-0.4*denomInject+0.2*denomInject2);
      asymInject[54] = numerInject /
                       (1-0.8*denomInject+0.2*denomInject2);

      // fix A = 4*amp, vary B
      asymInject[55] = 4*numerInject / 
                       (1+0.1*denomInject+0.2*denomInject2);
      asymInject[56] = 4*numerInject /
                       (1+0.2*denomInject+0.2*denomInject2);
      asymInject[57] = 4*numerInject /
                       (1+0.4*denomInject+0.2*denomInject2);
      asymInject[58] = 4*numerInject /
                       (1+0.8*denomInject+0.2*denomInject2);
      asymInject[59] = 4*numerInject /
                       (1-0.1*denomInject+0.2*denomInject2);
      asymInject[60] = 4*numerInject /
                       (1-0.2*denomInject+0.2*denomInject2);
      asymInject[61] = 4*numerInject /
                       (1-0.4*denomInject+0.2*denomInject2);
      asymInject[62] = 4*numerInject /
                       (1-0.8*denomInject+0.2*denomInject2);
      */

      success = true;
    };


    // if matching criteria not satisfied, set helicity
    // as undefined 
    if(!eleIsMatch || !hadIsMatch[0] || !hadIsMatch[1]) {
      success = false;
    };


    // calculate injected helicity: 2=spin-, 3=spin+
    rn = RNG->Uniform(); // generate random number within [0,1]
    for(int f=0; f<NumInjections; f++) {
      asymInject[f] *= 0.863; // polarization (cf EventTree::Polarization())
      if(success)
        helicityMC[f] = (rn<0.5*(1+asymInject[f])) ? 3 : 2;
      else
        helicityMC[f] = 0; // undefined
    };

    outTr->Fill();
  };


  // write and close
  outTr->AutoSave();
  inFile->Close();
  outFile->Close();
};
