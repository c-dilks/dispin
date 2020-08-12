#include <cstdlib>
#include <iostream>

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
#include "TGraph.h"

// DihBsa
#include "Constants.h"
#include "Tools.h"
#include "DIS.h"
#include "Trajectory.h"
#include "Dihadron.h"
#include "EventTree.h"

void HadronCompareCanv(TCanvas * canv, TH1D * dist[2], TH2D * corr);
void Hadron2dCanv(TCanvas * canv, TH2D * distA, TH2D * distB);
TString corrTitle(TString var);
TString distTitle(TString var);
TString dist2Title(TString hadron, TString varX,TString varY);

TString infiles;
Int_t whichPair;
Int_t whichHad[2];
TString hadName[2];
TString hadTitle[2];

int main(int argc, char** argv) {

   // ARGUMENTS
   infiles = "outroot/*.root";
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


   TFile * outfile = new TFile("plots.root","RECREATE");

   const Int_t NBINS = 150;
   Float_t deltaPhi;


   // DIS kinematics
   TH1D * WDist = new TH1D("WDist","W distribution (w/o W cut);W",NBINS,0,6);
   TH1D * XDist = new TH1D("XDist","x distribution;x",NBINS,0,1);
   TH1D * Q2Dist = new TH1D("Q2Dist","Q^{2} distribution;Q^{2}",NBINS,0,12);
   TH2D * Q2vsW = new TH2D("Q2vsW","Q^{2} vs. W (w/o W cut);W;Q^{2}",
                                   NBINS,0,6,NBINS,0,12);
   TH2D * Q2vsX = new TH2D("Q2vsX","Q^{2} vs. x;x;Q^{2}",NBINS,0,1,NBINS,0,12);
   TH1D * YDist = new TH1D("YDist","y distribution (w/o y cut);y",NBINS,0,1);
   
   // electron kinematics
   TH1D * eleEDist = new TH1D("eleEDist","e^{-} E distribution",NBINS,0,12);
   TH1D * elePtDist = new TH1D("elePtDist","e^{-} p_{T} distribution",NBINS,0,4);
   TH1D * eleEtaDist = new TH1D("eleEtaDist","e^{-} #eta distribution",NBINS,-3,6);
   TH1D * elePhiDist = new TH1D("elePhiDist","e^{-} #phi distribution",NBINS,-PIe,PIe);
   TH2D * eleEtaVsPhi = new TH2D("eleEtavsPhi","e^{-} #eta vs #phi;#phi;#eta",
     NBINS,-PIe,PIe,NBINS,-3,6);
   TH2D * eleEVsPhi = new TH2D("eleEvsPhi","e^{-} E vs #phi;#phi;E",
     NBINS,-PIe,PIe,NBINS,0,12);
   TH2D * elePtVsPhi = new TH2D("elePtvsPhi","e^{-} p_{T} vs #phi;#phi;#p_{T}",
     NBINS,-PIe,PIe,NBINS,0,4);
   TH1D * eleVzDist = new TH1D("eleVzDist","e^{-} V_{z} distribution",
     NBINS,-20,20);
   TH2D * eleVxyDist = new TH2D("eleVxyDist","e^{-} V_{y} vs. V_{x}",
     3*NBINS,-6,6,3*NBINS,-6,6);

   // dihadron's hadron kinematic correlations
   TH2D * hadECorr = new TH2D("hadECorr",corrTitle("E"),NBINS,0,10,NBINS,0,10);
   TH2D * hadPCorr = new TH2D("hadPCorr",corrTitle("p"),NBINS,0,10,NBINS,0,10);
   TH2D * hadPtCorr = new TH2D("hadPtCorr",corrTitle("p_{T}"),NBINS,0,4,NBINS,0,4);
   TH2D * hadEtaCorr = new TH2D("hadEtaCorr",corrTitle("#eta"),NBINS,0,5,NBINS,0,5);
   TH2D * hadPhiCorr = new TH2D("hadPhiCorr",corrTitle("#phi"),
                                             NBINS,-PIe,PIe,NBINS,-PIe,PIe);
   TH2D * hadZCorr = new TH2D("hadZCorr",corrTitle("z"),NBINS,0,1,NBINS,0,1);
   TH2D * hadXFCorr = new TH2D("hadXFCorr",corrTitle("x_{F}"),
     NBINS,-1,1,NBINS,-1,1);
   TH2D * hadVzCorr = new TH2D("hadVzCorr",corrTitle("V_{z}"),
     NBINS,-20,20,NBINS,-20,20);
   
   // dihadron's hadron kinematics
   TH1D * hadEDist[2];
   TH1D * hadPDist[2];
   TH1D * hadPtDist[2];
   TH1D * hadEtaDist[2];
   TH1D * hadPhiDist[2];
   TH1D * hadZDist[2];
   TH1D * hadXFDist[2];
   TH1D * hadVzDist[2];
   TH2D * hadVxyDist[2];
   TH2D * hadEtaVsPhi[2];
   TH2D * hadEVsPhi[2];
   TH2D * hadPtVsPhi[2];
   for(int h=0; h<2; h++) {
     hadEDist[h] = new TH1D(TString(hadName[h]+"hadEDist"),distTitle("E"),
       NBINS,0,10);
     hadPDist[h] = new TH1D(TString(hadName[h]+"hadPDist"),distTitle("p"),
       NBINS,0,10);
     hadPtDist[h] = new TH1D(TString(hadName[h]+"hadPtDist"),distTitle("p_{T}"),
       NBINS,0,4);
     hadEtaDist[h] = new TH1D(TString(hadName[h]+"hadEtaDist"),distTitle("#eta"),
       NBINS,0,5);
     hadPhiDist[h] = new TH1D(TString(hadName[h]+"hadPhiDist"),distTitle("#phi"),
       NBINS,-PIe,PIe);
     hadZDist[h] = new TH1D(TString(hadName[h]+"hadZDist"),distTitle("z"),
       NBINS,0,1);
     hadXFDist[h] = new TH1D(TString(hadName[h]+"hadXFDist"),distTitle("x_{F}"),
       NBINS,-1,1);
     hadVzDist[h] = new TH1D(TString(hadName[h]+"hadVzDist"),
       distTitle("V_{z}"),
       NBINS,-20,20);
     hadVxyDist[h] = new TH2D(TString("hadVxyDist_"+hadName[h]),
       TString(hadTitle[h]+" V_{y} vs. V_{x}"),
       3*NBINS,-6,6,3*NBINS,-6,6);

     hadEtaVsPhi[h] = new TH2D(
       TString(hadName[h]+"hadEtaVsPhi"),dist2Title(hadTitle[h],"#phi","#eta"),
       NBINS,-PIe,PIe,NBINS,0,5);
     hadEVsPhi[h] = new TH2D(
       TString(hadName[h]+"hadEVsPhi"),dist2Title(hadTitle[h],"#phi","E"),
       NBINS,-PIe,PIe,NBINS,0,10);
     hadPtVsPhi[h] = new TH2D(
       TString(hadName[h]+"hadPtVsPhi"),dist2Title(hadTitle[h],"#phi","p_{T}"),
       NBINS,-PIe,PIe,NBINS,0,4);
   };


   // dihadron kinematics
   TString plotTitle = "#Delta#phi = #phi(" + hadTitle[qA] + ")" +
                                 " - #phi(" + hadTitle[qB] + 
                                 ") distribution;#Delta#phi";
   TH1D * deltaPhiDist = new TH1D("deltaPhiDist",plotTitle,NBINS,-PIe,PIe);

   TH1D * MhDist = new TH1D("MhDist","M_{h} distribution;M_{h}",2*NBINS,0,3);
   TH1D * PhDist = new TH1D("PhDist","|P_{h}| distribution;|P_{h}|",NBINS,0,10);
   TH1D * PhPerpDist = new TH1D("PhPerpDist","|P_{h}^{perp}| distribution;|P_{h}^{perp}|",
     NBINS,0,2);
   TH1D * ZpairDist = new TH1D("ZpairDist","z_{pair} distribution;z_{pair}",NBINS,0,1);
   TH1D * zetaDist = new TH1D("zetaDist","#zeta distribution;#zeta",NBINS,-1,1);
   TH1D * xFDist = new TH1D("xFDist","x_{F} distribution;x_{F}",NBINS,-2,2);
   TH1D * MmissDist = new TH1D("MmissDist","M_{X} distribution;M_{X}",NBINS,-2,6);

   TH1D * MmissDistZoom = new TH1D("MmissDistZoom","M_{X} distribution;M_{X}",
     2*NBINS,0.5,3);
   TH2D * MmissVsMh = new TH2D("MmissVsMh","M_{X} vs. M_{h};M_{h};M_{X}",
     NBINS,0,2.5,
     NBINS,0.5,3);
   
   TH1D * PhiHDist = new TH1D("PhiHDist","#phi_{h} distribution;#phi_{h}",
     NBINS,-PIe,PIe);
   TH1D * PhiRDist = new TH1D("PhiRDist","#phi_{R} distribution;#phi_{R}",
     NBINS,-PIe,PIe);
   TH2D * PhiHvsPhiR = new TH2D("PhiHvsPhiR","#phi_{h} vs. #phi_{R};#phi_{R};#phi_{h}",
     NBINS,-PIe,PIe,NBINS,-PIe,PIe);
   TH1D * PhiHRDist = new TH1D("PhiHRDist",
     "#phi_{h}-#phi_{R} distribution;#phi_{h}-#phi_{R}",
     NBINS,-PIe,PIe);

   plotTitle = "P_{h}^{perp}/M_{h} vs. sin(#phi_{h}-#phi_{R});";
   plotTitle += "sin(#phi_{h}-#phi_{R});P_{h}^{perp}/M_{h}";
   TH2D * g1perpWeightVsMod = new TH2D("g1perpWeightVsMod",plotTitle,
     NBINS,-1.1,1.1,
     NBINS,0,6);
   TH2D * PhPerpVsMh = new TH2D("PhPerpVsMh",
     "P_{h}^{perp} vs. M_{h};M_{h};P_{h}^{perp}",
     NBINS,0,3,
     NBINS,0,3);
   TH2D * RTvsMh = new TH2D("RTvsMh",
     "R_{T} vs. M_{h};M_{h};R_{T}",
     NBINS,0,3,
     NBINS,0,1.5);
   TH2D * xFvsZ[2];
   for(int h=0; h<2; h++) {
     xFvsZ[h] = new TH2D(TString("xFvsZ_"+hadName[h]),
       TString(hadTitle[h]+"x_{F} vs. z;z;x_{F}"),
       NBINS,0,1,NBINS,-1,1);
   };

   // distributions for partial wave analysis
   TH1D * thetaDist = new TH1D("thetaDist","#theta distribution;#theta",NBINS,0,PI);

   TH2D * thetaVsPhiH = new TH2D("thetaVsPhiH",
     "#theta vs #phi_{h};#phi_{h};#theta",
     NBINS,-PIe,PIe,NBINS,0,PIe);
   TH2D * thetaVsPhiR = new TH2D("thetaVsPhiR",
     "#theta vs #phi_{R};#phi_{R};#theta",
     NBINS,-PIe,PIe,NBINS,0,PIe);
   TH2D * thetaVsPhiHR = new TH2D("thetaVsPhiHR",
     "#theta vs #phi_{h}-#phi_{R};#phi_{h}-#phi_{R};#theta",
     NBINS,-PIe,PIe,NBINS,0,PIe);

   TH2D * thetaVsMh = new TH2D("thetaVsMh","#theta vs. M_{h};M_{h};#theta",
     NBINS,0,3,NBINS,0,PIe);
   TH2D * thetaVsZpair = new TH2D("thetaVsZpair","#theta vs. z;z;#theta",
     NBINS,0,1,NBINS,0,PIe);
   TH2D * thetaVsZeta = new TH2D("thetaVsZeta","#theta vs. #zeta;#zeta;#theta",
     NBINS,-1,1,NBINS,0,PIe);
   TH2D * thetaVsX = new TH2D("thetaVsX","#theta vs. x;x;#theta",
     NBINS,0,1,NBINS,0,PIe);
   TH2D * thetaVsPh = new TH2D("thetaVsPh","#theta vs. p;p;#theta",
     NBINS,0,10,NBINS,0,PIe);

   TH2D * thetaVsZ[2];
   TH2D * thetaVsHadP[2];
   TH2D * YHvsXF[2];
   for(int h=0; h<2; h++) {
     thetaVsZ[h] = new TH2D(TString("thetaVsZ_"+hadName[h]),
       TString("#theta vs. "+hadTitle[h]+" z;z;#theta"),
       NBINS,0,1,NBINS,0,PIe);
     thetaVsHadP[h] = new TH2D(TString("thetaVsHadP_"+hadName[h]),
       TString("#theta vs. "+hadTitle[h]+" p;p;#theta"),
       NBINS,0,10,NBINS,0,PIe);
     YHvsXF[h] = new TH2D(TString("YHvsXF_"+hadName[h]),
       TString(hadTitle[h]+" y_{h} vs. x_{F};x_{F};y_{h}"),
       NBINS,-1,1,NBINS,-4,4);
   };
   
   TH1D * sinThetaDist = new TH1D("sinThetaDist",
     "sin(#theta) distribution;sin(#theta)",NBINS,-1.1,1.1);
   TH1D * sinThetaCosThetaDist = new TH1D("sinThetaCosThetaDist",
     "sin(#theta)cos(#theta) distribution;sin(#theta)cos(#theta)",NBINS,-1.1,1.1);

   TH1D * cosThetaDist = new TH1D("cosThetaDist",
     "cos(#theta) distribution;cos(#theta)",NBINS,-1.1,1.1);


   // PhiH and PhiR vs. other variables
   TH2D * PhiHvsMh = new TH2D("PhiHvsMh","#phi_{h} vs. M_{h};M_{h};#phi_{h}",
     NBINS,0,3,NBINS,-PIe,PIe);
   TH2D * PhiHvsZ = new TH2D("PhiHvsZ","#phi_{h} vs. z;z;#phi_{h}",
     NBINS,0,1,NBINS,-PIe,PIe);
   TH2D * PhiHvsX = new TH2D("PhiHvsX","#phi_{h} vs. x;x;#phi_{h}",
     NBINS,0,1,NBINS,-PIe,PIe);

   TH2D * PhiRvsMh = new TH2D("PhiRvsMh","#phi_{R} vs. M_{h};M_{h};#phi_{R}",
     NBINS,0,3,NBINS,-PIe,PIe);
   TH2D * PhiRvsZ = new TH2D("PhiRvsZ","#phi_{R} vs. z;z;#phi_{R}",
     NBINS,0,1,NBINS,-PIe,PIe);
   TH2D * PhiRvsX = new TH2D("PhiRvsX","#phi_{R} vs. x;x;#phi_{R}",
     NBINS,0,1,NBINS,-PIe,PIe);

   TH2D * PhiRvsAlpha = new TH2D("PhiRvsAlpha",
     "#phi_{R} vs. #alpha;#alpha;#phi_{R}",
     NBINS,0,1.3,NBINS,-PIe,PIe);
   TH2D * PhiHRvsAlpha = new TH2D("PhiHRvsAlpha",
     "#phi_{h}-#phi_{R} vs. #alpha;#alpha;#phi_{h}-#phi_{R}",
     NBINS,0,1.3,NBINS,-PIe,PIe);
   TH1D * vzDiffHadHad = new TH1D("vzDiffHadHad",
     TString("V_{z}("+hadTitle[qA]+")-V_{z}("+hadTitle[qB]+") distribution"),
     NBINS,-20,20);
   TH2D * vzDiffEleHad = new TH2D("vzDiffEleHad",
     TString("V_{z}("+
       hadTitle[qA]+")-V_{z}(e^{-}) vs. V_{z}("+
       hadTitle[qB]+")-V_{z}(e^{-})"),
     NBINS,-40,40,NBINS,-40,40);


   // kinematic factor distributions
   enum KF_enum {kfA, kfB, kfC, kfV, kfW, kfWA, kfVA, kfCA, kfBA, Nkf};
   TString kfName[Nkf];
   TString kfTitle[Nkf];
   kfName[kfA] = "kfA"; kfTitle[kfA] = "A(y)";
   kfName[kfB] = "kfB"; kfTitle[kfB] = "B(y)";
   kfName[kfC] = "kfC"; kfTitle[kfC] = "C(y)";
   kfName[kfV] = "kfV"; kfTitle[kfV] = "V(y)";
   kfName[kfW] = "kfW"; kfTitle[kfW] = "W(y)";
   kfName[kfWA] = "kfWA"; kfTitle[kfWA] = "W(y)/A(y)";
   kfName[kfVA] = "kfVA"; kfTitle[kfVA] = "V(y)/A(y)";
   kfName[kfCA] = "kfCA"; kfTitle[kfCA] = "C(y)/A(y)";
   kfName[kfBA] = "kfBA"; kfTitle[kfBA] = "B(y)/A(y)";
   Float_t kfVal[Nkf];
   Float_t kfRange[Nkf][2];
   /*
   kfRange[kfA][0]=0.5; kfRange[kfA][1]=1; 
   kfRange[kfB][0]=0; kfRange[kfB][1]=1; 
   kfRange[kfC][0]=0; kfRange[kfC][1]=0.5;
   kfRange[kfV][0]=0; kfRange[kfV][1]=2; 
   kfRange[kfW][0]=0; kfRange[kfW][1]=0.5;
   kfRange[kfWA][0]=0.5; kfRange[kfWA][1]=1.5;
   kfRange[kfVA][0]=0.5; kfRange[kfVA][1]=1.5;
   kfRange[kfCA][0]=0.5; kfRange[kfCA][1]=1.5; 
   kfRange[kfBA][0]=0; kfRange[kfBA][1]=2; 
   */
   kfRange[kfA][0]=-1; kfRange[kfA][1]=2.5; 
   kfRange[kfB][0]=-1; kfRange[kfB][1]=2.5; 
   kfRange[kfC][0]=-1; kfRange[kfC][1]=2.5;
   kfRange[kfV][0]=-1; kfRange[kfV][1]=2.5; 
   kfRange[kfW][0]=-1; kfRange[kfW][1]=2.5;
   kfRange[kfWA][0]=-1; kfRange[kfWA][1]=2.5;
   kfRange[kfVA][0]=-1; kfRange[kfVA][1]=2.5;
   kfRange[kfCA][0]=-1; kfRange[kfCA][1]=2.5; 
   kfRange[kfBA][0]=-1; kfRange[kfBA][1]=2.5; 
   TH2D * kfVsMh[Nkf];
   TH2D * kfVsPhPerp[Nkf];
   TH2D * kfVsX[Nkf];
   TH2D * kfVsQ2[Nkf];
   TH2D * kfVsMmiss[Nkf];
   TH2D * kfVsZpair[Nkf];
   TH2D * kfVsPhiR[Nkf];
   TH2D * kfVsPhiH[Nkf];
   TH2D * kfVsPhiHR[Nkf];
   for(int k=0; k<Nkf; k++) {
     kfVsMh[k] = new TH2D(TString(kfName[k]+"vsMh"),
       TString(kfTitle[k]+" vs. M_{h}"),
       NBINS,0,3,NBINS,kfRange[k][0],kfRange[k][1]);
     kfVsPhPerp[k] = new TH2D(TString(kfName[k]+"vsPhPerp"),
       TString(kfTitle[k]+" vs. P_{h}^{perp}"),
       NBINS,0,3,NBINS,kfRange[k][0],kfRange[k][1]);
     kfVsX[k] = new TH2D(TString(kfName[k]+"vsX"),
       TString(kfTitle[k]+" vs. x"),
       NBINS,0,1,NBINS,kfRange[k][0],kfRange[k][1]);
     kfVsQ2[k] = new TH2D(TString(kfName[k]+"vsQ2"),
       TString(kfTitle[k]+" vs. Q^{2}"),
       NBINS,0,12,NBINS,kfRange[k][0],kfRange[k][1]);
     kfVsMmiss[k] = new TH2D(TString(kfName[k]+"vsMmiss"),
       TString(kfTitle[k]+" vs. M_{X}"),
       NBINS,0,3.5,NBINS,kfRange[k][0],kfRange[k][1]);
     kfVsZpair[k] = new TH2D(TString(kfName[k]+"vsZpair"),
       TString(kfTitle[k]+" vs. z"),
       NBINS,0,1,NBINS,kfRange[k][0],kfRange[k][1]);
     kfVsPhiH[k] = new TH2D(TString(kfName[k]+"vsPhiH"),
       TString(kfTitle[k]+" vs. #phi_{h}"),
       NBINS,-PIe,PIe,NBINS,kfRange[k][0],kfRange[k][1]);
     kfVsPhiR[k] = new TH2D(TString(kfName[k]+"vsPhiR"),
       TString(kfTitle[k]+" vs. #phi_{R}"),
       NBINS,-PIe,PIe,NBINS,kfRange[k][0],kfRange[k][1]);
     kfVsPhiHR[k] = new TH2D(TString(kfName[k]+"vsPhiHR"),
       TString(kfTitle[k]+" vs. #phi_{h}-#phi_{R}"),
       NBINS,-PIe,PIe,NBINS,kfRange[k][0],kfRange[k][1]);
   };

     
   // event-level distributions
   TH1D * helicityDist = new TH1D("helicityDist","helicity",5,-2,3);
  

   ///////////////////////////
   //                       //
   //      EVENT LOOP       //
   //                       //
   ///////////////////////////

   printf("begin loop through %lld events...\n",ev->ENT);
   Int_t hadI[2];
   for(int i=0; i<ev->ENT; i++) {
     //if(i>10000) break; // limiter

     ev->GetEvent(i);


     // fill DIS kinematic plots
     // ------------------------
     if(ev->cutDihadron) {

       //ev->PrintEvent();

       if(ev->cutQ2 && ev->cutY) {
         WDist->Fill(ev->W);
         Q2vsW->Fill(ev->W,ev->Q2);
       };

       if(ev->cutQ2 && ev->cutW) {
         YDist->Fill(ev->y);
       };
     };


     // fill dihadron kinematics plots
     // ------------------------------
     if(ev->Valid()) {

       eleEDist->Fill(ev->eleE);
       elePtDist->Fill(ev->elePt);
       eleEtaDist->Fill(ev->eleEta);
       elePhiDist->Fill(ev->elePhi);
       eleEtaVsPhi->Fill(ev->elePhi,ev->eleEta);
       eleEVsPhi->Fill(ev->elePhi,ev->eleE);
       elePtVsPhi->Fill(ev->elePhi,ev->elePt);
       eleVzDist->Fill(ev->eleVertex[eZ]);
       eleVxyDist->Fill(ev->eleVertex[eX],ev->eleVertex[eY]);


       Q2vsX->Fill(ev->x,ev->Q2);
       XDist->Fill(ev->x);
       Q2Dist->Fill(ev->Q2);


       hadECorr->Fill(ev->hadE[qB],ev->hadE[qA]);
       hadPCorr->Fill(ev->hadP[qB],ev->hadP[qA]);
       hadPtCorr->Fill(ev->hadPt[qB],ev->hadPt[qA]);
       hadEtaCorr->Fill(ev->hadEta[qB],ev->hadEta[qA]);
       hadPhiCorr->Fill(ev->hadPhi[qB],ev->hadPhi[qA]);
       hadZCorr->Fill(ev->Z[qB],ev->Z[qA]);
       hadXFCorr->Fill(ev->hadXF[qB],ev->hadXF[qA]);
       hadVzCorr->Fill(ev->hadVertex[qB][eZ],ev->hadVertex[qA][eZ]);

       for(int h=0; h<2; h++) {
         hadEDist[h]->Fill(ev->hadE[h]);
         hadPDist[h]->Fill(ev->hadP[h]);
         hadPtDist[h]->Fill(ev->hadPt[h]);
         hadEtaDist[h]->Fill(ev->hadEta[h]);
         hadPhiDist[h]->Fill(ev->hadPhi[h]);
         hadZDist[h]->Fill(ev->Z[h]);
         hadXFDist[h]->Fill(ev->hadXF[h]);
         hadVzDist[h]->Fill(ev->hadVertex[h][eZ]);
         hadVxyDist[h]->Fill(ev->hadVertex[h][eX],ev->hadVertex[h][eY]);

         hadEtaVsPhi[h]->Fill(ev->hadPhi[h],ev->hadEta[h]);
         hadEVsPhi[h]->Fill(ev->hadPhi[h],ev->hadE[h]);
         hadPtVsPhi[h]->Fill(ev->hadPhi[h],ev->hadPt[h]);
       };
       vzDiffHadHad->Fill(ev->hadVertex[qA][eZ]-ev->hadVertex[qB][eZ]);
       vzDiffEleHad->Fill(ev->hadVertex[qB][eZ]-ev->eleVertex[eZ],
                          ev->hadVertex[qA][eZ]-ev->eleVertex[eZ]);

       deltaPhi = Tools::AdjAngle(ev->hadPhi[qA] - ev->hadPhi[qB]);
       deltaPhiDist->Fill(deltaPhi);

       MhDist->Fill(ev->Mh);
       PhDist->Fill(ev->Ph);
       PhPerpDist->Fill(ev->PhPerp);
       ZpairDist->Fill(ev->Zpair);
       zetaDist->Fill(ev->zeta);
       xFDist->Fill(ev->xF);
       MmissDist->Fill(ev->Mmiss);
       MmissDistZoom->Fill(ev->Mmiss);
       MmissVsMh->Fill(ev->Mh,ev->Mmiss);

       PhiHDist->Fill(ev->PhiH);
       PhiRDist->Fill(ev->PhiR);
       PhiHvsPhiR->Fill(ev->PhiR,ev->PhiH);

       PhiHRDist->Fill(ev->PhiHR);
       g1perpWeightVsMod->Fill(TMath::Sin(ev->PhiHR),ev->PhPerp/ev->Mh);
       PhPerpVsMh->Fill(ev->Mh,ev->PhPerp);
       RTvsMh->Fill(ev->Mh,ev->RT);

       thetaDist->Fill(ev->theta);
       sinThetaDist->Fill(TMath::Sin(ev->theta));
       sinThetaCosThetaDist->Fill(TMath::Sin(ev->theta)*TMath::Cos(ev->theta));
       cosThetaDist->Fill(TMath::Cos(ev->theta));

       thetaVsPhiH->Fill(ev->PhiH,ev->theta);
       thetaVsPhiR->Fill(ev->PhiR,ev->theta);
       thetaVsPhiHR->Fill(ev->PhiHR,ev->theta);

       thetaVsMh->Fill(ev->Mh,ev->theta);
       thetaVsX->Fill(ev->x,ev->theta);
       thetaVsZpair->Fill(ev->Zpair,ev->theta);
       thetaVsZeta->Fill(ev->zeta,ev->theta);
       thetaVsPh->Fill(ev->Ph,ev->theta);
       for(int h=0; h<2; h++) {
         thetaVsZ[h]->Fill(ev->Z[h],ev->theta);
         thetaVsHadP[h]->Fill(ev->hadP[h],ev->theta);
         YHvsXF[h]->Fill(ev->hadXF[h],ev->GetBreitRapidity(h));
         xFvsZ[h]->Fill(ev->Z[h],ev->hadXF[h]);
       };

       PhiHvsMh->Fill(ev->Mh,ev->PhiH);
       PhiHvsX->Fill(ev->x,ev->PhiH);
       PhiHvsZ->Fill(ev->Zpair,ev->PhiH);

       PhiRvsMh->Fill(ev->Mh,ev->PhiR);
       PhiRvsX->Fill(ev->x,ev->PhiR);
       PhiRvsZ->Fill(ev->Zpair,ev->PhiR);
       PhiRvsAlpha->Fill(ev->alpha,ev->PhiR);
       PhiHRvsAlpha->Fill(ev->alpha,ev->PhiHR);

      
       kfVal[kfA] = ev->GetKinematicFactor('A');
       kfVal[kfB] = ev->GetKinematicFactor('B');
       kfVal[kfC] = ev->GetKinematicFactor('C');
       kfVal[kfV] = ev->GetKinematicFactor('V');
       kfVal[kfW] = ev->GetKinematicFactor('W');
       kfVal[kfWA] = kfVal[kfW] / kfVal[kfA];
       kfVal[kfVA] = kfVal[kfV] / kfVal[kfA];
       kfVal[kfCA] = kfVal[kfC] / kfVal[kfA];
       kfVal[kfBA] = kfVal[kfB] / kfVal[kfA];
       for(int k=0; k<Nkf; k++) {
         kfVsMh[k]->Fill(ev->Mh,kfVal[k]);
         kfVsPhPerp[k]->Fill(ev->PhPerp,kfVal[k]);
         kfVsX[k]->Fill(ev->x,kfVal[k]);
         kfVsQ2[k]->Fill(ev->Q2,kfVal[k]);
         kfVsMmiss[k]->Fill(ev->Mmiss,kfVal[k]);
         kfVsZpair[k]->Fill(ev->Zpair,kfVal[k]);
         kfVsPhiR[k]->Fill(ev->PhiR,kfVal[k]);
         kfVsPhiH[k]->Fill(ev->PhiH,kfVal[k]);
         kfVsPhiHR[k]->Fill(ev->PhiHR,kfVal[k]);
       };
       

       helicityDist->Fill(ev->helicity);

     };


   }; // eo event loop


   WDist->Write();
   XDist->Write();
   Q2Dist->Write();
   Q2vsW->Write();
   Q2vsX->Write();
   YDist->Write();

   eleEDist->Write();
   elePtDist->Write();
   eleEtaDist->Write();
   elePhiDist->Write();
   eleVzDist->Write();
   eleVxyDist->Write();

   eleEtaVsPhi->Write();
   eleEVsPhi->Write();
   elePtVsPhi->Write();


   TCanvas * hadECanv = new TCanvas("hadECanv","hadECanv",1000,800);
   TCanvas * hadPCanv = new TCanvas("hadPCanv","hadPCanv",1000,800);
   TCanvas * hadPtCanv = new TCanvas("hadPtCanv","hadPtCanv",1000,800);
   TCanvas * hadEtaCanv = new TCanvas("hadEtaCanv","hadEtaCanv",1000,800);
   TCanvas * hadPhiCanv = new TCanvas("hadPhiCanv","hadPhiCanv",1000,800);
   TCanvas * hadZCanv = new TCanvas("hadZCanv","hadZCanv",1000,800);
   TCanvas * hadXFCanv = new TCanvas("hadXFCanv","hadXFCanv",1000,800);
   TCanvas * hadVzCanv = new TCanvas("hadVzCanv","hadVzCanv",1000,800);
   TCanvas * hadEtaVsPhiCanv = new TCanvas("hadEtaVsPhiCanv","hadEtaVsPhiCanv",1000,800);
   TCanvas * hadEVsPhiCanv = new TCanvas("hadEVsPhiCanv","hadEVsPhiCanv",1000,800);
   TCanvas * hadPtVsPhiCanv = new TCanvas("hadPtVsPhiCanv","hadPtVsPhiCanv",1000,800);

   HadronCompareCanv(hadECanv, hadEDist, hadECorr);
   HadronCompareCanv(hadPCanv, hadPDist, hadPCorr);
   HadronCompareCanv(hadPtCanv, hadPtDist, hadPtCorr);
   HadronCompareCanv(hadEtaCanv, hadEtaDist, hadEtaCorr);
   HadronCompareCanv(hadPhiCanv, hadPhiDist, hadPhiCorr);
   HadronCompareCanv(hadZCanv, hadZDist, hadZCorr);
   HadronCompareCanv(hadXFCanv, hadXFDist, hadXFCorr);
   HadronCompareCanv(hadVzCanv, hadVzDist, hadVzCorr);
   Hadron2dCanv(hadEtaVsPhiCanv, hadEtaVsPhi[qA], hadEtaVsPhi[qB]);
   Hadron2dCanv(hadEVsPhiCanv, hadEVsPhi[qA], hadEVsPhi[qB]);
   Hadron2dCanv(hadPtVsPhiCanv, hadPtVsPhi[qA], hadPtVsPhi[qB]);

   hadECanv->Write();
   hadPCanv->Write();
   hadPtCanv->Write();
   hadEtaCanv->Write();
   hadPhiCanv->Write();
   hadZCanv->Write();
   hadXFCanv->Write();
   hadEtaVsPhiCanv->Write();
   hadEVsPhiCanv->Write();
   hadPtVsPhiCanv->Write();

   hadVzCanv->Write();
   hadVxyDist[qA]->Write();
   hadVxyDist[qB]->Write();
   vzDiffHadHad->Write();
   vzDiffEleHad->Write();

   deltaPhiDist->Write();

   MhDist->Write();
   PhDist->Write();
   PhPerpDist->Write();
   ZpairDist->Write();
   zetaDist->Write();
   xFDist->Write();
   MmissDist->Write();
   MmissDistZoom->Write();
   MmissVsMh->Write();

   PhiHDist->Write();
   PhiRDist->Write();
   PhiHvsPhiR->Write();

   PhiHRDist->Write();
   g1perpWeightVsMod->Write();
   PhPerpVsMh->Write(); 
   RTvsMh->Write(); 
  
   thetaDist->Write();
   sinThetaDist->Write();
   cosThetaDist->Write();
   sinThetaCosThetaDist->Write();

   thetaVsPhiH->Write();
   thetaVsPhiR->Write();
   thetaVsPhiHR->Write();

   thetaVsMh->Write();
   thetaVsX->Write();
   thetaVsZpair->Write();
   thetaVsZeta->Write();
   for(int h=0; h<2; h++) xFvsZ[h]->Write();
   for(int h=0; h<2; h++) YHvsXF[h]->Write();
   for(int h=0; h<2; h++) thetaVsZ[h]->Write();
   thetaVsPh->Write();
   for(int h=0; h<2; h++) thetaVsHadP[h]->Write();

   PhiHvsMh->Write();
   PhiHvsX->Write();
   PhiHvsZ->Write();

   PhiRvsMh->Write();
   PhiRvsX->Write();
   PhiRvsZ->Write();
   PhiRvsAlpha->Write();
   PhiHRvsAlpha->Write();

   helicityDist->Write();


   outfile->mkdir("kinematicFactors");
   outfile->cd("kinematicFactors");
   for(int k=0; k<Nkf; k++) {
     kfVsMh[k]->Write();
     kfVsPhPerp[k]->Write();
     kfVsX[k]->Write();
     kfVsQ2[k]->Write();
     kfVsMmiss[k]->Write();
     kfVsZpair[k]->Write();
     kfVsPhiR[k]->Write();
     kfVsPhiH[k]->Write();
     kfVsPhiHR[k]->Write();
   };
   outfile->cd("/");

   outfile->Close();

};



// make title for hadron correlation plots
TString corrTitle(TString var) {
  TString varX = var + "(" + hadTitle[qB] + ")";
  TString varY = var + "(" + hadTitle[qA] + ")";
  TString ret = varY + " vs. " + varX + ";" + varX + ";" + varY;
  return ret;
};


TString distTitle(TString var) {
  TString col[2]; 
  if(whichHad[qA]!=whichHad[qB]) {
    for(int cc=0; cc<2; cc++) {
      col[cc] = PartColorName(dihHadIdx(whichHad[qA],whichHad[qB],cc)) + 
                ":" + hadTitle[cc];
    };
  } else {
    col[qA] = "solid:" + hadTitle[qA];
    col[qB] = "dashed:" + hadTitle[qB]; 
  };
  TString ret = var + " distribution (" + col[qA] + " " + col[qB] + ");" + var;
  return ret;
};


TString dist2Title(TString hadron, TString varX,TString varY) {
  return hadron + " " + varY + " vs. " + varX + ";" + varX + ";" + varY;
};



// make canvas for hadron correlation plots
void HadronCompareCanv(TCanvas * canv, TH1D * dist[2], TH2D * corr) {

  for(int h=0; h<2; h++) {
    dist[h]->SetLineColor(PartColor(dihHadIdx(whichHad[qA],whichHad[qB],h)));
    dist[h]->SetLineWidth(2);
  };
  if(whichHad[qA]==whichHad[qB]) dist[qB]->SetLineStyle(kDashed);

  canv->Divide(2,1);

  canv->cd(1);
  Int_t f = dist[qA]->GetMaximum() > dist[qB]->GetMaximum() ? qA:qB;
  dist[f]->Draw();
  dist[(f+1)%2]->Draw("SAME");

  canv->cd(2);
  corr->Draw("colz");
  canv->GetPad(2)->SetGrid(1,1);
};

// make canvas for 2d plots for each hadron 
void Hadron2dCanv(TCanvas * canv, TH2D * distA, TH2D * distB) {
  canv->Divide(2,1);
  canv->cd(1);
  distA->Draw("colz");
  canv->cd(2);
  distB->Draw("colz");
};

