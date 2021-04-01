void DrawCatTreeDists(TString infileN="catTreeData.XFgt0.root") {
  TFile * infile = new TFile(infileN,"READ");
  TTree * tree = (TTree*) infile->Get("tree");

  TH2D * ZvsMh = new TH2D(
    "ZvsMh",
    "z_{pair} vs. M_{h};M_{h} [GeV];z_{pair}",
    100,0,2.5,100,0.2,1);
  TH2D * PhPerpvsMh = new TH2D(
    "PhPerpvsMh",
    "p_{T} vs. M_{h};M_{h} [GeV];p_{T} [GeV]",
    100,0,2.5,100,0,2);

  tree->Project("ZvsMh","Z:Mh");
  tree->Project("PhPerpvsMh","PhPerp:Mh");

  Float_t textSize=0.04;
  ZvsMh->GetXaxis()->SetTitleSize(textSize);
  ZvsMh->GetXaxis()->SetLabelSize(textSize);
  ZvsMh->GetYaxis()->SetTitleSize(textSize);
  ZvsMh->GetYaxis()->SetLabelSize(textSize);
  PhPerpvsMh->GetXaxis()->SetTitleSize(textSize);
  PhPerpvsMh->GetXaxis()->SetLabelSize(textSize);
  PhPerpvsMh->GetYaxis()->SetTitleSize(textSize);
  PhPerpvsMh->GetYaxis()->SetLabelSize(textSize);

  gStyle->SetOptStat(0);
  TCanvas * canv;
  TLine * line;
  canv = new TCanvas("ZvsMh_canv","ZvsMh_canv",800,700);
  canv->SetGrid(1,1);
  ZvsMh->Draw("COLZ");
  line = new TLine(0.6,ZvsMh->GetYaxis()->GetXmin(),0.6,ZvsMh->GetYaxis()->GetXmax());
  line->SetLineWidth(3); line->Draw();
  line = new TLine(0.95,ZvsMh->GetYaxis()->GetXmin(),0.95,ZvsMh->GetYaxis()->GetXmax());
  line->SetLineWidth(3); line->Draw();
  canv = new TCanvas("PhPerpvsMh_canv","PhPerpvsMh_canv",800,700);
  canv->SetGrid(1,1);
  PhPerpvsMh->Draw("COLZ");
  line = new TLine(0.6,PhPerpvsMh->GetYaxis()->GetXmin(),0.6,PhPerpvsMh->GetYaxis()->GetXmax());
  line->SetLineWidth(3); line->Draw();
  line = new TLine(0.95,PhPerpvsMh->GetYaxis()->GetXmin(),0.95,PhPerpvsMh->GetYaxis()->GetXmax());
  line->SetLineWidth(3); line->Draw();
};
