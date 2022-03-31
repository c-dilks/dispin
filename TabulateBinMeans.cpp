#include <cstdlib>
#include <iostream>
#include <vector>
#include <map>
#include <functional>
#include <utility>

// ROOT
#include "TFile.h"
#include "TTree.h"
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
#include "CatTree.h"

using std::map;
using std::pair;
using std::make_pair;
using std::function;

TString inputData;
Int_t pairType;
Int_t ivType;
Bool_t useEventTree;
TFile *outfile;
Binning * BS;
EventTree * ev;
map<TString,map<Int_t,TH1D*>> distHash;
Int_t nBins[3];

int PrintUsage();
void SetDefaultArgs();
void PrintMeans(TString title, map<Int_t,TH1D*> mapdist);
void WriteDist(TH1D *dist);
void VarLoop(function<void(TString)> lambda);
void DistListLoop(function<void(TString,map<Int_t,TH1D*>)> lambda);
void DistHashLoop(function<void(TString,Int_t,TH1D*)> lambda);
TString VarTitle(TString varName);
void DrawPlot(
    TString name, TString titleX, TString titleY,
    function<Double_t(Int_t)> lambdaXval, function<Double_t(Int_t)> lambdaXerr,
    function<Double_t(Int_t)> lambdaYval, function<Double_t(Int_t)> lambdaYerr,
    Int_t bn1=-1, Int_t bn2=-1
    );
Double_t CalculateDepolHere(Int_t twist, Int_t binNum);

int main(int argc, char** argv) {

  // read options
  SetDefaultArgs();
  int opt;
  enum inputType_enum {iFile,iDir};
  Int_t inputType = -1;
  Int_t nd=0;
  useEventTree = true;
  while( (opt=getopt(argc,argv,"c|f:d:p:i:n:")) != -1 ) {
    switch(opt) {
      case 'c': /* read catTree instead of outroot file EventTree */
        useEventTree = false;
        break;
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
      case 'n': /* number of bins for each independent variable */
        optind--;
        for( ; optind<argc && *argv[optind]!='-'; optind++) {
          if(nd<3) nBins[nd++] = (Int_t) strtof(argv[optind],NULL);
        };
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
  printf("nBins = ( %d, %d, %d )\n",nBins[0],nBins[1],nBins[2]);
  printf("useEventTree = %d\n",useEventTree);

  // set binning scheme
  BS = new Binning();
  Bool_t schemeSuccess = BS->SetScheme(ivType,nBins[0],nBins[1],nBins[2]);
  if(!schemeSuccess) {
    fprintf(stderr,"ERROR: Binning::SetScheme failed\n");
    return 0;
  };

  // instantiate EventTree or catTree
  // (use 1 file if inputType==iFile, 
  //  or all root files in inputData if inputType==iDir)
  if(useEventTree) ev = new EventTree(inputData+(inputType==iDir?"/*.root":""),pairType);
  else ev = new CatTree(inputData);

  // define output file
  TString outfileN = Form("meanvmean.%d.",ivType); 
  if(!useEventTree) outfileN += inputData;
  else outfileN += ".root";
  outfileN = "meanvmean/"+outfileN;
  outfile = new TFile(outfileN,"RECREATE");

  // define distHash = { varName => { binNum => distribution } }
  // - define variable names, each of which will be associated with a set of histograms
  // - useful if the variable names are consistent with `Binning`, do NOT use underscores
  distHash.insert( make_pair("Mh",       map<Int_t,TH1D*>{} ));
  distHash.insert( make_pair("X",        map<Int_t,TH1D*>{} ));
  distHash.insert( make_pair("Z",        map<Int_t,TH1D*>{} ));
  distHash.insert( make_pair("Q2",       map<Int_t,TH1D*>{} ));
  distHash.insert( make_pair("Y",        map<Int_t,TH1D*>{} ));
  distHash.insert( make_pair("PhPerp",   map<Int_t,TH1D*>{} ));
  distHash.insert( make_pair("DepolCA",  map<Int_t,TH1D*>{} ));
  distHash.insert( make_pair("DepolWA",  map<Int_t,TH1D*>{} ));
  distHash.insert( make_pair("DepolA",   map<Int_t,TH1D*>{} ));
  distHash.insert( make_pair("DepolC",   map<Int_t,TH1D*>{} ));
  distHash.insert( make_pair("DepolW",   map<Int_t,TH1D*>{} ));
  distHash.insert( make_pair("PhiH",     map<Int_t,TH1D*>{} ));
  distHash.insert( make_pair("PhiR",     map<Int_t,TH1D*>{} ));
  distHash.insert( make_pair("Theta",    map<Int_t,TH1D*>{} ));
  distHash.insert( make_pair("Mmiss",    map<Int_t,TH1D*>{} ));
  distHash.insert( make_pair("XF",       map<Int_t,TH1D*>{} ));
  distHash.insert( make_pair("DYsgn",    map<Int_t,TH1D*>{} ));
  distHash.insert( make_pair("Helicity", map<Int_t,TH1D*>{} ));
  if(useEventTree) {
    distHash.insert( make_pair("P0",      map<Int_t,TH1D*>{} ));
    distHash.insert( make_pair("P1",      map<Int_t,TH1D*>{} ));
    distHash.insert( make_pair("F",       map<Int_t,TH1D*>{} ));
    distHash.insert( make_pair("G",       map<Int_t,TH1D*>{} ));
    distHash.insert( make_pair("FG",      map<Int_t,TH1D*>{} ));
    distHash.insert( make_pair("FGH",     map<Int_t,TH1D*>{} ));
  };
  // - define histograms
  TString bStr;
  for(Int_t b : BS->binVec) {
    bStr = Form("Bin%d",b);
    distHash.at("Mh").insert(       make_pair(b, new TH1D(TString("Mh"+bStr),       TString("M_{h} for "+bStr),            100,  0,    3   )));
    distHash.at("X").insert(        make_pair(b, new TH1D(TString("X"+bStr),        TString("x for "+bStr),                100,  0,    1   )));
    distHash.at("Z").insert(        make_pair(b, new TH1D(TString("Z"+bStr),        TString("z for "+bStr),                100,  0,    1   )));
    distHash.at("Q2").insert(       make_pair(b, new TH1D(TString("Q2"+bStr),       TString("Q^{2} for "+bStr),            100,  0,    12  )));
    distHash.at("Y").insert(        make_pair(b, new TH1D(TString("Y"+bStr),        TString("y for "+bStr),                100,  0,    1   )));
    distHash.at("PhPerp").insert(   make_pair(b, new TH1D(TString("PhPerp"+bStr),   TString("p_{T} for "+bStr),            100,  0,    5   )));
    distHash.at("PhiH").insert(     make_pair(b, new TH1D(TString("PhiH"+bStr),     TString("#phi_{h} for "+bStr),         100,  -PI,  PI  )));
    distHash.at("PhiR").insert(     make_pair(b, new TH1D(TString("PhiR"+bStr),     TString("#phi_{R} for "+bStr),         100,  -PI,  PI  )));
    distHash.at("Theta").insert(    make_pair(b, new TH1D(TString("Theta"+bStr),    TString("#theta for "+bStr),           100,  0,    PI  )));
    distHash.at("Mmiss").insert(    make_pair(b, new TH1D(TString("Mmiss"+bStr),    TString("M_{X} for "+bStr),            100,  0,    3   )));
    distHash.at("XF").insert(       make_pair(b, new TH1D(TString("XF"+bStr),       TString("x_{F} for "+bStr),            300,  -1,   1   )));
    distHash.at("DYsgn").insert(    make_pair(b, new TH1D(TString("DYsgn"+bStr),    TString("#Delta Y_{h} for "+bStr),     100,  -10,  10  )));
    distHash.at("Helicity").insert( make_pair(b, new TH1D(TString("Helicity"+bStr), TString("helicity for "+bStr),         3,    -1.5, 1.5 )));
    distHash.at("DepolCA").insert(  make_pair(b, new TH1D(TString("DepolCA"+bStr),  TString("K_{2} for "+bStr),            1000, -10,  10  )));
    distHash.at("DepolWA").insert(  make_pair(b, new TH1D(TString("DepolWA"+bStr),  TString("K_{3} for "+bStr),            1000, -10,  10  )));
    distHash.at("DepolA").insert(   make_pair(b, new TH1D(TString("DepolA"+bStr),   TString("A(#varepsilon,y) for "+bStr), 1000, -10,  10  )));
    distHash.at("DepolC").insert(   make_pair(b, new TH1D(TString("DepolC"+bStr),   TString("C(#varepsilon,y) for "+bStr), 1000, -10,  10  )));
    distHash.at("DepolW").insert(   make_pair(b, new TH1D(TString("DepolW"+bStr),   TString("W(#varepsilon,y) for "+bStr), 1000, -10,  10  )));
    if(useEventTree) {
      distHash.at("P0").insert(     make_pair(b, new TH1D(TString("P0"+bStr),       TString("P_{2, 0}(cos#theta) for "+bStr), 1000, -0.6, 1.1 )));
      distHash.at("P1").insert(     make_pair(b, new TH1D(TString("P1"+bStr),       TString("sin#theta for "+bStr),           1000, -1.1, 1.1 )));
      distHash.at("F").insert(      make_pair(b, new TH1D(TString("F"+bStr),        TString("F for "+bStr),                   1000, -1.1, 1.1 )));
      distHash.at("G").insert(      make_pair(b, new TH1D(TString("G"+bStr),        TString("G for "+bStr),                   1000, -1.1, 1.1 )));
      distHash.at("FG").insert(     make_pair(b, new TH1D(TString("FG"+bStr),       TString("FG for "+bStr),                  1000, -1.1, 1.1 )));
      distHash.at("FGH").insert(    make_pair(b, new TH1D(TString("FGH"+bStr),      TString("FGH for "+bStr),                 1000, -1.1, 1.1 )));
    };
  };

  // histogram settings
  DistHashLoop( [&](TString varName, Int_t binNum, TH1D *dist){
      dist->StatOverflows(kTRUE);
      });


  //-----------------------------------------------------
  // EVENT LOOP  
  //-----------------------------------------------------
  Int_t bn;
  Float_t iv;
  Bool_t valid;
  printf("begin loop through %lld events...\n",ev->ENT);
  for(Long64_t i=0; i<ev->ENT; i++) {
    // if(i>100000) break; // limiter
    ev->GetEvent(i);

    valid = useEventTree ? ev->Valid() : true;
    if(valid) {

      // get bin number
      bn = BS->FindBin(ev);

      // fill histograms
      distHash.at("Mh").at(bn)->Fill(ev->Mh,ev->GetWeight());
      distHash.at("X").at(bn)->Fill(ev->x,ev->GetWeight());
      distHash.at("Z").at(bn)->Fill(ev->Zpair,ev->GetWeight());
      distHash.at("Q2").at(bn)->Fill(ev->Q2,ev->GetWeight());
      distHash.at("Y").at(bn)->Fill(ev->y,ev->GetWeight());
      distHash.at("PhPerp").at(bn)->Fill(ev->PhPerp,ev->GetWeight());
      distHash.at("PhiH").at(bn)->Fill(ev->PhiH,ev->GetWeight());
      distHash.at("PhiR").at(bn)->Fill(ev->PhiR,ev->GetWeight());
      distHash.at("Theta").at(bn)->Fill(ev->theta,ev->GetWeight());
      distHash.at("Mmiss").at(bn)->Fill(ev->Mmiss,ev->GetWeight());
      distHash.at("XF").at(bn)->Fill(ev->xF,ev->GetWeight());
      distHash.at("DYsgn").at(bn)->Fill(ev->DYsgn,ev->GetWeight());
      distHash.at("Helicity").at(bn)->Fill(ev->GetSpinIdx(),ev->GetWeight());
      distHash.at("DepolCA").at(bn)->Fill(ev->GetDepol2(),ev->GetWeight());
      distHash.at("DepolWA").at(bn)->Fill(ev->GetDepol3(),ev->GetWeight());
      distHash.at("DepolA").at(bn)->Fill(ev->GetDepolarizationFactor('A'),ev->GetWeight());
      distHash.at("DepolC").at(bn)->Fill(ev->GetDepolarizationFactor('C'),ev->GetWeight());
      distHash.at("DepolW").at(bn)->Fill(ev->GetDepolarizationFactor('W'),ev->GetWeight());
      if(useEventTree) {
        distHash.at("P0").at(bn)->Fill(0.5*(3*TMath::Power(TMath::Cos(ev->theta),2)-1),ev->GetWeight());
        distHash.at("P1").at(bn)->Fill(TMath::Sin(ev->theta),ev->GetWeight());
        distHash.at("F").at(bn)->Fill(TMath::Cos(ev->PhiH),ev->GetWeight());
        distHash.at("G").at(bn)->Fill(TMath::Sin(ev->PhiR),ev->GetWeight());
        distHash.at("FG").at(bn)->Fill(TMath::Sin(2*ev->PhiH-ev->PhiR)*TMath::Sin(ev->PhiH),ev->GetWeight());
        distHash.at("FGH").at(bn)->Fill( TMath::Sin(ev->PhiH-ev->PhiR) * TMath::Sin(ev->PhiH-ev->PhiR) * TMath::Cos(ev->PhiH) ,ev->GetWeight());
      }
    };
  };
  // END EVENT LOOP /////////////


  // lambda: calculate error propagation of a*b or a/b
  auto propErr = [&](Double_t a, Double_t aErr, Double_t b, Double_t bErr){ return a/b * TMath::Hypot(aErr/a,bErr); };

  // lambda: draw mean of yDist vs. mean of xDist
  // - set bn1 or bn2 to restrict higher dimensional bins
  auto DrawMeanVsMean = [&](
      TString name,
      TString titleX, TString titleY,
      map<Int_t,TH1D*> xDistList,
      map<Int_t,TH1D*> yDistList,
      Int_t bn1=-1, Int_t bn2=-1
      ) {
    DrawPlot(name,titleX,titleY,
        [&](Int_t b){ return xDistList.at(b)->GetMean();      },
        [&](Int_t b){ return xDistList.at(b)->GetMeanError(); },
        [&](Int_t b){ return yDistList.at(b)->GetMean();      },
        [&](Int_t b){ return yDistList.at(b)->GetMeanError(); },
        bn1, bn2
        );
  };
    

  // draw plots (usually mean vs. mean) /////////////////////////////////

  // lambda: loop over each variable in distHash, drawing its mean vs. each bin mean in the Binning scheme
  auto LoopDrawMeanVsMean = [&](Int_t bn1=-1, Int_t bn2=-1){
    // set plot name
    TString plotNsuffix = "_vs_" + BS->GetIVname(0);
    if(bn1>=0) plotNsuffix += "_" + BS->GetIVname(1) + Form("Bin%d",bn1);
    if(bn2>=0) plotNsuffix += "_" + BS->GetIVname(2) + Form("Bin%d",bn2);
    // lambda for drawing the plot with DrawPlot
    auto lambdaDrawPlot = [&](TString varName) {
      enum xy {kx,ky};
      TString varN[2];
      varN[kx] = BS->GetIVname(0);
      varN[ky] = varName;
      map<Int_t,TH1D*> dists[2];
      // set lambda functions for values and errors for DrawPlot
      function<Double_t(Int_t)> lambdaVal[2];
      function<Double_t(Int_t)> lambdaErr[2];
      for(int k=0; k<2; k++) { // loop over x and y
        dists[k] = distHash.at(varN[k]);
        if( varN[k].Contains("Phi") || varN[k]=="Theta" ) { // if angle distribution
          if(k==kx) { // (lambdaVal and lambdaErr cannot contain `k` in the body)
            lambdaVal[k] = [&](Int_t b){ return Tools::AdjAngleThreeQuarters(Tools::MeanAngle(dists[kx].at(b))); }; // mean of angles
            lambdaErr[k] = [&](Int_t b){ return 0.0; /*TODO*/ };
          } else {
            lambdaVal[k] = [&](Int_t b){ return Tools::AdjAngleThreeQuarters(Tools::MeanAngle(dists[ky].at(b))); }; // mean of angles
            lambdaErr[k] = [&](Int_t b){ return 0.0; /*TODO*/ };
          };
        } else { // if not angle distribution
          if(k==kx) {
            lambdaVal[k] = [&](Int_t b){ return dists[kx].at(b)->GetMean(); }; // usual mean
            lambdaErr[k] = [&](Int_t b){ return dists[kx].at(b)->GetMeanError(); }; 
          } else {
            lambdaVal[k] = [&](Int_t b){ return dists[ky].at(b)->GetMean(); }; // usual mean
            lambdaErr[k] = [&](Int_t b){ return dists[ky].at(b)->GetMeanError(); }; 
          };
        };
      };
      // draw
      DrawPlot(
          varName + plotNsuffix,
          VarTitle(varN[kx]), VarTitle(varN[ky]),
          lambdaVal[kx], lambdaErr[kx],
          lambdaVal[ky], lambdaErr[ky],
          bn1, bn2
          );
    };
    // execution: call lambdaDrawPlot for each variable
    VarLoop(lambdaDrawPlot);
  };

  // call LoopDrawMeanVsMean for each higher-dimensional bin
  switch(BS->dimensions) {
    case 1:
      LoopDrawMeanVsMean();
      break;
    case 2:
      for(Int_t bn1=0; bn1<BS->GetNbins(1); bn1++) LoopDrawMeanVsMean(bn1);
      break;
    case 3:
      for(Int_t bn2=0; bn2<BS->GetNbins(2); bn2++) {
        for(Int_t bn1=0; bn1<=BS->GetNbins(1); bn1++) LoopDrawMeanVsMean(bn1,bn2);
      };
      break;
  };


  // draw depolarization plots, using various methods for the averaging
  // depol. C/A vs. X
  DrawPlot("depol2_via_ave1","<"+BS->GetIVtitle(0)+">","<C>/<A>",
      [&](Int_t b){ return distHash.at(BS->GetIVname(0)).at(b)->GetMean(); },
      [&](Int_t b){ return distHash.at(BS->GetIVname(0)).at(b)->GetMeanError(); },
      [&](Int_t b){ return distHash.at("DepolC").at(b)->GetMean() / distHash.at("DepolA").at(b)->GetMean(); },
      [&](Int_t b){ return propErr(
        distHash.at("DepolC").at(b)->GetMean(), distHash.at("DepolC").at(b)->GetMeanError(),
        distHash.at("DepolA").at(b)->GetMean(), distHash.at("DepolA").at(b)->GetMeanError()
        );}
      );
  DrawMeanVsMean("depol2_via_ave2","<"+BS->GetIVtitle(0)+">","<C/A>",
      distHash.at(BS->GetIVname(0)),
      distHash.at("DepolCA")
      );
  DrawPlot("depol2_via_ave3","<"+BS->GetIVtitle(0)+">","C(<x>,<y>,<Q2>)/A(<x>,<y>,<Q2>)",
      [&](Int_t b){ return distHash.at(BS->GetIVname(0)).at(b)->GetMean(); },
      [&](Int_t b){ return distHash.at(BS->GetIVname(0)).at(b)->GetMeanError(); },
      [&](Int_t b){ return CalculateDepolHere(2,b); },
      [&](Int_t b){ return 0.0; } // TODO
      );
  // depol. W/A vs. X
  DrawPlot("depol3_via_ave1","<"+BS->GetIVtitle(0)+">","<W>/<A>",
      [&](Int_t b){ return distHash.at(BS->GetIVname(0)).at(b)->GetMean(); },
      [&](Int_t b){ return distHash.at(BS->GetIVname(0)).at(b)->GetMeanError(); },
      [&](Int_t b){ return distHash.at("DepolW").at(b)->GetMean() / distHash.at("DepolA").at(b)->GetMean(); },
      [&](Int_t b){ return propErr(
        distHash.at("DepolW").at(b)->GetMean(), distHash.at("DepolW").at(b)->GetMeanError(),
        distHash.at("DepolA").at(b)->GetMean(), distHash.at("DepolA").at(b)->GetMeanError()
        );}
      );
  DrawMeanVsMean("depol3_via_ave2","<"+BS->GetIVtitle(0)+">","<W/A>",
      distHash.at(BS->GetIVname(0)),
      distHash.at("DepolWA")
      );
  DrawPlot("depol3_via_ave3","<"+BS->GetIVtitle(0)+">","W(<x>,<y>,<Q2>)/A(<x>,<y>,<Q2>)",
      [&](Int_t b){ return distHash.at(BS->GetIVname(0)).at(b)->GetMean(); },
      [&](Int_t b){ return distHash.at(BS->GetIVname(0)).at(b)->GetMeanError(); },
      [&](Int_t b){ return CalculateDepolHere(3,b); },
      [&](Int_t b){ return 0.0; } // TODO
      );


  // print means for cross check
  DistListLoop( [&](TString varName, map<Int_t,TH1D*> distList){ PrintMeans(varName,distList); });

  // write histograms
  DistHashLoop( [&](TString varName, Int_t binNum, TH1D *dist){ WriteDist(dist); });

  // close
  printf("produced %s\n",outfile->GetName());
  outfile->Close();
};


///////////////////////////////////////////
// print table of mean values
void PrintMeans(TString title, map<Int_t,TH1D*> mapdist) {
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
  if(totDist->GetEntries()==0) return; // do not print anything if there are no entries
  printf("%s\n",printStr.Data());
  totDist->Write();

  // calculate total mean and RMS:
  printf("  mean = %f\n",totDist->GetMean());
  printf("  RMS  = %f\n",Tools::CalculateRMS(totDist));
};

///////////////////////////////////////////
// write distribution, if it is not empty
void WriteDist(TH1D *dist) {
  if(dist->GetEntries()>0) dist->Write();
};

///////////////////////////////////////////
// loop through variables of distHash, executing lambda(varName) for each variable
void VarLoop(function<void(TString)> lambda) {
  for(auto const& kv : distHash) {
    auto varName = kv.first;
    lambda(varName);
  };
};

///////////////////////////////////////////
// loop through variables of distHash, executing lambda(varName,distList) for each variable
void DistListLoop(function<void(TString,map<Int_t,TH1D*>)> lambda) {
  for(auto const& kv : distHash) {
    auto varName  = kv.first;
    auto distList = kv.second;
    lambda(varName,distList);
  };
};

///////////////////////////////////////////
// loop through distHash, executing lambda(varName,binNum,dist) for each distribution
void DistHashLoop(function<void(TString,Int_t,TH1D*)> lambda) {
  for(auto const& kv : distHash) {
    auto varName  = kv.first;
    auto distList = kv.second;
    for(auto const& kv2 : distList) {
      auto binNum = kv2.first;
      auto dist   = kv2.second;
      lambda(varName,binNum,dist);
    };
  };
};

///////////////////////////////////////////
// draw plot of `lambdaYval+-lambdaYerr` vs. `lambdaXval+-lambdaXerr`
void DrawPlot(
    TString name, TString titleX, TString titleY,
    function<Double_t(Int_t)> lambdaXval, function<Double_t(Int_t)> lambdaXerr,
    function<Double_t(Int_t)> lambdaYval, function<Double_t(Int_t)> lambdaYerr,
    Int_t bn1, Int_t bn2
    )
{
  TGraphErrors *gr = new TGraphErrors();
  gr->SetName(name);
  gr->SetTitle(titleY+" vs. "+titleX+";"+titleX+";"+titleY);
  gr->SetMarkerStyle(kFullCircle);
  gr->SetMarkerColor(kBlack);
  Int_t cnt = 0;
  for(Int_t b : BS->binVec) {
    if(bn1>=0 && bn1!=BS->UnhashBinNum(b,1)) continue; // if bn1>=0, restrict iv[1] bin to be bn1
    if(bn2>=0 && bn2!=BS->UnhashBinNum(b,2)) continue; // ditto for bn2
    gr->SetPoint( cnt, lambdaXval(b), lambdaYval(b) );
    gr->SetPointError( cnt, lambdaXerr(b), lambdaYerr(b) );
    cnt++;
  };
  gr->Write();
};

///////////////////////////////////////////
// get varTitle given varName
TString VarTitle(TString varName) {
  TString ret = distHash.at(varName).at(0)->GetTitle();
  ret(TRegexp(" for.*$")) = "";
  return ret;
};

///////////////////////////////////////////
// set default arguments
void SetDefaultArgs() {
  inputData = "";
  pairType = EncodePairType(kPip,kPim);
  ivType = Binning::vM + 1;
  for(int d=0; d<3; d++) nBins[d] = -1;
};

///////////////////////////////////////////
// local depolarization calculation, for testing depolarization averaging
Double_t CalculateDepolHere(Int_t twist, Int_t binNum) {
  Double_t meanQ2 = distHash.at("Q2").at(binNum)->GetMean();
  Double_t meanX  = distHash.at("X").at(binNum)->GetMean();
  Double_t meanY  = distHash.at("Y").at(binNum)->GetMean();
  Double_t gamma = 2*PartMass(kP)*meanX / TMath::Sqrt(meanQ2);
  Double_t epsilon = ( 1 - meanY - TMath::Power(gamma*meanY,2)/4 ) /
    ( 1 - meanY + meanY*meanY/2 + TMath::Power(gamma*meanY,2)/4 );
  Double_t depolA = meanY*meanY / (2 - 2*epsilon); // A(x,y)
  Double_t depolC = depolA * TMath::Sqrt(1-epsilon*epsilon);
  Double_t depolW = depolA * TMath::Sqrt(2*epsilon*(1-epsilon));
  if(depolA==0) {
    fprintf(stderr,"ERROR: depolA=0, setting depol=1.0 for this bin\n");
    return 1.0;
  } else {
    switch(twist) {
      case 2: return depolC/depolA; break;
      case 3: return depolW/depolA; break;
    };
  };
  fprintf(stderr,"ERROR: depol calculation failed\n");
  return 1.0;
};


///////////////////////////////////////////
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
  printf(" -c\tread a CatTree instead of EventTree (faster, but less plots are made)\n");
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

  printf(" -n\tnumber of bins, listed for each independent variable,\n");
  printf("   \tseparated by spaces\n\n");

  return 0;
};
