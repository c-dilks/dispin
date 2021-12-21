// draw depol plots
void xcheckDepol(TString infileN0="tables0.root", TString infileN1="tables1.root") {

  // TString grN = "aveW_aveA_vs_aveX";
  TString grN = "aveC_aveA_vs_aveX";

  TFile *infile[2];
  infile[0] = new TFile(infileN0,"READ");
  infile[1] = new TFile(infileN1,"READ");

  TMultiGraph *mgr = new TMultiGraph();
  TGraphErrors *gr[2];
  for(int f=0;f<2;f++) {
    gr[f] = (TGraphErrors*) infile[f]->Get(grN);
    gr[f]->GetXaxis()->SetRangeUser(0.1,0.4);
    // gr[f]->GetYaxis()->SetRangeUser(0.3,0.7);
    gr[f]->SetMarkerColor(f==0?kRed-7:kAzure+1);
    gr[f]->SetLineColor(f==0?kRed-7:kAzure+1);
    gr[f]->SetMarkerStyle(f==0?kFullTriangleUp:kFullTriangleDown);
    gr[f]->SetMarkerSize(3);
    gr[f]->SetLineWidth(4);
    mgr->Add(gr[f]);
  }

  TCanvas *canv = new TCanvas("canv","canv",900,600);
  canv->SetGrid(1,1);
  mgr->Draw("APE");
  mgr->GetXaxis()->SetRangeUser(0.1,0.4);
  // mgr->GetYaxis()->SetRangeUser(0.3,0.7);
  canv->SaveAs("xcheck.png");
}
