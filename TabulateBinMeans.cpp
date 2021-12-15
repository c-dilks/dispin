#include <cstdlib>
#include <iostream>
#include <vector>
#include <map>
#include <functional>

// ROOT
#include "TFile.h"
#include "TString.h"
#include "TMath.h"
#include "TRegexp.h"
#include "TH1.h"
#include "TH2.h"
#include "TF1.h"
#include "TSystem.h"
#include "TObjArray.h"

// dispin
#include "Tools.h"
#include "Constants.h"
#include "Binning.h"
#include "EventTree.h"

TString inputData;
Int_t pairType;
Int_t ivType;
Binning * BS;
EventTree * ev;

int PrintUsage();
void SetDefaultArgs();
void PrintMeans(TString title, std::map<Int_t,TH1D*> mapdist);
void DrawPlot(
    TString name, TString titleX, TString titleY,
    std::function<Double_t(Int_t)> lambdaXval, std::function<Double_t(Int_t)> lambdaXerr,
    std::function<Double_t(Int_t)> lambdaYval, std::function<Double_t(Int_t)> lambdaYerr
    );

int main(int argc, char** argv) {

  // read options
  SetDefaultArgs();
  int opt;
  enum inputType_enum {iFile,iDir};
  Int_t inputType = -1;
  while( (opt=getopt(argc,argv,"f:d:p:i:")) != -1 ) {
    switch(opt) {
      case 'f': /* input file */
        if(inputType>=0) return PrintUsage();
        inputData = optarg;
        inputType = iFile;
        break;
      case 'd': /* input directory */
        if(inputType>=0) return PrintUsage();
        inputData = optarg;
        inputType = iDir;
        break;
      case 'p': /* pair type (hexadecimal number) */
        pairType = (Int_t) strtof(optarg,NULL);
        break;
      case 'i': /* independent variables */
        ivType = (Int_t) strtof(optarg,NULL);
        break;
      default: return PrintUsage();
    };
  };
  if(inputType!=iFile && inputType!=iDir) {
    fprintf(stderr,"ERROR: must specify input file or directory\n");
    return PrintUsage();
  };
  if(pairType==0x99) {
    fprintf(stderr,"ERROR: pi0 pi0 channel is under construction\n");
    return 0;
  };

  // print arguments' values
  Tools::PrintSeparator(40,"=");
  printf("inputData = %s\n",inputData.Data());
  printf("pairType = 0x%x\n",pairType);
  printf("ivType = %d\n",ivType);

  // set binning scheme
  BS = new Binning();
  Bool_t schemeSuccess = BS->SetScheme(ivType);
  if(!schemeSuccess) {
    fprintf(stderr,"ERROR: Binning::SetScheme failed\n");
    return 0;
  };

  // instantiate EventTree 
  // (use 1 file if inputType==iFile, 
  //  or all root files in inputData if inputType==iDir)
  ev = new EventTree(inputData+(inputType==iDir?"/*.root":""),pairType);

  
  // define histograms
  TFile * outfile = new TFile("tables.root","RECREATE");
  TH1D * distMh; std::map<Int_t,TH1D*> mapdistMh;
  TH1D * distX; std::map<Int_t,TH1D*> mapdistX;
  TH1D * distZ; std::map<Int_t,TH1D*> mapdistZ;
  TH1D * distQ2; std::map<Int_t,TH1D*> mapdistQ2;
  TH1D * distY; std::map<Int_t,TH1D*> mapdistY;
  TH1D * distPhPerp; std::map<Int_t,TH1D*> mapdistPhPerp;
  TH1D * distDepolA; std::map<Int_t,TH1D*> mapdistDepolA;
  TH1D * distDepolC; std::map<Int_t,TH1D*> mapdistDepolC;
  TH1D * distDepolW; std::map<Int_t,TH1D*> mapdistDepolW;
  TH1D * distDepolCA; std::map<Int_t,TH1D*> mapdistDepolCA;
  TH1D * distDepolWA; std::map<Int_t,TH1D*> mapdistDepolWA;
  TH1D * distP0; std::map<Int_t,TH1D*> mapdistP0;
  TH1D * distP1; std::map<Int_t,TH1D*> mapdistP1;
  TH1D * distF; std::map<Int_t,TH1D*> mapdistF;
  TH1D * distG; std::map<Int_t,TH1D*> mapdistG;
  TH1D * distFG; std::map<Int_t,TH1D*> mapdistFG;
  TH1D * distFGH; std::map<Int_t,TH1D*> mapdistFGH;
  TString bStr;
  for(Int_t b : BS->binVec) {
    bStr = Form("Bin%d",b);
    distMh = new TH1D(TString("distMh"+bStr),TString("Mh for "+bStr),100,0,3);
    distX = new TH1D(TString("distX"+bStr),TString("X for "+ bStr),100,0,1);
    distZ = new TH1D(TString("distZ"+bStr),TString("Z for "+ bStr),100,0,1);
    distQ2 = new TH1D(TString("distQ2"+bStr),TString("Q2 for "+bStr),100,0,12);
    distY = new TH1D(TString("distY"+bStr),TString("Y for "+ bStr),100,0,1);
    distPhPerp = new TH1D(TString("distPhPerp"+bStr),TString("PhPerp for "+bStr),100,0,5);
    distDepolA = new TH1D(TString("distDepolA"+bStr),TString("DepolA for "+bStr),1000,-1,2.5);
    distDepolC = new TH1D(TString("distDepolC"+bStr),TString("DepolC for "+bStr),1000,-1,2.5);
    distDepolW = new TH1D(TString("distDepolW"+bStr),TString("DepolW for "+bStr),1000,-1,2.5);
    distDepolCA = new TH1D(TString("distDepolCA"+bStr),TString("DepolCA for "+bStr),1000,-1,2.5);
    distDepolWA = new TH1D(TString("distDepolWA"+bStr),TString("DepolWA for "+bStr),1000,-1,2.5);
    distP0 = new TH1D(TString("distP0"+bStr),TString("P_{2,0}(cos#theta) for "+bStr),1000,-0.6,1.1);
    distP1 = new TH1D(TString("distP1"+bStr),TString("sin#theta for "+bStr),1000,-1.1,1.1);
    distF = new TH1D(TString("distF"+bStr),TString("F for "+bStr),1000,-1.1,1.1);
    distG = new TH1D(TString("distG"+bStr),TString("G for "+bStr),1000,-1.1,1.1);
    distFG = new TH1D(TString("distFG"+bStr),TString("FG for "+bStr),1000,-1.1,1.1);
    distFGH = new TH1D(TString("distFGH"+bStr),TString("FGH for "+bStr),1000,-1.1,1.1);
    mapdistMh.insert(std::pair<Int_t,TH1D*>(b,distMh));
    mapdistX.insert(std::pair<Int_t,TH1D*>(b,distX));
    mapdistZ.insert(std::pair<Int_t,TH1D*>(b,distZ));
    mapdistQ2.insert(std::pair<Int_t,TH1D*>(b,distQ2));
    mapdistY.insert(std::pair<Int_t,TH1D*>(b,distY));
    mapdistPhPerp.insert(std::pair<Int_t,TH1D*>(b,distPhPerp));
    mapdistDepolA.insert(std::pair<Int_t,TH1D*>(b,distDepolA));
    mapdistDepolC.insert(std::pair<Int_t,TH1D*>(b,distDepolC));
    mapdistDepolW.insert(std::pair<Int_t,TH1D*>(b,distDepolW));
    mapdistDepolCA.insert(std::pair<Int_t,TH1D*>(b,distDepolCA));
    mapdistDepolWA.insert(std::pair<Int_t,TH1D*>(b,distDepolWA));
    mapdistP0.insert(std::pair<Int_t,TH1D*>(b,distP0));
    mapdistP1.insert(std::pair<Int_t,TH1D*>(b,distP1));
    mapdistF.insert(std::pair<Int_t,TH1D*>(b,distF));
    mapdistG.insert(std::pair<Int_t,TH1D*>(b,distG));
    mapdistFG.insert(std::pair<Int_t,TH1D*>(b,distFG));
    mapdistFGH.insert(std::pair<Int_t,TH1D*>(b,distFGH));
  };




  //-----------------------------------------------------
  // EVENT LOOP  
  //-----------------------------------------------------
  Int_t bn;
  Float_t iv;
  printf("begin loop through %lld events...\n",ev->ENT);
  for(int i=0; i<ev->ENT; i++) {
    if(i>100000) break; // limiter
    ev->GetEvent(i);
    if(ev->Valid()) {

      // get bin number
      bn = BS->FindBin(ev);

      // fill histograms
      mapdistMh.at(bn)->Fill(ev->Mh);
      mapdistX.at(bn)->Fill(ev->x);
      mapdistZ.at(bn)->Fill(ev->Zpair);
      mapdistQ2.at(bn)->Fill(ev->Q2);
      mapdistY.at(bn)->Fill(ev->y);
      mapdistPhPerp.at(bn)->Fill(ev->PhPerp);
      mapdistDepolA.at(bn)->Fill(ev->GetDepolarizationFactor('A'));
      mapdistDepolC.at(bn)->Fill(ev->GetDepolarizationFactor('C'));
      mapdistDepolW.at(bn)->Fill(ev->GetDepolarizationFactor('W'));
      mapdistDepolCA.at(bn)->Fill(ev->GetDepolarizationFactor('C')/ev->GetDepolarizationFactor('A'));
      mapdistDepolWA.at(bn)->Fill(ev->GetDepolarizationFactor('W')/ev->GetDepolarizationFactor('A'));
      mapdistP0.at(bn)->Fill(0.5*(3*TMath::Power(TMath::Cos(ev->theta),2)-1));
      mapdistP1.at(bn)->Fill(TMath::Sin(ev->theta));
      mapdistF.at(bn)->Fill(TMath::Cos(ev->PhiH));
      mapdistG.at(bn)->Fill(TMath::Sin(ev->PhiR));
      mapdistFG.at(bn)->Fill(TMath::Sin(2*ev->PhiH-ev->PhiR)*TMath::Sin(ev->PhiH));
      mapdistFGH.at(bn)->Fill(
        TMath::Sin(ev->PhiH-ev->PhiR) *
        TMath::Sin(ev->PhiH-ev->PhiR) *
        TMath::Cos(ev->PhiH)
      );
    };
  };

  // lambda: calculate error propagation of a*b or a/b
  auto propErr = [&](Double_t a, Double_t aErr, Double_t b, Double_t bErr){ return a/b * TMath::Hypot(aErr/a,bErr); };

  // draw plots (usually mean vs. mean)
  DrawPlot("aveWA_vs_aveX","<x>","<W/A>",
      [&](Int_t b){ return mapdistX.at(b)->GetMean(); },
      [&](Int_t b){ return mapdistX.at(b)->GetStdDev(); },
      [&](Int_t b){ return mapdistDepolWA.at(b)->GetMean(); },
      [&](Int_t b){ return mapdistDepolWA.at(b)->GetStdDev(); }
      );
  DrawPlot("aveW_aveA_vs_aveX","<x>","<W>/<A>",
      [&](Int_t b){ return mapdistX.at(b)->GetMean(); },
      [&](Int_t b){ return mapdistX.at(b)->GetStdDev(); },
      [&](Int_t b){ return mapdistDepolW.at(b)->GetMean() / mapdistDepolA.at(b)->GetMean(); },
      [&](Int_t b){ return propErr(
        mapdistDepolW.at(b)->GetMean(), mapdistDepolW.at(b)->GetStdDev(),
        mapdistDepolA.at(b)->GetMean(), mapdistDepolA.at(b)->GetStdDev()
        );}
      );
    

  // print means for cross check
  PrintMeans("x",mapdistX);
  PrintMeans("z",mapdistZ);
  PrintMeans("Mpipi",mapdistMh);
  PrintMeans("Q2",mapdistQ2);
  PrintMeans("y",mapdistY);
  PrintMeans("pHT",mapdistPhPerp);
  PrintMeans("A(y,eps)",mapdistDepolA);
  PrintMeans("C(y,eps)",mapdistDepolC);
  PrintMeans("W(y,eps)",mapdistDepolW);
  PrintMeans("C/A(y,eps)",mapdistDepolCA);
  PrintMeans("W/A(y,eps)",mapdistDepolWA);
  PrintMeans("P_{2,0}(cos#theta)",mapdistP0);
  PrintMeans("sin#theta",mapdistP1);
  PrintMeans("F",mapdistF);
  PrintMeans("G",mapdistG);
  PrintMeans("FG",mapdistFG);
  PrintMeans("FGH",mapdistFGH);

  // write histograms
  for(Int_t b : BS->binVec) {
    mapdistMh.at(b)->Write();
    mapdistX.at(b)->Write();
    mapdistZ.at(b)->Write();
    mapdistQ2.at(b)->Write();
    mapdistY.at(b)->Write();
    mapdistPhPerp.at(b)->Write();
    mapdistDepolA.at(b)->Write();
    mapdistDepolC.at(b)->Write();
    mapdistDepolW.at(b)->Write();
    mapdistDepolCA.at(b)->Write();
    mapdistDepolWA.at(b)->Write();
    mapdistP0.at(b)->Write();
    mapdistP1.at(b)->Write();
    mapdistF.at(b)->Write();
    mapdistG.at(b)->Write();
    mapdistFG.at(b)->Write();
    mapdistFGH.at(b)->Write();
  };

  printf("produced %s\n",outfile->GetName());
  outfile->Close();
};


///////////////////////////////////////////
// print table of mean values
void PrintMeans(TString title, std::map<Int_t,TH1D*> mapdist) {
  TString printStr = title + ": {";
  TH1D * totDist;
  TH1D * zeroDist;
  for(Int_t b : BS->binVec) {
    if(b==0) {
      zeroDist = mapdist.at(b);
      totDist = (TH1D*)zeroDist->Clone(TString("tot_"+TString(zeroDist->GetName())));
    } else {
      totDist->Add(mapdist.at(b));
    };
    printStr = Form("%s%s%.3f",
      printStr.Data(),
      b>0?", ":"",
      mapdist.at(b)->GetMean()
    );
    //if(title.Contains("2,0")) printf("%d & %.3f\n",b,mapdist.at(b)->GetMean()); // for latex
  };
  printStr += "}";
  printf("%s\n",printStr.Data());
  totDist->Write();

  // calculate total mean and RMS:
  printf("  mean = %f\n",totDist->GetMean());
  printf("  RMS  = %f\n",Tools::CalculateRMS(totDist));
};

///////////////////////////////////////////
// draw plot of `lambdaYval+-lambdaYerr` vs. `lambdaXval+-lambdaXerr`
void DrawPlot(
    TString name, TString titleX, TString titleY,
    std::function<Double_t(Int_t)> lambdaXval, std::function<Double_t(Int_t)> lambdaXerr,
    std::function<Double_t(Int_t)> lambdaYval, std::function<Double_t(Int_t)> lambdaYerr
    )
{
  TGraphErrors *gr = new TGraphErrors();
  gr->SetName(name);
  gr->SetTitle(titleY+" vs. "+titleX+";"+titleX+";"+titleY);
  gr->SetMarkerStyle(kFullCircle);
  gr->SetMarkerColor(kBlack);
  Int_t cnt = 0;
  for(Int_t b : BS->binVec) {
    gr->SetPoint( cnt, lambdaXval(b), lambdaYval(b) );
    gr->SetPointError( cnt, lambdaXerr(b), lambdaYerr(b) );
    cnt++;
  };
  gr->Write();
};

///////////////////////////////////////////
// set default arguments
void SetDefaultArgs() {
  inputData = "";
  pairType = EncodePairType(kPip,kPim);
  ivType = Binning::vM + 1;
};


// help printout
int PrintUsage() {

  SetDefaultArgs();
  BS = new Binning();
  fprintf(stderr,"\nUSAGE: TabulateBinMeans.exe [-f or -d input_data ] [options...]\n\n");

  printf("INPUT DATA:\n");
  printf(" -f\tsingle ROOT file\n");
  printf(" -d\tdirectory of ROOT files\n");
  printf(" NOTE: specify input with either -f or -d, but not both\n");
  printf("\n");

  printf("OPTIONS:\n");

  printf(" -p\tpair type, specified as a hexadecimal number\n");
  printf("   \trun PrintEnumerators.C for notation\n");
  printf("   \tdefault = 0x%x (%s)\n\n",pairType,PairTitle(pairType).Data());

  printf(" -i\tindependent variable specifier: 1, 2, or 3-digit number which\n");
  printf("   \tspecifies the independent variables that asymmetries will be\n");
  printf("   \tplotted against. The number of digits will be the number of\n");
  printf("   \tdimensions in the multi-dimensional binning\n");
  printf("   \t* the allowed digits are:\n");
  BS = new Binning();
  for(int i=0; i<Binning::nIV; i++) {
    printf("   \t  %d = %s\n",i+1,(BS->IVtitle[i]).Data());
  };
  printf("   \tdefault = %d\n\n",ivType);

  return 0;
};
