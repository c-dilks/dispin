/* read outroot and radroot files, simultaneously, and
 * determine systematic uncertainty from radiative corrections
 */

#include <cstdlib>
#include <iostream>
#include <map>

// ROOT
#include "TFile.h"
#include "TString.h"
#include "TRegexp.h"
#include "TROOT.h"

// Dispin
#include "Constants.h"
#include "EventTree.h"
#include "Binning.h"

using std::cout;
using std::cerr;
using std::endl;
using std::map;
using std::pair;

// argument variables
TString inFiles[2];
TString outFileN;
Int_t pairType;
Int_t nBins[3];
Int_t ivType;
Long64_t limiter;

// global variables
Binning *BS;
EventTree *ev[2];
TFile *outFile;
enum outrad { nom, rad }; // nominal, RC-modified
enum histEnum {fir,ful,all,nH}; // see histogram types below

// subroutines
void SetDefaultArgs();
int PrintUsage();
TString RX(TString s, TString v);
void WritePlots(TH2D **hist);

//////////////////////////////////////


int main(int argc, char** argv) {

  // read options
  SetDefaultArgs();
  int opt;
  enum inputType_enum {iFile,iDir};
  Int_t inputType = -1;
  Int_t nd=0;
  while( (opt=getopt(argc,argv,"f:d:o:p:i:n:l:")) != -1 ) {
    switch(opt) {
      case 'f': /* input file */
        if(inputType>=0) return PrintUsage();
        inFiles[nom] = optarg;
        inputType = iFile;
        break;
      case 'd': /* input directory */
        if(inputType>=0) return PrintUsage();
        inFiles[nom] = optarg;
        inputType = iDir;
        break;
      case 'o': /* output file name */
        outFileN = optarg;
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
      case 'l': /* limiter */
        limiter = (Long64_t) strtof(optarg,NULL);
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

  // determine radroot file/directory path
  inFiles[rad] = inFiles[nom];
  inFiles[rad](TRegexp("^outroot")) = "radroot";

  // print arguments' values
  Tools::PrintSeparator(40,"=");
  printf("outrootFiles = %s\n",inFiles[nom].Data());
  printf("radrootFiles = %s\n",inFiles[rad].Data());
  printf("pairType = 0x%x\n",pairType);
  printf("ivType = %d\n",ivType);
  printf("nBins = ( %d, %d, %d )\n",nBins[0],nBins[1],nBins[2]);
  Tools::PrintSeparator(40,"=");

  // set binning scheme
  BS = new Binning();
  Bool_t schemeSuccess = BS->SetScheme(pairType,ivType,nBins[0],nBins[1],nBins[2]);
  if(!schemeSuccess) {
    fprintf(stderr,"ERROR: Binning::SetScheme failed\n");
    return 0;
  };

  // instantiate EventTree for outroot and radroot files
  // (use 1 file if inputType==iFile, or all root files in inFiles if inputType==iDir)
  for(int f=0; f<2; f++)
    ev[f] = new EventTree(inFiles[f]+(inputType==iDir?"/*.root":""),pairType);

  // define output file
  if(outFileN=="") {
    gROOT->ProcessLine(".! mkdir -p radiative");
    if(inputType==iDir) {
      outFileN = "radiative/tree";
      outFileN += "_" + PairName(pairType);
      for(int d=0; d<BS->dimensions; d++) outFileN += "_" + BS->GetIVname(d);
      outFileN += ".root";
    } else if(inputType==iFile) {
      outFileN = inFiles[nom];
      outFileN(TRegexp("^.*/")) = "radiative/tree.";
    };
  }
  printf("\nCREATING TREE FILE = %s\n\n",outFileN.Data());
  outFile = new TFile(outFileN,"RECREATE");


  //-----------------------------------------------------
  // output data structures
  //-----------------------------------------------------

  // build map of 3-digit bin number -> object
  map<Int_t,Long64_t> countHash[nH];
  map<Int_t,TH1*> ivDistHash;
  Float_t ivVal[3] = {-10000.0,-10000.0,-10000.0};
  for(Int_t bn : BS->binVec) {

    // counts
    for(int h=0; h<nH; h++) countHash[h].insert(pair<Int_t,Long64_t>(bn,0));

    // iv distributions
    TString ivDistN = Form("iv_%d",bn);
    TString ivDistT = "iv dist";
    for(int d=0; d<BS->dimensions; d++) ivDistT += ";" + BS->GetIVtitle(d);
    switch(BS->dimensions) {
      case 1:
        ivDistHash.insert(pair<Int_t,TH1D*>( bn, new TH1D(ivDistN, ivDistT,
                500, BS->GetIVmin(0), BS->GetIVmax(0)
                )));
        break;
      case 2:
        ivDistHash.insert(pair<Int_t,TH2D*>( bn, new TH2D(ivDistN, ivDistT,
                500, BS->GetIVmin(0), BS->GetIVmax(0),
                500, BS->GetIVmin(1), BS->GetIVmax(1)
                )));
        break;
      case 3:
        ivDistHash.insert(pair<Int_t,TH3D*>( bn, new TH3D(ivDistN, ivDistT,
                500, BS->GetIVmin(0), BS->GetIVmax(0),
                500, BS->GetIVmin(1), BS->GetIVmax(1),
                500, BS->GetIVmin(2), BS->GetIVmax(2)
                )));
        break;
    };
  };

  // 2D histograms of RC-modified vs. nominal
  /* histogram types:
   * - FIR: RC-modified value From Invalid Region (rejected), nominal value from allowed region
   * - FUL: nominal value from allowed region, don't care about RC-modified value
   * - ALL: allow all events, no cuts
   */
  TString histT[nH], histN[nH];
  histT[fir] = "VAR(nom) allowed, VAR(mod) rejected";
  histT[ful] = "VAR(nom) allowed";
  histT[all] = "VAR correlation, no cuts";
  histN[fir] = "fir";
  histN[ful] = "ful";
  histN[all] = "all";

  TH2D *histMmiss[nH];
  TH2D *histX[nH];
  TH2D *histQ2[nH];
  TH2D *histMh[nH];
  TH2D *histZ[nH];
  TH2D *histPhiH[nH];
  TH2D *histPhiR[nH];
  TH2D *histTheta[nH];
  TH2D *histXF[nH];
  TH2D *histPT[nH];

  for(int h=0; h<nH; h++) {
    TString hT = histT[h] + ";VAR(nom);VAR(mod)";
    TString hN = "VAR_correlation_" + histN[h];
    histMmiss[h] = new TH2D( RX(hN,"Mmiss"), RX(hT,"M_{X}"),    50, 0,   5,   50, 0,   5   );
    histX[h]     = new TH2D( RX(hN,"X"),     RX(hT,"x"),        50, 0,   1,   50, 0,   1   );
    histQ2[h]    = new TH2D( RX(hN,"Q2"),    RX(hT,"Q^{2}"),    50, 0,   10,  50, 0,   10  );
    histMh[h]    = new TH2D( RX(hN,"Mh"),    RX(hT,"M_{h}"),    50, 0,   2.5, 50, 0,   2.5 );
    histZ[h]     = new TH2D( RX(hN,"Z"),     RX(hT,"z"),        50, 0,   1,   50, 0,   1   );
    histPhiH[h]  = new TH2D( RX(hN,"PhiH"),  RX(hT,"#phi_{h}"), 50, -PI, PI,  50, -PI, PI  );
    histPhiR[h]  = new TH2D( RX(hN,"PhiR"),  RX(hT,"#phi_{R}"), 50, -PI, PI,  50, -PI, PI  );
    histTheta[h] = new TH2D( RX(hN,"Theta"), RX(hT,"#theta"),   50, 0,   PI,  50, 0,   PI  );
    histXF[h]    = new TH2D( RX(hN,"XF"),    RX(hT,"x_{F}"),    50, -1,  1,   50, -1,  1   );
    histPT[h]    = new TH2D( RX(hN,"PT"),    RX(hT,"p_{T}"),    50, 0,   2.5, 50, 0,   2.5 );
  };

  auto FillData = [&](Int_t hh, Int_t bb) {
    histMmiss[hh]-> Fill( ev[nom]->Mmiss,  ev[rad]->Mmiss  );
    histX[hh]->     Fill( ev[nom]->x,      ev[rad]->x      );
    histQ2[hh]->    Fill( ev[nom]->Q2,     ev[rad]->Q2     );
    histMh[hh]->    Fill( ev[nom]->Mh,     ev[rad]->Mh     );
    histZ[hh]->     Fill( ev[nom]->Zpair,  ev[rad]->Zpair  );
    histPhiH[hh]->  Fill( ev[nom]->PhiH,   ev[rad]->PhiH   );
    histPhiR[hh]->  Fill( ev[nom]->PhiR,   ev[rad]->PhiR   );
    histTheta[hh]-> Fill( ev[nom]->theta,  ev[rad]->theta  );
    histXF[hh]->    Fill( ev[nom]->xF,     ev[rad]->xF     );
    histPT[hh]->    Fill( ev[nom]->PhPerp, ev[rad]->PhPerp );
    if(bb>0) countHash[hh].at(bb)++;
  };



  //-----------------------------------------------------
  // EVENT LOOP  
  //-----------------------------------------------------
  printf("begin loop through %lld events...\n",ev[nom]->ENT);
  for(int i=0; i<ev[nom]->ENT; i++) {

    ev[nom]->GetEvent(i);
    ev[rad]->GetEvent(i);

    // check nominal values
    if(ev[nom]->Valid()) {

      // truncate at limit
      if(limiter>0 && i>limiter) break;

      // fill iv dists
      Int_t binnum = BS->FindBin(ev[nom]); // binning is according to the nominal value
      for(int d=0; d<BS->dimensions; d++) ivVal[d] = BS->GetIVval(d);
      switch(BS->dimensions) {
        case 1: ((TH1D*)ivDistHash.at(binnum))->Fill(ivVal[0]);                   break;
        case 2: ((TH2D*)ivDistHash.at(binnum))->Fill(ivVal[0],ivVal[1]);          break;
        case 3: ((TH3D*)ivDistHash.at(binnum))->Fill(ivVal[0],ivVal[1],ivVal[2]); break;
      };

      // friend check: make sure outroot and radroot trees are synced by checking
      // that both are currently reading the same dihadron
      // - we do not use TTree friends, rather EventTrees for better control
      if( ev[nom]->runnum     != ev[rad]->runnum     ||
          ev[nom]->evnum      != ev[rad]->evnum      ||
          ev[nom]->hadRow[qA] != ev[rad]->hadRow[qA] ||
          ev[nom]->hadRow[qB] != ev[rad]->hadRow[qB]
        )
      {
        fprintf(stderr,"ERROR: event mis-match\n");
        return 1;
      }

      // FIR cut, where RC-modified value is "FIR" = From Invalid Region
      Bool_t firCut;
      // firCut = ! ev[rad]->Valid();
      firCut = ev[rad]->Mmiss < 1.5;

      // fill data structures
      FillData(ful,binnum);
      if(firCut) FillData(fir,binnum);

    }; // end if(ev[nom]->Valid())

    FillData(all,-1); // fill no-cuts data structures

  }; // end EVENT LOOP


  // write out to output file
  WritePlots(histMmiss);
  WritePlots(histX);
  WritePlots(histQ2);
  WritePlots(histMh);
  WritePlots(histZ);
  WritePlots(histPhiH);
  WritePlots(histPhiR);
  WritePlots(histTheta);
  WritePlots(histXF);
  WritePlots(histPT);
  for(Int_t bn : BS->binVec) ivDistHash.at(bn)->Write();
  outFile->Close();
  printf("wrote %s\n",outFileN.Data());
  return 0;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


// set default arguments
void SetDefaultArgs() {
  for(int f=0; f<2; f++) inFiles[f] = "";
  outFileN = "";
  pairType = EncodePairType(kPip,kPim);
  ivType = Binning::vM + 1;
  limiter = 0;
  for(int d=0; d<3; d++) nBins[d] = -1;
};


// help printout
int PrintUsage() {

  SetDefaultArgs();
  BS = new Binning();
  fprintf(stderr,"\nUSAGE: buildSpinroot.exe [-f or -d input_data ] [options...]\n\n");

  printf("INPUT DATA:\n");
  printf(" -f\tsingle ROOT file\n");
  printf(" -d\tdirectory of ROOT files\n");
  printf(" NOTE: specify input with either -f or -d, but not both\n");
  printf("\n");

  printf("OPTIONS:\n");

  printf(" -o\toutput file name\n");
  printf("   \tdefault is a name based on the input file or directory name\n\n");

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

  printf(" -l\tlimiter, truncate to this many events (default take all)\n\n");

  return 0;
};


// return copy of s, with "VAR" replaced with v
TString RX(TString s, TString v) {
  TString r=s;
  r.ReplaceAll("VAR",v);
  return r;
};

// write 2D histograms, and include 1D projections
void WritePlots(TH2D **hist) {
  for(int h=0; h<nH; h++) hist[h]->Write();
  TH1D *proj[2][nH]; // [nom,rad] [histogram type]
  TString xT = hist[0]->GetXaxis()->GetTitle();
  xT(TRegexp("nom")) = "blue=nom,red=mod";
  for(int h=0; h<nH; h++) {
    proj[nom][h] = hist[h]->ProjectionX();
    proj[rad][h] = hist[h]->ProjectionY();
    proj[nom][h]->SetLineColor(kBlue+3);
    proj[rad][h]->SetLineColor(kRed-7);
    proj[nom][h]->GetXaxis()->SetTitle(xT);
    TString canvN = TString("canv_") + hist[h]->GetName();
    TCanvas *canv = new TCanvas(canvN,canvN);
    proj[nom][h]->Draw();
    proj[rad][h]->Draw("same");
    canv->Write();
  };
};
