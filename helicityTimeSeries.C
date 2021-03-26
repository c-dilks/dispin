// draw <helicity> vs. run number

R__LOAD_LIBRARY(DiSpin)

void helicityTimeSeries(TString outrootDir="outroot.rga_inbending_all") {
  TChain * c = new TChain("tree");
  c->Add(outrootDir+"/*.root");
  Int_t runnumMin = c->GetMinimum("runnum");
  Int_t runnumMax = c->GetMaximum("runnum");
  TH2D * d = new TH2D("d","mean helicity vs. runnum",
    runnumMax-runnumMin+1,runnumMin-0.5,runnumMax+0.5,
    3,-1.5,1.5);
  c->Project("d","helicity:runnum");
  d->Draw("colz");
  TProfile * p = d->ProfileX();
  p->SetLineWidth(1);
  p->SetMarkerStyle(kFullCircle);
  p->Fit("pol0","","",runnumMin-1,runnumMax+1);
};
