// reads plots.root and draws the kinematic factor plots, with profiles on them

void drawKinematicFactorPlots() {
  gStyle->SetOptStat(0);

  TFile * infile = new TFile("plots.root","READ");

  enum KF_enum {kfA, kfB, kfC, kfV, kfW, kfWA, kfVA, kfCA, kfBA, Nkf};
  TString kfName[Nkf];
  TString kfTitle[Nkf];
  kfName[kfA] = "kfA"; kfTitle[kfA] = "A(y)";
  kfName[kfB] = "kfB"; kfTitle[kfB] = "B(y)";
  kfName[kfC] = "kfC"; kfTitle[kfC] = "C(y)";
  kfName[kfV] = "kfV"; kfTitle[kfV] = "V(y)";
  kfName[kfW] = "kfW"; kfTitle[kfW] = "W(y)";
  kfName[kfWA] = "kfWA"; kfTitle[kfWA] = "W(y)/A(y)";
  kfName[kfVA] = "kfVA"; kfTitle[kfVA] = "V(y)/A(y)";
  kfName[kfCA] = "kfCA"; kfTitle[kfCA] = "C(y)/A(y)";
  kfName[kfBA] = "kfBA"; kfTitle[kfBA] = "B(y)/A(y)";

  for(int k=0; k<Nkf; k++) kfName[k] = "/kinematicFactors/"+kfName[k];

  TH2D * kfVsMh[Nkf];
  TH2D * kfVsQ2[Nkf];
  TH2D * kfVsMmiss[Nkf];
  TH2D * kfVsPhPerp[Nkf];
  TH2D * kfVsX[Nkf];
  TH2D * kfVsZpair[Nkf];
  TH2D * kfVsPhiR[Nkf];
  TH2D * kfVsPhiH[Nkf];
  TH2D * kfVsPhiHR[Nkf];

  TProfile * kfVsMh_pf[Nkf];
  TProfile * kfVsQ2_pf[Nkf];
  TProfile * kfVsMmiss_pf[Nkf];
  TProfile * kfVsPhPerp_pf[Nkf];
  TProfile * kfVsX_pf[Nkf];
  TProfile * kfVsZpair_pf[Nkf];
  TProfile * kfVsPhiR_pf[Nkf];
  TProfile * kfVsPhiH_pf[Nkf];
  TProfile * kfVsPhiHR_pf[Nkf];

  for(int k=0; k<Nkf; k++) {
    kfVsMh[k] = (TH2D*) infile->Get(TString(kfName[k]+"vsMh"));
    kfVsQ2[k] = (TH2D*) infile->Get(TString(kfName[k]+"vsQ2"));
    kfVsMmiss[k] = (TH2D*) infile->Get(TString(kfName[k]+"vsMmiss"));
    kfVsPhPerp[k] = (TH2D*) infile->Get(TString(kfName[k]+"vsPhPerp"));
    kfVsX[k] = (TH2D*) infile->Get(TString(kfName[k]+"vsX"));
    kfVsZpair[k] = (TH2D*) infile->Get(TString(kfName[k]+"vsZpair"));
    kfVsPhiH[k] = (TH2D*) infile->Get(TString(kfName[k]+"vsPhiH"));
    kfVsPhiR[k] = (TH2D*) infile->Get(TString(kfName[k]+"vsPhiR"));
    kfVsPhiHR[k] = (TH2D*) infile->Get(TString(kfName[k]+"vsPhiHR"));
  };

  for(int k=0; k<Nkf; k++) {
    kfVsMh_pf[k] = kfVsMh[k]->ProfileX();
    kfVsQ2_pf[k] = kfVsQ2[k]->ProfileX();
    kfVsMmiss_pf[k] = kfVsMmiss[k]->ProfileX();
    kfVsPhPerp_pf[k] = kfVsPhPerp[k]->ProfileX();
    kfVsX_pf[k] = kfVsX[k]->ProfileX();
    kfVsZpair_pf[k] = kfVsZpair[k]->ProfileX();
    kfVsPhiR_pf[k] = kfVsPhiR[k]->ProfileX();
    kfVsPhiH_pf[k] = kfVsPhiH[k]->ProfileX();
    kfVsPhiHR_pf[k] = kfVsPhiHR[k]->ProfileX();
  };

  for(int k=0; k<Nkf; k++) {
    kfVsMh_pf[k]->SetLineColor(kBlack);
    kfVsQ2_pf[k]->SetLineColor(kBlack);
    kfVsMmiss_pf[k]->SetLineColor(kBlack);
    kfVsPhPerp_pf[k]->SetLineColor(kBlack);
    kfVsX_pf[k]->SetLineColor(kBlack);
    kfVsZpair_pf[k]->SetLineColor(kBlack);
    kfVsPhiR_pf[k]->SetLineColor(kBlack);
    kfVsPhiH_pf[k]->SetLineColor(kBlack);
    kfVsPhiHR_pf[k]->SetLineColor(kBlack);
  };

  for(int k=0; k<Nkf; k++) {
    kfVsMh_pf[k]->SetLineWidth(3);
    kfVsQ2_pf[k]->SetLineWidth(3);
    kfVsMmiss_pf[k]->SetLineWidth(3);
    kfVsPhPerp_pf[k]->SetLineWidth(3);
    kfVsX_pf[k]->SetLineWidth(3);
    kfVsZpair_pf[k]->SetLineWidth(3);
    kfVsPhiR_pf[k]->SetLineWidth(3);
    kfVsPhiH_pf[k]->SetLineWidth(3);
    kfVsPhiHR_pf[k]->SetLineWidth(3);
  };


  ///*
  TCanvas * canv[Nkf];
  for(int k=0; k<Nkf; k++) {
    canv[k] = new TCanvas(
      TString("canv_"+kfName[k]),TString("canv_"+kfName[k]),1000,1000);
    canv[k]->Divide(4,2);

    for(int p=1; p<=8; p++) canv[k]->GetPad(p)->SetLogz();

    canv[k]->cd(1);
    kfVsX[k]->Draw("colz");
    kfVsX_pf[k]->Draw("same");
    canv[k]->cd(2);
    kfVsQ2[k]->Draw("colz");
    kfVsQ2_pf[k]->Draw("same");
    canv[k]->cd(3);
    kfVsZpair[k]->Draw("colz");
    kfVsZpair_pf[k]->Draw("same");
    canv[k]->cd(4);
    kfVsPhiH[k]->Draw("colz");
    kfVsPhiH_pf[k]->Draw("same");
    canv[k]->cd(5);
    kfVsPhPerp[k]->Draw("colz");
    kfVsPhPerp_pf[k]->Draw("same");
    canv[k]->cd(6);
    kfVsMmiss[k]->Draw("colz");
    kfVsMmiss_pf[k]->Draw("same");
    canv[k]->cd(7);
    kfVsMh[k]->Draw("colz");
    kfVsMh_pf[k]->Draw("same");
    canv[k]->cd(8);
    kfVsPhiR[k]->Draw("colz");
    kfVsPhiR_pf[k]->Draw("same");
    //kfVsPhiHR[k]->Draw("colz");
    //kfVsPhiHR_pf[k]->Draw("same");
  };
  //*/

  /*
  TCanvas * canv[Nkf];
  for(int k=0; k<Nkf; k++) {
    canv[k] = new TCanvas(
      TString("canv_"+kfName[k]),TString("canv_"+kfName[k]),1000,1000);
    canv[k]->Divide(3,1);

    for(int p=1; p<=3; p++) canv[k]->GetPad(p)->SetLogz();

    canv[k]->cd(1);
    kfVsX[k]->Draw("colz");
    kfVsX_pf[k]->Draw("same");
    canv[k]->cd(2);
    kfVsMh[k]->Draw("colz");
    kfVsMh_pf[k]->Draw("same");
    canv[k]->cd(3);
    kfVsZpair[k]->Draw("colz");
    kfVsZpair_pf[k]->Draw("same");
  };
  */
};
