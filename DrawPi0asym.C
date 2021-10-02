R__LOAD_LIBRARY(DiSpin)
void DrawPi0asym(TString infileN="bruspin.sfit.x/asym_minuit_BL0.root",TString axisTitle="x") {
  TFile *infile = new TFile(infileN,"READ");

  TGraphErrors *gr;
  TLine *zeroLine;
  int cnt=0;

  Double_t asymMax=0.07;
  Double_t asymMin = -asymMax;

  const int N=3;
  TCanvas *canv = new TCanvas("asym","asym",N*800,800);
  canv->Divide(N,1);

  for(TString grN :
      {
        "gr_AmpT2L1Mp1Lv0P0_BL0",
        "gr_AmpT2L2Mp2Lv0P0_BL0",
        "gr_AmpT3L1Mp1Lv0P0_BL0"
      })
  {
    canv->cd(++cnt);
    canv->GetPad(cnt)->SetBottomMargin(0.15);
    canv->GetPad(cnt)->SetLeftMargin(0.15);
    gr = (TGraphErrors*) infile->Get(grN);
    TString grT = gr->GetTitle();
    Tools::GlobalRegexp(grT,"X","x");
    Tools::GlobalRegexp(grT,"Z","z");
    Tools::GlobalRegexp(grT,"Mh","M_{h}");
    Tools::GlobalRegexp(grT,"phiR","phi_{R}");
    Tools::GlobalRegexp(grT,"phiH","phi_{h}");
    Tools::GlobalRegexp(grT,"\\*","");
    TString xT = grT;
    xT(TRegexp("^.*vs\\. ")) = "";
    if(xT.Contains("M")) xT += " [GeV]";
    grT += ";"+xT;
    gr->SetTitle(grT);
    gr->GetXaxis()->SetTitleSize(0.07);
    gr->GetXaxis()->SetTitleOffset(1.0);

    gr->SetMarkerStyle(kFullCircle);
    gr->SetMarkerColor(kBlack);
    gr->SetMarkerSize(2);
    gr->SetLineWidth(2);
    gr->GetYaxis()->SetRangeUser(asymMin,asymMax);
    if(xT.Contains("z")) gr->GetXaxis()->SetNdivisions(5);
    gr->Draw("APE");
    zeroLine = new TLine(
        gr->GetXaxis()->GetXmin(),0,
        gr->GetXaxis()->GetXmax(),0
        );
    zeroLine->SetLineColor(kBlack);
    zeroLine->SetLineWidth(2);
    zeroLine->SetLineStyle(kDashed);
    zeroLine->Draw();
  };
};

