R__LOAD_LIBRARY(DiSpin)
// draw sWeighted distributions (for making presentation plots)
// (cf. draw code in sPlotBru.C, which makes more detailed plots)
//
// output will be in `splotDir/kindist*.png`
// - `kindist1*.png`: unweighted, signal-sWeighted, BG-sweighted
// - `kindist2*.png`: signal-sWeighted, MC-trimmed

TTree *splotTr;
TTree *mcTr;
TString splotDir;
Double_t weightSum;
enum {kU,kS,kB,kM,N}; // unweighted, signal-sweighted, background-sweighted, monte carlo (unweighted)

void Draw(TString varN, TString varT, TString units, Int_t nbins, Double_t lb, Double_t ub) {

  TH1D *hist[N];
  TString histN[N];
  TString wN[N]; wN[kU]=""; wN[kS]="_s"; wN[kB]="_b"; wN[kM]="_m";
  varT += " distribution;" + varT + (units==""?"":" ["+units+"]");
  for(int i=0; i<N; i++) {
    histN[i] = varN+"_hist"+wN[i];
    hist[i] = new TH1D( histN[i], varT, nbins, lb, ub );
  };

  splotTr->Project( histN[kU], varN, "TMath::Abs(Signal*BG)>0");
  splotTr->Project( histN[kS], varN, "Signal");
  splotTr->Project( histN[kB], varN, "BG");
  mcTr->Project(    histN[kM], varN, "");

  hist[kU]->SetLineColor(kBlack); hist[kU]->SetLineWidth(3);
  hist[kS]->SetMarkerColor(kRed+2);    hist[kS]->SetMarkerStyle(kFullTriangleUp);
  hist[kB]->SetMarkerColor(kAzure+10); hist[kB]->SetMarkerStyle(kFullTriangleDown);
  hist[kM]->SetMarkerColor(kAzure+10); hist[kM]->SetMarkerStyle(kFullTriangleDown);

  TCanvas *canv1 = new TCanvas( varN+"_canv1", varN+"_canv1", 800, 600 );
  canv1->Divide(1,1);
  canv1->cd(1);
  //canv1->GetPad(1)->SetBottomMargin(0.15);
  //canv1->GetPad(1)->SetLeftMargin(0.15);
  hist[kU]->Draw();
  hist[kS]->Draw("SAME");
  hist[kB]->Draw("SAME");
  Tools::UnzoomVertical(canv1->GetPad(1),"",1);
  canv1->SaveAs(splotDir+"/kindist1_"+varN+".png");

  TCanvas *canv2 = new TCanvas( varN+"_canv2", varN+"_canv2", 800, 600 );
  canv2->Divide(1,1);
  canv2->cd(1);
  //canv2->GetPad(1)->SetBottomMargin(0.15);
  //canv2->GetPad(1)->SetLeftMargin(0.15);
  TH1D *histSigNormalized = (TH1D*)hist[kS]->Clone(histN[kS]+"_clone");
  TH1D *histMCNormalized = (TH1D*)hist[kM]->Clone(histN[kM]+"_clone");
  Double_t splotNum,mcNum;
  splotNum = weightSum; // [ not (Double_t)splotTr->GetEntries(); ]
  mcNum = (Double_t)mcTr->GetEntries();
  histSigNormalized->Scale(1/splotNum);
  histMCNormalized->Scale(1/mcNum);
  histSigNormalized->Draw();
  histMCNormalized->Draw("SAME");
  Tools::UnzoomVertical(canv2->GetPad(1),"",1);
  canv2->SaveAs(splotDir+"/kindist2_"+varN+".png");
};

/////////////////////////////////////
/////////////////////////////////////
/////////////////////////////////////

void DrawSweightedDists(
  TString splotDir_="splot.singlebin",
  TString mcFileN="catTreeMC.mc.PRL.0x3b.idx.trimmed.root"
) {
  // open files
  gStyle->SetOptStat(0);
  splotDir=splotDir_;
  TFile *splotFile = new TFile(splotDir+"/DataWeightedTree.root","READ");
  splotTr = (TTree*) splotFile->Get("tree");
  TFile *mcFile = new TFile(mcFileN,"READ");
  mcTr = (TTree*) mcFile->Get("tree");

  // get sum of sWeights
  Double_t sw;
  weightSum = 0;
  splotTr->SetBranchAddress("Signal",&sw);
  for(Long64_t e=0; e<splotTr->GetEntries(); e++) {
    splotTr->GetEntry(e);
    weightSum += sw;
  };

  // draw histograms
  Draw("X","x","",100,0,1);
  Draw("Q2","Q^{2}","GeV^{2}",100,0,10);
  Draw("Mh","M_{h}","GeV",100,0,2);
  Draw("Z","z","",100,0,1);
  Draw("PhPerp","p_{T}","GeV",100,0,2);
  Draw("PhiH","#phi_{h}","",100,-TMath::Pi(),TMath::Pi());
  Draw("PhiR","#phi_{R}","",100,-TMath::Pi(),TMath::Pi());
  Draw("Theta","#theta","",100,0,TMath::Pi());

  // cleanup
  splotFile->Close();
  mcFile->Close();
};
