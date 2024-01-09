#include <cstdlib>

// ROOT
#include "TFile.h"
#include "TTree.h"
#include "TChain.h"
#include "TString.h"
#include "TMath.h"
#include "TRegexp.h"
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "TCanvas.h"
#include "TGraph.h"

// Dispin
#include "src/Constants.h"
#include "src/Tools.h"
#include "src/EventTree.h"

void HadronCompareCanv(TCanvas * canv, TH1D * dist[2], TH2D * corr);
void Hadron2dCanv(TCanvas * canv, TH2D * distA, TH2D * distB);
TString corrTitle(TString var);
TString distTitle(TString var);
TString dist2Title(TString hadron, TString varX,TString varY);

TString infiles;
TString outfileN;
Int_t whichPair;
Int_t whichHad[2];
TString hadName[2];
TString hadTitle[2];
TString dihTitle;

int main(int argc, char** argv) {

   // ARGUMENTS
   infiles = "outroot/*.root";
   outfileN = "plots.root";
   whichPair = EncodePairType(kPip,kPim);
   if(argc==1) {
     fprintf(stderr,"USAGE: %s [outroot file(s)] [output plots.root file] [pairType]\n",argv[0]);
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
   };
   dihTitle = PairTitle(whichPair);

   EventTree * ev = new EventTree(infiles,whichPair);


   TFile * outfile = new TFile(outfileN,"RECREATE");

   const Int_t NBINS = 150;
   Float_t deltaPhi;


   // DIS kinematics
   TH1D * WDist = new TH1D("WDist","W distribution;W",NBINS,1,5);
   TH1D * XDist = new TH1D("XDist","x distribution;x",NBINS,0,1);
   TH1D * Q2Dist = new TH1D("Q2Dist","Q^{2} distribution;Q^{2}",NBINS,0,12);
   TH2D * Q2vsW = new TH2D("Q2vsW","Q^{2} vs. W;W;Q^{2}",
                                   NBINS,0,6,NBINS,0,12);
   TH2D * Q2vsX = new TH2D("Q2vsX","Q^{2} vs. x;x;Q^{2}",NBINS,0,1,NBINS,0,12);
   TH1D * YDist = new TH1D("YDist","y distribution;y",NBINS,0,1);
   
   // electron kinematics
   TH1D * eleEDist = new TH1D("eleEDist","e^{-} E distribution",NBINS,0,11);
   TH1D * elePDist = new TH1D("elePDist","e^{-} p distribution",NBINS,0,11);
   TH1D * elePtDist = new TH1D("elePtDist","e^{-} p_{T} distribution",NBINS,0,4);
   TH1D * eleEtaDist = new TH1D("eleEtaDist","e^{-} #eta distribution",NBINS,-3,6);
   TH1D * eleThetaDist = new TH1D("eleThetaDist","e^{-} #theta distribution",NBINS,0,40);
   TH1D * elePhiDist = new TH1D("elePhiDist","e^{-} #phi distribution",NBINS,-PIe,PIe);
   TH2D * eleEtaVsPhi = new TH2D("eleEtavsPhi","e^{-} #eta vs #phi;#phi;#eta",
     NBINS,-PIe,PIe,NBINS,-3,6);
   TH2D * eleEVsPhi = new TH2D("eleEvsPhi","e^{-} E vs #phi;#phi;E",
     NBINS,-PIe,PIe,NBINS,0,12);
   TH2D * elePtVsPhi = new TH2D("elePtvsPhi","e^{-} p_{T} vs #phi;#phi;#p_{T}",
     NBINS,-PIe,PIe,NBINS,0,4);
   TH1D * eleVzDist = new TH1D("eleVzDist","e^{-} V_{z} distribution",
     3*NBINS,-40,40);
   TH2D * eleVxyDist = new TH2D("eleVxyDist","e^{-} V_{y} vs. V_{x}",
     3*NBINS,-6,6,3*NBINS,-6,6);

   // dihadron's hadron kinematic correlations
   TH2D * hadECorr = new TH2D("hadECorr",corrTitle("E"),NBINS,0,10,NBINS,0,10);
   TH2D * hadPCorr = new TH2D("hadPCorr",corrTitle("p"),NBINS,0,10,NBINS,0,10);
   TH2D * hadPtCorr = new TH2D("hadPtCorr",corrTitle("p_{T}"),NBINS,0,4,NBINS,0,4);
   TH2D * hadPperpCorr = new TH2D("hadPperpCorr",corrTitle("p_{perp}"),NBINS,0,2,NBINS,0,2);
   TH2D * hadEtaCorr = new TH2D("hadEtaCorr",corrTitle("#eta"),NBINS,0,5,NBINS,0,5);
   TH2D * hadThetaCorr = new TH2D("hadThetaCorr",corrTitle("#theta"),NBINS,0,40,NBINS,0,40);
   TH2D * hadPhiCorr = new TH2D("hadPhiCorr",corrTitle("#phi"), NBINS,-PIe,PIe,NBINS,-PIe,PIe);
   TH2D * hadZCorr = new TH2D("hadZCorr",corrTitle("z"),NBINS,0,1,NBINS,0,1);
   TH2D * hadXFCorr = new TH2D("hadXFCorr",corrTitle("x_{F}"), NBINS,-1,1,NBINS,-1,1);
   TH2D * hadYHCorr = new TH2D("hadYHCorr",corrTitle("Y_{h}"), NBINS,-4,4,NBINS,-4,4);
   TH2D * hadPhiHCorr = new TH2D("hadPhiHCorr",corrTitle("#phi_{h}"), NBINS,-4,4,NBINS,-4,4);
   TH2D * hadVzCorr = new TH2D("hadVzCorr",corrTitle("V_{z}"), NBINS,-30,30,NBINS,-30,30);
   TH2D * hadEleVzDiffCorr = new TH2D("hadEleVzDiffCorr",corrTitle("V_{z}(had)-V_{z}(e^{-})"), NBINS,-100,100,NBINS,-100,100);
   
   // dihadron's hadron kinematics
   TH1D * hadEDist[2];
   TH1D * hadPDist[2];
   TH1D * hadPtDist[2];
   TH1D * hadPperpDist[2];
   TH1D * hadEtaDist[2];
   TH1D * hadThetaDist[2];
   TH1D * hadPhiDist[2];
   TH1D * hadZDist[2];
   TH1D * hadXFDist[2];
   TH1D * hadYHDist[2];
   TH1D * hadPhiHDist[2];
   TH1D * hadVzDist[2];
   TH1D * hadEleVzDiffDist[2];
   TH1D * hadChi2pidDist[2];
   TH2D * hadChi2pidVsP[2];
   TH2D * hadVxyDist[2];
   TH2D * hadEtaVsPhi[2];
   TH2D * hadEVsPhi[2];
   TH2D * hadPtVsPhi[2];
   for(int h=0; h<2; h++) {
     hadEDist[h] = new TH1D(TString(hadName[h]+"hadEDist"),distTitle("E"),
       NBINS,0,10);
     hadPDist[h] = new TH1D(TString(hadName[h]+"hadPDist"),distTitle("p"),
       NBINS,0,8);
     hadPtDist[h] = new TH1D(TString(hadName[h]+"hadPtDist"),distTitle("p_{T}"),
       NBINS,0,4);
     hadPperpDist[h] = new TH1D(TString(hadName[h]+"hadPperpDist"),distTitle("p_{perp}"),
       NBINS,0,2);
     hadEtaDist[h] = new TH1D(TString(hadName[h]+"hadEtaDist"),distTitle("#eta"),
       NBINS,0,5);
     hadThetaDist[h] = new TH1D(TString(hadName[h]+"hadThetaDist"),distTitle("#theta"),
       NBINS,0,40);
     hadPhiDist[h] = new TH1D(TString(hadName[h]+"hadPhiDist"),distTitle("#phi"),
       NBINS,-PIe,PIe);
     hadZDist[h] = new TH1D(TString(hadName[h]+"hadZDist"),distTitle("z"),
       NBINS,0,1);
     hadXFDist[h] = new TH1D(TString(hadName[h]+"hadXFDist"),distTitle("x_{F}"),
       NBINS,-1,1);
     hadYHDist[h] = new TH1D(TString(hadName[h]+"hadYHDist"),distTitle("Y_{h}"),
       NBINS,-4,4);
     hadPhiHDist[h] = new TH1D(TString(hadName[h]+"hadPhiHDist"),distTitle("#phi_{h}"),
       NBINS,-PIe,PIe);
     hadVzDist[h] = new TH1D(TString(hadName[h]+"hadVzDist"),
       distTitle("V_{z}"),
       NBINS,-30,30);
     hadEleVzDiffDist[h] = new TH1D(TString(hadName[h]+"hadEleVzDiffDist"),
       distTitle("V_{z}(had)-V_{z}(e^{-})"),
       6*NBINS,-100,100);
     hadChi2pidDist[h] = new TH1D(TString(hadName[h]+"hadChi2pidDist"),distTitle("#chi^{2}_{pid}"),
       NBINS,-6,6);
     hadChi2pidVsP[h] = new TH2D(TString(hadName[h]+"hadChi2pidVsP"),hadTitle[h]+" #chi^{2}_{pid} vs. p;p;#chi^{2}_{pid}",
       NBINS,0,8, NBINS,-6,6);

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
   TH1D * MmissDist = new TH1D("MmissDist","M_{X} distribution;M_{X}",2*NBINS,-1,4);

   TH1D * MmissDistZoom = new TH1D("MmissDistZoom","M_{X} distribution;M_{X}",
     3*NBINS,0.5,3);
   TH1D * YHDist = new TH1D("YHDist","Y_{hh} distribution;Y_{hh}",NBINS,-3,3);


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

   TH2D * PhiDVsPhiR = new TH2D("PhiDVsPhiR","#Delta#phi vs. #phi_{R};#phi_{R};#Delta#phi",
     NBINS,-PIe,PIe,NBINS,-PIe,PIe);
   TH2D * SinPhiDVsPhiR = new TH2D("SinPhiDVsPhiR","sin(#Delta#phi) vs. sin(#phi_{R});sin(#phi_{R});sin(#Delta#phi)",
     NBINS,-1.1,1.1,NBINS,-1.1,1.1);
   TH2D * PhiDVsPhiHR = new TH2D("PhiDVsPhiHR","#Delta#phi vs. #phi_{h}-#phi_{R};#phi_{h}-#phi_{R};#Delta#phi",
     NBINS,-PIe,PIe,NBINS,-PIe,PIe);
   TH2D * SinPhiDVsPhiHR = new TH2D("SinPhiDVsPhiHR","sin(#Delta#phi) vs. sin(#phi_{h}-#phi_{R});sin(#phi_{h}-#phi_{R});sin(#Delta#phi)",
     NBINS,-1.1,1.1,NBINS,-1.1,1.1);
   TH2D * PhiDVsPhiHR2 = new TH2D("PhiDVsPhiHR2","#Delta#phi vs. 2#phi_{h}-2#phi_{R};2#phi_{h}-2#phi_{R};#Delta#phi",
     NBINS,-PIe,PIe,NBINS,-PIe,PIe);
   TH2D * SinPhiDVsPhiHR2 = new TH2D("SinPhiDVsPhiHR2","sin(#Delta#phi) vs. sin(2#phi_{h}-2#phi_{R});sin(2#phi_{h}-2#phi_{R});sin(#Delta#phi)",
     NBINS,-1.1,1.1,NBINS,-1.1,1.1);

   plotTitle = "P_{h}^{perp}/M_{h} vs. sin(#phi_{h}-#phi_{R});";
   plotTitle += "sin(#phi_{h}-#phi_{R});P_{h}^{perp}/M_{h}";
   TH2D * g1perpWeightVsMod = new TH2D("g1perpWeightVsMod",plotTitle,
     NBINS,-1.1,1.1,
     NBINS,0,6);
   TH2D * PhPerpVsMh = new TH2D("PhPerpVsMh",
     "P_{h}^{perp} vs. M_{h};M_{h};P_{h}^{perp}",
     NBINS,0,3,
     NBINS,0,3);
   TH2D * YHVsMh = new TH2D("YHVsMh",
     "Y_{hh} vs. M_{h};M_{h};Y_{h}",
     NBINS,0,3,
     NBINS,-3,3);
   TH2D * RTvsMh = new TH2D("RTvsMh",
     "R_{T} vs. M_{h};M_{h};R_{T}",
     NBINS,0,3,
     NBINS,0,1.5);

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
   TH2D * YHvsYCM[2];
   TH2D * YHvsXF[2];
   TH2D * YCMvsXF[2];
   TH2D * YHvsZ[2];
   TH2D * YCMvsZ[2];
   TH2D * XFvsZ[2];
   TH2D * YHvsMh[2];
   TH2D * XFvsMh[2];
   TH2D * PperpvsMh[2];
   TH2D * hadPperpVsYH[2];
   TH2D * dihPhiHvsHadPhiH[2];
   for(int h=0; h<2; h++) {
     thetaVsZ[h] = new TH2D(TString("thetaVsZ_"+hadName[h]),
       TString("#theta vs. "+hadTitle[h]+" z;z;#theta"),
       NBINS,0,1,NBINS,0,PIe);
     thetaVsHadP[h] = new TH2D(TString("thetaVsHadP_"+hadName[h]),
       TString("#theta vs. "+hadTitle[h]+" p;p;#theta"),
       NBINS,0,10,NBINS,0,PIe);
     YHvsYCM[h] = new TH2D(TString("YHvsYCM_"+hadName[h]),
       TString(hadTitle[h]+" Y_{h} vs. Y_{CM};Y_{CM};Y_{h}"),
       NBINS,-3,3,NBINS,-3,3);
     YHvsXF[h] = new TH2D(TString("YHvsXF_"+hadName[h]),
       TString(hadTitle[h]+" Y_{h} vs. x_{F};x_{F};Y_{h}"),
       NBINS,-1,1,NBINS,-3,3);
     YCMvsXF[h] = new TH2D(TString("YCMvsXF_"+hadName[h]),
       TString(hadTitle[h]+" Y_{CM} vs. x_{F};x_{F};Y_{CM}"),
       NBINS,-1,1,NBINS,-3,3);
     YHvsZ[h] = new TH2D(TString("YHvsZ_"+hadName[h]),
       TString(hadTitle[h]+"Y_{H} vs. z;z;Y_{H}"),
       NBINS,0,1,NBINS,-4,4);
     YCMvsZ[h] = new TH2D(TString("YCMvsZ_"+hadName[h]),
       TString(hadTitle[h]+"Y_{CM} vs. z;z;Y_{CM}"),
       NBINS,0,1,NBINS,-4,4);
     XFvsZ[h] = new TH2D(TString("XFvsZ_"+hadName[h]),
       TString(hadTitle[h]+"x_{F} vs. z;z;x_{F}"),
       NBINS,0,1,NBINS,-1,1);
     YHvsMh[h] = new TH2D(TString("YHvsMh_"+hadName[h]),
       TString(hadTitle[h]+" Y_{h} vs. M_{h};M_{h};Y_{h}"),
       2*NBINS,0,3,NBINS,-4,4);
     XFvsMh[h] = new TH2D(TString("XFvsMh_"+hadName[h]),
       TString(hadTitle[h]+" x_{F} vs. M_{h};M_{h};x_{F}"),
       2*NBINS,0,3,NBINS,-1,1);
     PperpvsMh[h] = new TH2D(TString("PperpvsMh_"+hadName[h]),
       TString(hadTitle[h]+" p_{perp} vs. M_{h};M_{h};p_{perp}"),
       2*NBINS,0,3,NBINS,0,2);
     hadPperpVsYH[h] = new TH2D(TString("hadPperpVsYH_"+hadName[h]),
       TString(hadTitle[h]+" p_{perp} vs. Y_{h};Y_{h};p_{perp}"),
       NBINS,-4,4,NBINS,0,2);
     dihPhiHvsHadPhiH[h] = new TH2D(TString("dihPhiH_vs_"+hadName[h]+"PhiH"),
       TString(dihTitle+" #phi_{h} vs. "+hadTitle[h]+" #phi_{h};"+hadTitle[h]+" #phi_{h};"+dihTitle+" #phi_{h}"),
       NBINS,-PIe,PIe,NBINS,-PIe,PIe);
   };


   TH3D * MhVsYHcorr = new TH3D("MhVsYHcorr",
     TString("M_{h} vs. Y_{h} correlation;Y_{h}("+hadTitle[qA]+");Y_{h}("+hadTitle[qB]+");M_{h}"),
     70,-3,3,
     70,-3,3,
     100,0,2
   );
   TH3D * MhVsXFcorr = new TH3D("MhVsXFcorr",
     TString("M_{h} vs. x_{F} correlation;x_{F}("+hadTitle[qA]+");x_{F}("+hadTitle[qB]+");M_{h}"),
     70,-1,1,
     70,-1,1,
     100,0,2
   );
   TH3D * MhVsPperpcorr = new TH3D("MhVsPperpcorr",
     TString("M_{h} vs. p_{perp} correlation;p_{perp}("+hadTitle[qA]+");p_{perp}("+hadTitle[qB]+");M_{h}"),
     70,0,2,
     70,0,2,
     100,0,2
   );
   TH3D * MhVsPtVsYh = new TH3D("MhVsPtVsYh",
     TString("M_{h} vs. P_{h}^{perp} vs. Y_{hh};Y_{hh};P_{h}^{perp};M_{h}"),
     70,-3,3,
     70,0,2,
     100,0,2
   );
   TH3D * MhVsThetaVsAlpha = new TH3D("MhVsThetaVsAlpha",
     TString("M_{h} vs. #theta_{h} vs. #alpha;#alpha;#theta_{h};M_{h}"),
     70,0,70,
     70,0,37,
     100,0,2
   );
   TH3D * MhVsPhiHVsPhiR = new TH3D("MhVsPhiHVsPhiR",
     TString("M_{h} vs. #phi_{h} vs. #phi_{R};#phi_{R};#phi_{h};M_{h}"),
     70,-PI,PI,
     70,-PI,PI,
     100,0,2
   );
   TH3D * MhVsPhiDVsPhiR = new TH3D("MhVsPhiDVsPhiR",
     TString("M_{h} vs. #Delta#phi vs. #phi_{R};#phi_{R};#Delta#phi;M_{h}"),
     70,-PI,PI,
     70,-PI,PI,
     100,0,2
   );

   TH3D * AlphaVsYHcorr = new TH3D("AlphaVsYHcorr",
     TString("#alpha vs. Y_{h} correlation;Y_{h}("+hadTitle[qA]+");Y_{h}("+hadTitle[qB]+");#alpha"),
     70,-3,3,
     70,-3,3,
     100,0,75
   );
   TH3D * AlphaVsPperpcorr = new TH3D("AlphaVsPperpcorr",
     TString("#alpha vs. p_{perp} correlation;p_{perp}("+hadTitle[qA]+");p_{perp}("+hadTitle[qB]+");#alpha"),
     70,0,2,
     70,0,2,
     100,0,75
   );

   TH2D * MhVsAlpha = new TH2D(
     "MhVsAlpha","M_{h} vs. #alpha [deg];#alpha [deg];M_{h}",
     NBINS,0,75,NBINS,0,3);
   
   TH1D * sinThetaDist = new TH1D("sinThetaDist",
     "sin(#theta) distribution;sin(#theta)",NBINS,-1.1,1.1);
   TH1D * sinThetaCosThetaDist = new TH1D("sinThetaCosThetaDist",
     "sin(#theta)cos(#theta) distribution;sin(#theta)cos(#theta)",NBINS,-1.1,1.1);

   TH1D * cosThetaDist = new TH1D("cosThetaDist",
     "cos(#theta) distribution;cos(#theta)",NBINS,-1.1,1.1);
   TH1D * p20dist = new TH1D("p20dist",
     "P_{2,0}(cos#theta)",NBINS,-0.6,1.1);


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
     NBINS,-30,30);

   TH2D * betaVsP[2];
   for(int h=0; h<2; h++) {
     betaVsP[h] = new TH2D(
       TString("betaVsP_"+hadName[h]),dist2Title(hadTitle[h],"p","#beta"),
       5*NBINS,0,10,5*NBINS,0.7,1.2);
   };


   // ECAL energy, sampling fraction (for electrons)
   TH1D * elePCALenDist = new TH1D("elePCALenDist","electron PCAL energy",NBINS,0,1.8);
   TH1D * eleECINenDist = new TH1D("eleECINenDist","electron EC_{in} energy",NBINS,0,1.5);
   TH1D * eleECOUTenDist = new TH1D("eleECOUTenDist","electron EC_{out} energy",NBINS,0,0.6);
   TH2D * eleECALvsPCALedep = new TH2D("eleECALvsPCALedep","E_{ECIN}+E_{ECOUT} vs. E_{PCAL};E_{PCAL};E_{ECIN}+E_{ECOUT}",
     2*NBINS, 0, 1.5, 2*NBINS, 0, 1.0);
   TH1D * eleSampFracDist = new TH1D("eleSampFracDist","electron sampling fraction",2*NBINS,0,0.4);
   TH2D * eleSFvsP = new TH2D("eleSFvsP","electron S.F. vs. p;p;S.F.", NBINS,0,11, 2*NBINS,0,0.4);
   TH2D * eleDiagonalSFdist = new TH2D("eleDiagonalSFdist","eleECINen/eleP vs. elePCALen/eleP;elePCALen/eleP;eleECINen/eleP",
     NBINS,0,0.35, NBINS,0,0.35);

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
   ///*
   kfRange[kfA][0]=0.4; kfRange[kfA][1]=0.7; 
   kfRange[kfB][0]=0; kfRange[kfB][1]=0.7; 
   kfRange[kfC][0]=0.2; kfRange[kfC][1]=0.6;
   kfRange[kfV][0]=0.2; kfRange[kfV][1]=1.4; 
   kfRange[kfW][0]=0.2; kfRange[kfW][1]=0.45;
   kfRange[kfWA][0]=0.4; kfRange[kfWA][1]=0.8;
   kfRange[kfVA][0]=0.8; kfRange[kfVA][1]=2;
   kfRange[kfCA][0]=0.2; kfRange[kfCA][1]=1; 
   kfRange[kfBA][0]=0.2; kfRange[kfBA][1]=1; 
   //*/
   /*
   kfRange[kfA][0]=-1; kfRange[kfA][1]=2.5; 
   kfRange[kfB][0]=-1; kfRange[kfB][1]=2.5; 
   kfRange[kfC][0]=-1; kfRange[kfC][1]=2.5;
   kfRange[kfV][0]=-1; kfRange[kfV][1]=2.5; 
   kfRange[kfW][0]=-1; kfRange[kfW][1]=2.5;
   kfRange[kfWA][0]=-1; kfRange[kfWA][1]=2.5;
   kfRange[kfVA][0]=-1; kfRange[kfVA][1]=2.5;
   kfRange[kfCA][0]=-1; kfRange[kfCA][1]=2.5; 
   kfRange[kfBA][0]=-1; kfRange[kfBA][1]=2.5; 
   */
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

   
   // diphoton distributions
   TH1D * diphMdist = new TH1D(
      "diphMdist",
      "M_{#gamma#gamma} distribution;M_{#gamma#gamma}",
      3*NBINS,0,1);

     
   // event-level distributions
   TH1D * helicityDist = new TH1D("helicityDist","helicity",5,-2,3);
   TH1D * dihadronCntDist = new TH1D("dihadronCntDist","number of dihadrons per event",10,0,10);
   Int_t evnumTmp = -10000;
   Int_t dihadronCnt = 0;

   // string spinner plots
   TH2D * SS_Q2_diff;
   TH2D * SS_W_diff;
   TH2D * SS_x_diff;
   TH2D * SS_y_diff;
   if(ev->useStringSpinner) {
     auto diffRange = [] (auto &h, auto scale) { return scale * (h->GetXaxis()->GetXmax() - h->GetXaxis()->GetXmin()); };
     SS_Q2_diff = new TH2D("SS_Q2_diff", "Q^{2}: comparison between Pythia and Reconstructed;gen;rec - gen", NBINS, Q2Dist->GetXaxis()->GetXmin(), Q2Dist->GetXaxis()->GetXmax(), NBINS, -diffRange(Q2Dist, 0.02), diffRange(Q2Dist, 0.02));
     SS_W_diff  = new TH2D("SS_W_diff",  "W: comparison between Pythia and Reconstructed;gen;rec - gen",     NBINS, WDist->GetXaxis()->GetXmin(),  WDist->GetXaxis()->GetXmax(),  NBINS, -diffRange(WDist,  0.02), diffRange(WDist,  0.02));
     SS_x_diff  = new TH2D("SS_x_diff",  "x: comparison between Pythia and Reconstructed;gen;rec - gen",     NBINS, XDist->GetXaxis()->GetXmin(),  XDist->GetXaxis()->GetXmax(),  NBINS, -diffRange(XDist,  0.2),  diffRange(XDist,  0.2));
     SS_y_diff  = new TH2D("SS_y_diff",  "y: comparison between Pythia and Reconstructed;gen;rec - gen",     NBINS, YDist->GetXaxis()->GetXmin(),  YDist->GetXaxis()->GetXmax(),  NBINS, -diffRange(YDist,  0.02), diffRange(YDist,  0.02));
   }
  

   ///////////////////////////
   //                       //
   //      EVENT LOOP       //
   //                       //
   ///////////////////////////

   printf("begin loop through %lld events...\n",ev->ENT);
   Int_t hadI[2];
   Float_t alphaDeg;
   for(int i=0; i<ev->ENT; i++) {
     //if(i>10000) break; // limiter

     ev->GetEvent(i);

     // fill dihadron kinematics plots
     // ------------------------------
     if(ev->Valid()) {

       if(evnumTmp<0) evnumTmp=ev->evnum;
       if(ev->evnum!=evnumTmp) {
         dihadronCntDist->Fill(dihadronCnt);
         dihadronCnt=0;
         evnumTmp=ev->evnum;
       };
       dihadronCnt++;

       WDist->Fill(ev->W);
       Q2vsW->Fill(ev->W,ev->Q2);
       YDist->Fill(ev->y);

       eleEDist->Fill(ev->eleE);
       elePDist->Fill(ev->eleP);
       elePtDist->Fill(ev->elePt);
       eleEtaDist->Fill(ev->eleEta);
       eleThetaDist->Fill(ev->eleTheta);
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
       hadPperpCorr->Fill(ev->hadPperp[qB],ev->hadPperp[qA]);
       hadEtaCorr->Fill(ev->hadEta[qB],ev->hadEta[qA]);
       hadThetaCorr->Fill(ev->hadTheta[qB],ev->hadTheta[qA]);
       hadPhiCorr->Fill(ev->hadPhi[qB],ev->hadPhi[qA]);
       hadZCorr->Fill(ev->Z[qB],ev->Z[qA]);
       hadXFCorr->Fill(ev->hadXF[qB],ev->hadXF[qA]);
       hadYHCorr->Fill(ev->hadYH[qB],ev->hadYH[qA]);
       hadPhiHCorr->Fill(ev->hadPhiH[qB],ev->hadPhiH[qA]);
       hadVzCorr->Fill(ev->hadVertex[qB][eZ],ev->hadVertex[qA][eZ]);
       hadEleVzDiffCorr->Fill(ev->hadVertex[qB][eZ]-ev->eleVertex[eZ],
                              ev->hadVertex[qA][eZ]-ev->eleVertex[eZ]);

       for(int h=0; h<2; h++) {
         hadEDist[h]->Fill(ev->hadE[h]);
         hadPDist[h]->Fill(ev->hadP[h]);
         hadPtDist[h]->Fill(ev->hadPt[h]);
         hadPperpDist[h]->Fill(ev->hadPperp[h]);
         hadEtaDist[h]->Fill(ev->hadEta[h]);
         hadThetaDist[h]->Fill(ev->hadTheta[h]);
         hadPhiDist[h]->Fill(ev->hadPhi[h]);
         hadZDist[h]->Fill(ev->Z[h]);
         hadXFDist[h]->Fill(ev->hadXF[h]);
         hadYHDist[h]->Fill(ev->hadYH[h]);
         hadPhiHDist[h]->Fill(ev->hadPhiH[h]);
         hadVzDist[h]->Fill(ev->hadVertex[h][eZ]);
         hadEleVzDiffDist[h]->Fill(ev->hadVertex[h][eZ] - ev->eleVertex[eZ]);
         hadChi2pidDist[h]->Fill(ev->hadChi2pid[h]);
         hadChi2pidVsP[h]->Fill(ev->hadP[h],ev->hadChi2pid[h]);
         hadVxyDist[h]->Fill(ev->hadVertex[h][eX],ev->hadVertex[h][eY]);

         hadEtaVsPhi[h]->Fill(ev->hadPhi[h],ev->hadEta[h]);
         hadEVsPhi[h]->Fill(ev->hadPhi[h],ev->hadE[h]);
         hadPtVsPhi[h]->Fill(ev->hadPhi[h],ev->hadPt[h]);
       };
       vzDiffHadHad->Fill(ev->hadVertex[qA][eZ]-ev->hadVertex[qB][eZ]);

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
       YHDist->Fill(ev->YH);

       MmissVsMh->Fill(ev->Mh,ev->Mmiss);

       PhiHDist->Fill(ev->PhiH);
       PhiRDist->Fill(ev->PhiR);
       PhiHvsPhiR->Fill(ev->PhiR,ev->PhiH);
       PhiDVsPhiR->Fill(ev->PhiR,ev->PhiD);
       SinPhiDVsPhiR->Fill(TMath::Sin(ev->PhiR),TMath::Sin(ev->PhiD));
       PhiDVsPhiHR->Fill(Tools::AdjAngle(ev->PhiH-ev->PhiR),ev->PhiD);
       SinPhiDVsPhiHR->Fill(TMath::Sin(Tools::AdjAngle(ev->PhiH-ev->PhiR)),TMath::Sin(ev->PhiD));
       PhiDVsPhiHR2->Fill(Tools::AdjAngle(2*ev->PhiH-2*ev->PhiR),ev->PhiD);
       SinPhiDVsPhiHR2->Fill(TMath::Sin(Tools::AdjAngle(2*ev->PhiH-2*ev->PhiR)),TMath::Sin(ev->PhiD));

       PhiHRDist->Fill(ev->PhiHR);
       g1perpWeightVsMod->Fill(TMath::Sin(ev->PhiHR),ev->PhPerp/ev->Mh);
       PhPerpVsMh->Fill(ev->Mh,ev->PhPerp);
       YHVsMh->Fill(ev->Mh,ev->YH);
       RTvsMh->Fill(ev->Mh,ev->RT);

       thetaDist->Fill(ev->theta);
       sinThetaDist->Fill(TMath::Sin(ev->theta));
       sinThetaCosThetaDist->Fill(TMath::Sin(ev->theta)*TMath::Cos(ev->theta));
       cosThetaDist->Fill(TMath::Cos(ev->theta));
       p20dist->Fill(0.5*(3*TMath::Power(TMath::Cos(ev->theta),2)-1));

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
         YHvsYCM[h]->Fill(ev->hadYCM[h],ev->hadYH[h]);
         YHvsXF[h]->Fill(ev->hadXF[h],ev->hadYH[h]);
         YCMvsXF[h]->Fill(ev->hadXF[h],ev->hadYCM[h]);
         YHvsZ[h]->Fill(ev->Z[h],ev->hadYH[h]);
         YCMvsZ[h]->Fill(ev->Z[h],ev->hadYCM[h]);
         XFvsZ[h]->Fill(ev->Z[h],ev->hadXF[h]);
         YHvsMh[h]->Fill(ev->Mh,ev->hadYH[h]);
         XFvsMh[h]->Fill(ev->Mh,ev->hadXF[h]);
         PperpvsMh[h]->Fill(ev->Mh,ev->hadPperp[h]);
         hadPperpVsYH[h]->Fill(ev->hadYH[h],ev->hadPperp[h]);
         dihPhiHvsHadPhiH[h]->Fill(ev->hadPhiH[h],ev->PhiH);
       };
       
       alphaDeg = ev->alpha*TMath::RadToDeg();

       MhVsYHcorr->Fill(ev->hadYH[qA],ev->hadYH[qB],ev->Mh);
       MhVsXFcorr->Fill(ev->hadXF[qA],ev->hadXF[qB],ev->Mh);
       MhVsPperpcorr->Fill(ev->hadPperp[qA],ev->hadPperp[qB],ev->Mh);
       MhVsPtVsYh->Fill(ev->YH,ev->PhPerp,ev->Mh);
       MhVsThetaVsAlpha->Fill(alphaDeg,Tools::EtaToTheta(ev->PhEta),ev->Mh);
       MhVsPhiHVsPhiR->Fill(ev->PhiR,ev->PhiH,ev->Mh);
       MhVsPhiDVsPhiR->Fill(ev->PhiR,ev->PhiD,ev->Mh);

       AlphaVsYHcorr->Fill(ev->hadYH[qA],ev->hadYH[qB],alphaDeg);
       AlphaVsPperpcorr->Fill(ev->hadPperp[qA],ev->hadPperp[qB],alphaDeg);

       MhVsAlpha->Fill(alphaDeg,ev->Mh);

       PhiHvsMh->Fill(ev->Mh,ev->PhiH);
       PhiHvsX->Fill(ev->x,ev->PhiH);
       PhiHvsZ->Fill(ev->Zpair,ev->PhiH);

       PhiRvsMh->Fill(ev->Mh,ev->PhiR);
       PhiRvsX->Fill(ev->x,ev->PhiR);
       PhiRvsZ->Fill(ev->Zpair,ev->PhiR);
       PhiRvsAlpha->Fill(ev->alpha,ev->PhiR);
       PhiHRvsAlpha->Fill(ev->alpha,ev->PhiHR);

       for(int h=0; h<2; h++) {
         betaVsP[h]->Fill(ev->hadP[h],ev->hadBeta[h]);
       };

       elePCALenDist->Fill(ev->elePCALen);
       eleECINenDist->Fill(ev->eleECINen);
       eleECOUTenDist->Fill(ev->eleECOUTen);
       eleECALvsPCALedep->Fill( ev->elePCALen, ev->eleECINen + ev->eleECOUTen );
       eleSampFracDist->Fill(ev->eleSampFrac);
       eleSFvsP->Fill(ev->eleP,ev->eleSampFrac);
       if(ev->eleP>0) eleDiagonalSFdist->Fill( ev->elePCALen / ev->eleP,  ev->eleECINen / ev->eleP );
       else eleDiagonalSFdist->Fill(-10000,-10000);

       kfVal[kfA] = ev->GetDepolarizationFactor('A');
       kfVal[kfB] = ev->GetDepolarizationFactor('B');
       kfVal[kfC] = ev->GetDepolarizationFactor('C');
       kfVal[kfV] = ev->GetDepolarizationFactor('V');
       kfVal[kfW] = ev->GetDepolarizationFactor('W');
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

       diphMdist->Fill(ev->objDiphoton->M);

       if(ev->useStringSpinner) {
         SS_Q2_diff->Fill(ev->SS_Q2, ev->Q2 - ev->SS_Q2);
         SS_W_diff->Fill(ev->SS_W,  ev->W  - ev->SS_W );
         SS_x_diff->Fill(ev->SS_x,  ev->x  - ev->SS_x );
         SS_y_diff->Fill(ev->SS_y,  ev->y  - ev->SS_y );
       }

     };


   }; // eo event loop
   dihadronCntDist->Fill(dihadronCnt); // (count last event's dihadrons)

   WDist->Write();
   XDist->Write();
   Q2Dist->Write();
   Q2vsW->Write();
   Q2vsX->Write();
   YDist->Write();

   eleEDist->Write();
   elePDist->Write();
   elePtDist->Write();
   eleEtaDist->Write();
   eleThetaDist->Write();
   elePhiDist->Write();

   eleEtaVsPhi->Write();
   eleEVsPhi->Write();
   elePtVsPhi->Write();


   TCanvas * hadECanv = new TCanvas("hadECanv","hadECanv",1000,800);
   TCanvas * hadPCanv = new TCanvas("hadPCanv","hadPCanv",1000,800);
   TCanvas * hadPtCanv = new TCanvas("hadPtCanv","hadPtCanv",1000,800);
   TCanvas * hadPperpCanv = new TCanvas("hadPperpCanv","hadPperpCanv",1000,800);
   TCanvas * hadEtaCanv = new TCanvas("hadEtaCanv","hadEtaCanv",1000,800);
   TCanvas * hadThetaCanv = new TCanvas("hadThetaCanv","hadThetaCanv",1000,800);
   TCanvas * hadPhiCanv = new TCanvas("hadPhiCanv","hadPhiCanv",1000,800);
   TCanvas * hadZCanv = new TCanvas("hadZCanv","hadZCanv",1000,800);
   TCanvas * hadXFCanv = new TCanvas("hadXFCanv","hadXFCanv",1000,800);
   TCanvas * hadYHCanv = new TCanvas("hadYHCanv","hadYHCanv",1000,800);
   TCanvas * hadPhiHCanv = new TCanvas("hadPhiHCanv","hadPhiHCanv",1000,800);
   TCanvas * hadVzCanv = new TCanvas("hadVzCanv","hadVzCanv",1000,800);
   TCanvas * hadEleVzDiffCanv = new TCanvas("hadEleVzDiffCanv","hadEleVzDiffCanv",1000,800);
   TCanvas * hadEtaVsPhiCanv = new TCanvas("hadEtaVsPhiCanv","hadEtaVsPhiCanv",1000,800);
   TCanvas * hadEVsPhiCanv = new TCanvas("hadEVsPhiCanv","hadEVsPhiCanv",1000,800);
   TCanvas * hadPtVsPhiCanv = new TCanvas("hadPtVsPhiCanv","hadPtVsPhiCanv",1000,800);

   HadronCompareCanv(hadECanv, hadEDist, hadECorr);
   HadronCompareCanv(hadPCanv, hadPDist, hadPCorr);
   HadronCompareCanv(hadPtCanv, hadPtDist, hadPtCorr);
   HadronCompareCanv(hadPperpCanv, hadPperpDist, hadPperpCorr);
   HadronCompareCanv(hadEtaCanv, hadEtaDist, hadEtaCorr);
   HadronCompareCanv(hadThetaCanv, hadThetaDist, hadThetaCorr);
   HadronCompareCanv(hadPhiCanv, hadPhiDist, hadPhiCorr);
   HadronCompareCanv(hadZCanv, hadZDist, hadZCorr);
   HadronCompareCanv(hadXFCanv, hadXFDist, hadXFCorr);
   HadronCompareCanv(hadYHCanv, hadYHDist, hadYHCorr);
   HadronCompareCanv(hadPhiHCanv, hadPhiHDist, hadPhiHCorr);
   HadronCompareCanv(hadVzCanv, hadVzDist, hadVzCorr);
   HadronCompareCanv(hadEleVzDiffCanv, hadEleVzDiffDist, hadEleVzDiffCorr);
   Hadron2dCanv(hadEtaVsPhiCanv, hadEtaVsPhi[qA], hadEtaVsPhi[qB]);
   Hadron2dCanv(hadEVsPhiCanv, hadEVsPhi[qA], hadEVsPhi[qB]);
   Hadron2dCanv(hadPtVsPhiCanv, hadPtVsPhi[qA], hadPtVsPhi[qB]);

   hadECanv->Write();
   hadPCanv->Write();
   hadPtCanv->Write();
   hadPperpCanv->Write();
   hadEtaCanv->Write();
   hadThetaCanv->Write();
   hadPhiCanv->Write();
   hadZCanv->Write();
   hadXFCanv->Write();
   hadYHCanv->Write();
   hadPhiHCanv->Write();
   hadEtaVsPhiCanv->Write();
   hadEVsPhiCanv->Write();
   hadPtVsPhiCanv->Write();

   // vertex
   hadEleVzDiffCanv->Write();
   eleVzDist->Write();
   for(int h=0; h<2; h++) hadEleVzDiffDist[h]->Write();
   hadVzCanv->Write();
   for(int h=0; h<2; h++) hadVxyDist[h]->Write();
   hadVxyDist[qB]->Write();
   vzDiffHadHad->Write();
   eleVxyDist->Write();

   deltaPhiDist->Write();

   MhDist->Write();
   PhDist->Write();
   PhPerpDist->Write();
   ZpairDist->Write();
   zetaDist->Write();
   xFDist->Write();
   MmissDist->Write();
   MmissDistZoom->Write();
   YHDist->Write();
   for(int h=0; h<2; h++) hadYHDist[h]->Write();
   for(int h=0; h<2; h++) hadXFDist[h]->Write();

   MmissVsMh->Write();

   PhiHDist->Write();
   PhiRDist->Write();
   PhiHvsPhiR->Write();

   PhiHRDist->Write();
   g1perpWeightVsMod->Write();
   RTvsMh->Write(); 
  
   thetaDist->Write();
   sinThetaDist->Write();
   cosThetaDist->Write();
   p20dist->Write();
   sinThetaCosThetaDist->Write();

   thetaVsPhiH->Write();
   thetaVsPhiR->Write();
   thetaVsPhiHR->Write();

   thetaVsMh->Write();
   thetaVsX->Write();
   thetaVsZpair->Write();
   thetaVsZeta->Write();
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
   for(int h=0; h<2; h++) betaVsP[h]->Write();
   helicityDist->Write();
   dihadronCntDist->Write();

   diphMdist->Write();

   for(int h=0; h<2; h++) PperpvsMh[h]->Write();
   for(int h=0; h<2; h++) dihPhiHvsHadPhiH[h]->Write();
   for(int h=0; h<2; h++) hadPhiHDist[h]->Write();
   PhPerpVsMh->Write(); 
   MhVsPperpcorr->Write();
   MhVsAlpha->Write();
   MhVsThetaVsAlpha->Write();
   MhVsPhiHVsPhiR->Write();
   MhVsPhiDVsPhiR->Write();
   AlphaVsPperpcorr->Write();
   PhiDVsPhiR->Write();
   SinPhiDVsPhiR->Write();
   PhiDVsPhiHR->Write();
   SinPhiDVsPhiHR->Write();
   PhiDVsPhiHR2->Write();
   SinPhiDVsPhiHR2->Write();

   outfile->mkdir("pidRefinementDists");
   outfile->cd("pidRefinementDists");
   elePCALenDist->Write();
   eleECINenDist->Write();
   eleECOUTenDist->Write();
   eleECALvsPCALedep->Write();
   eleSampFracDist->Write();
   eleSFvsP->Write();
   eleDiagonalSFdist->Write();
   for(int h=0; h<2; h++) hadThetaDist[h]->Write();
   for(int h=0; h<2; h++) hadPDist[h]->Write();
   for(int h=0; h<2; h++) hadChi2pidDist[h]->Write();
   for(int h=0; h<2; h++) hadChi2pidVsP[h]->Write();
   outfile->cd("/");

   outfile->mkdir("rapidity");
   outfile->cd("rapidity");
   for(int h=0; h<2; h++) YHvsYCM[h]->Write();
   for(int h=0; h<2; h++) YHvsXF[h]->Write();
   for(int h=0; h<2; h++) YCMvsXF[h]->Write();
   for(int h=0; h<2; h++) YHvsZ[h]->Write();
   for(int h=0; h<2; h++) YCMvsZ[h]->Write();
   for(int h=0; h<2; h++) XFvsZ[h]->Write();
   for(int h=0; h<2; h++) YHvsMh[h]->Write();
   for(int h=0; h<2; h++) XFvsMh[h]->Write();
   for(int h=0; h<2; h++) hadPperpVsYH[h]->Write();
   YHVsMh->Write(); 
   MhVsYHcorr->Write();
   MhVsXFcorr->Write();
   MhVsPtVsYh->Write();
   AlphaVsYHcorr->Write();
   outfile->cd("/");



   outfile->mkdir("depolarizationFactors");
   outfile->cd("depolarizationFactors");
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

   if(ev->useStringSpinner) {
     outfile->mkdir("string_spinner");
     outfile->cd("string_spinner");
     SS_Q2_diff->Write();
     SS_W_diff->Write();
     SS_x_diff->Write();
     SS_y_diff->Write();
     outfile->cd("/");
   }

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

