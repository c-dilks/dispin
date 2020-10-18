// tabulate asymmetry amplitudes, uncertainty, and IV plotting position
// e.g. the bin mean)

R__LOAD_LIBRARY(DiSpin)
#include "Binning.h"

void TabulateAsym(TString infileN="spinroot/asym_42.root") {

  // read asymmetry graphs
  TFile * infile = new TFile(infileN,"READ");

  TListIter nextKey(infile->GetListOfKeys());
  TString keyname;
  TObjArray * asymArr = new TObjArray();
  TGraphAsymmErrors * gr;

  Double_t x,y,ex,ey;

  while(TKey * key = (TKey*) nextKey()) {
    keyname = TString(key->GetName());
    // read asymmetry graph
    if(keyname.Contains(TRegexp("^kindepMA")) &&
           !keyname.Contains("Canv")) {
      gr = (TGraphAsymmErrors*) key->ReadObj();
      asymArr->AddLast(gr);
    };
  };


  TObjArrayIter nextGr(asymArr);
  while(TGraphAsymmErrors * gr = (TGraphAsymmErrors*) nextGr()) {
    printf("\n%s\n",gr->GetTitle());
    for(int i=0; i<gr->GetN(); i++) {
      gr->GetPoint(i,x,y);
      ex = gr->GetErrorX(i); // (parabolic error)
      ey = gr->GetErrorY(i); // (parabolic error)
      printf("%.5f %.5f +- %.5f\n",x,y,ey);
    };
  };
};
