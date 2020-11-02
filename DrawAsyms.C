// draw individual asymmetry results
// - this was used for DNP2020 partial waves

R__LOAD_LIBRARY(DiSpin)
#include "Binning.h"

void DrawAsyms(TString infileN="spinroot_pw_3/asym_9_mlm.root") {

  // read asymmetry graphs
  TFile * infile = new TFile(infileN,"READ");

  TListIter nextKey(infile->GetListOfKeys());
  TString keyname;
  TGraphAsymmErrors * gr;

  TCanvas * canv = new TCanvas("canv","canv",1000,800);
  TString pngname;
  TLine * zero;
  Int_t amp;
  char noop[32];
  TString mod,title;

  while(TKey * key = (TKey*) nextKey()) {
    keyname = TString(key->GetName());
    // read asymmetry graph
    if(keyname.Contains(TRegexp("^kindepMA")) &&
           !keyname.Contains("Canv")) {
      gr = (TGraphAsymmErrors*) key->ReadObj();

      sscanf(keyname.Data(),"kindepMA_A%d_%s",&amp,noop);
      switch(amp) {
        case 0: mod="sin(#phi_{h})"; break;
        case 1: mod="cos(#theta) sin(#phi_{h})"; break;
        case 2: mod="sin(#theta) sin(#phi_{h}-#phi_{R})"; break;
        case 3: mod="sin(#theta) sin(#phi_{R})"; break;
        case 4: mod="sin(#theta) sin(2#phi_{h}-#phi_{R})"; break;
        case 5: mod="1/2 (3cos^{2}#theta-1) sin(#phi_{h})"; break;
        case 6: mod="sin(2#theta) sin(#phi_{h}-#phi_{R})"; break;
        case 7: mod="sin(2#theta) sin(#phi_{R})"; break;
        case 8: mod="sin(2#theta) sin(2#phi_{h}-#phi_{R})"; break;
        case 9: mod="sin^{2}(#theta) sin(2#phi_{h}-2#phi_{R})"; break;
        case 10: mod="sin^{2}(#theta) sin(-#phi_{h}+2#phi_{R})"; break;
        case 11: mod="sin^{2}(#theta) sin(3#phi_{h}-2#phi_{R})"; break;
      };
      title = gr->GetTitle();
      mod = "A_{LU}^{"+mod+"} vs.";
      title(TRegexp("A_{LU}.* vs.")) = mod;
      gr->SetTitle(title);
      gr->GetYaxis()->SetTitle("A_{LU}");

      gStyle->SetTitleBorderSize(4);
      gStyle->SetTitleSize(0.04,"main");
      gr->GetYaxis()->SetTitleOffset(1.3);
      gr->GetXaxis()->SetTitleSize(0.04);
      gr->GetYaxis()->SetTitleSize(0.04);

      gr->GetYaxis()->SetRangeUser(-0.1,0.1);


      gr->SetMarkerStyle(kFullCircle);
      gr->SetMarkerColor(kBlack);
      gr->SetMarkerSize(1.75);
      gr->SetLineColor(kBlack);
      gr->SetLineWidth(3);


      zero = new TLine(gr->GetXaxis()->GetXmin(),0,
                       gr->GetXaxis()->GetXmax(),0);
      zero->SetLineStyle(kDashed);
      zero->SetLineColor(kGray+2);
      zero->SetLineWidth(2);


      pngname = keyname + ".png";
      gr->Draw("APE");
      zero->Draw();
      canv->Print(pngname,"png");
    };
  };
};
