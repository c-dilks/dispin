// CompareCatTreeDists ------------------------------
// draw kinematic distributions from catTree*.root files
// - useful for presentations

TFile * infile[2];
TTree * tr[2];
TCanvas * canv;
Float_t textSize=0.04;
int f;
Double_t normalizer[2];
const Int_t NBINS = 100;

void CompareDist(TString varname);

void CompareCatTreeDists(
  TString infile0N="catTreeData.rga_inbending_fa18.0x35.root", /*green*/
  TString infile1N="catTreeData.rga_inbending_fa18.0x3c.root"  /*purple*/
) {
  infile[0] = new TFile(infile0N,"READ");
  infile[1] = new TFile(infile1N,"READ");
  for(f=0;f<2;f++) tr[f] = (TTree*)infile[f]->Get("tree");


  gStyle->SetOptStat(0);
  gStyle->SetPalette(kGreenPink);
  gROOT->ProcessLine(".! mkdir -p cattreecomp");


  // set normalization
  for(f=0;f<2;f++) {
    //normalizer[f] = ((TH1D*)infile[f]->Get("dihadronCntDist"))->GetEntries();// electron yield
    normalizer[f] = (Double_t)tr[f]->GetEntries();
    //normalizer[f] = 1;
  };

  CompareDist("Mh");
  CompareDist("X");
  CompareDist("Z");
  CompareDist("Q2");
  CompareDist("PhPerp");
  CompareDist("PhiH");
  CompareDist("PhiR");
  CompareDist("Theta");
  CompareDist("XF");
  CompareDist("DYsgn");
};


void CompareDist(TString varname) {
  TH1D * dist[2];
  TH1D * rat;
  TString distN[2];
  Double_t varmin[2];
  Double_t varmax[2];
  printf("compare %s\n",varname.Data());

  // {min,max}ima
  for(f=0;f<2;f++) {
    varmin[f] = tr[f]->GetMinimum(varname);
    varmax[f] = tr[f]->GetMaximum(varname);
  };
  Double_t distmin = TMath::Min(
      0.0,
      TMath::Min(varmin[0],varmin[1])
      );
  Double_t distmax = TMath::Max(varmax[0],varmax[1]);

  // distributions
  for(f=0;f<2;f++) {
    distN[f] = Form("%sDist%d",varname.Data(),f);
    dist[f] = new TH1D(distN[f],varname,NBINS,distmin,distmax);
    tr[f]->Project(distN[f],varname);
    dist[f]->Scale(1/normalizer[f]); // normalization
    //dist[f]->Sumw2(); // (already done, redundant)
    dist[f]->SetMarkerStyle(f==0?kFullTriangleUp:kFullTriangleDown);
    dist[f]->SetMarkerSize(1.0);
    dist[f]->SetMarkerColor(f==0?kGreen+1:kViolet+2);
    dist[f]->SetLineColor(f==0?kGreen+1:kViolet+2);
    dist[f]->SetLineWidth(2);
    dist[f]->GetXaxis()->SetTitleSize(textSize);
    dist[f]->GetXaxis()->SetLabelSize(textSize);
    dist[f]->GetYaxis()->SetTitleSize(textSize);
    dist[f]->GetYaxis()->SetLabelSize(textSize);
  };

  // ratio
  rat = new TH1D(varname+"Rat",varname,NBINS,distmin,distmax);
  rat->Divide(dist[0],dist[1]); // dist[0] / dist[1]
  rat->SetLineColor(kBlack);
  rat->SetMarkerColor(kBlack);
  rat->SetMarkerStyle(kFullCircle);
  rat->SetMarkerSize(0.5);
  rat->GetYaxis()->SetRangeUser(0.0,5);
  TString ratT = rat->GetTitle();
  ratT(TRegexp("distribution")) = "Data/MC";
  rat->SetTitle(ratT);

  // draw canvas
  canv = new TCanvas(
    TString(varname+"_canv"),TString(varname+"_canv"),1600,800);
  canv->Divide(2,1);
  canv->cd(1); canv->GetPad(1)->SetGrid(1,1);
  if(dist[0]->GetMaximum() > dist[1]->GetMaximum()) {
    dist[0]->Draw("P"); dist[1]->Draw("PSAME");
  } else {
    dist[1]->Draw("P"); dist[0]->Draw("PSAME");
  };
  canv->cd(2); canv->GetPad(2)->SetGrid(1,1);
  rat->Draw("E");
  canv->Print("cattreecomp/"+TString(varname)+".png");

  // print
  cout << "integrals:"
       << " " << dist[0]->Integral()
       << " " << dist[1]->Integral()
       << endl;

};
