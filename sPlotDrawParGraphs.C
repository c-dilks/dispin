// draw a canvas of sPlot ParGraphs
void sPlotDrawParGraphs(TString infileN="splot.z/ParGraphsZ.root") {
  TFile *infile = new TFile(infileN,"READ");
  TCanvas *canv = new TCanvas("canv","canv",1800,1000);
  TGraphErrors *gr[6];
  gr[0] = (TGraphErrors*) infile->Get("Yld_BG");
  gr[1] = (TGraphErrors*) infile->Get("Yld_Signal");
  gr[2] = (TGraphErrors*) infile->Get("b0");
  gr[3] = (TGraphErrors*) infile->Get("b1");
  gr[4] = (TGraphErrors*) infile->Get("pi0mu");
  gr[5] = (TGraphErrors*) infile->Get("pi0sigma");
  canv->Divide(3,2);
  for(int i=0; i<6; i++) {
    canv->cd(i+1);
    canv->GetPad(i+1)->SetGrid(1,1);
    canv->GetPad(i+1)->SetLeftMargin(0.15);
    canv->GetPad(i+1)->SetBottomMargin(0.15);
    gr[i]->SetMarkerStyle(kFullCircle);
    gr[i]->Draw("APE");
  };
  canv->SaveAs("canv.png");
};
