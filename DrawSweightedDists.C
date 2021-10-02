R__LOAD_LIBRARY(DiSpin)
// draw sWeighted distributions (for making presentation plots)
// (cf. draw code in sPlotBru.C, which makes more detailed plots)

TTree *tr;
TString inDir;
enum {kU,kS,kB,N}; // unweighted, signal-sweighted, background-sweighted

void Draw(TString varN, TString varT, TString units, Int_t nbins, Double_t lb, Double_t ub) {

  TH1D *hist[N];
  TString histN[N];
  TString wN[N]; wN[kU]=""; wN[kS]="_s"; wN[kB]="_b";
  varT += " distribution;" + varT + (units==""?"":" ["+units+"]");
  for(int i=0; i<N; i++) {
    histN[i] = varN+"_hist"+wN[i];
    hist[i] = new TH1D( histN[i], varT, nbins, lb, ub );
  };

  tr->Project( histN[kU], varN, "TMath::Abs(Signal*BG)>0");
  tr->Project( histN[kS], varN, "Signal");
  tr->Project( histN[kB], varN, "BG");

  hist[kU]->SetLineColor(kBlack); hist[kU]->SetLineWidth(3);
  hist[kS]->SetMarkerColor(kRed+2);    hist[kS]->SetMarkerStyle(kFullTriangleUp);
  hist[kB]->SetMarkerColor(kAzure+10); hist[kB]->SetMarkerStyle(kFullTriangleDown);

  TCanvas *canv = new TCanvas( varN+"_canv", varN+"_canv", 800, 600 );
  canv->Divide(1,1);
  canv->cd(1);
  //canv->GetPad(1)->SetBottomMargin(0.15);
  //canv->GetPad(1)->SetLeftMargin(0.15);
  for(int i=0; i<N; i++) hist[i]->Draw(i>0?"SAME":"");
  Tools::UnzoomVertical(canv->GetPad(1),"",1);
  canv->SaveAs(inDir+"/kindist_"+varN+".png");
};



void DrawSweightedDists(TString inDir_="splot.singlebin") {
  gStyle->SetOptStat(0);
  inDir=inDir_;
  TFile *infile = new TFile(inDir+"/DataWeightedTree.root","READ");
  tr = (TTree*) infile->Get("tree");

  Draw("X","x","",100,0,1);
  Draw("Q2","Q^{2}","GeV^{2}",100,0,10);
  Draw("Mh","M_{h}","GeV",100,0,2);
  Draw("Z","z","",100,0,1);
  Draw("PhPerp","p_{T}","GeV",100,0,2);
  Draw("PhiH","#phi_{h}","",100,-TMath::Pi(),TMath::Pi());
  Draw("PhiR","#phi_{R}","",100,-TMath::Pi(),TMath::Pi());
  Draw("Theta","#theta","",100,0,TMath::Pi());
};
  
  
