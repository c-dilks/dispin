R__LOAD_LIBRARY(DiSpin)
// CompareDiagnosticsDists ------------------------------
// draw kinematic distributions from diagnostics plots.root
// - useful for presentations

TFile * infile[2];
TCanvas * canv;
Float_t textSize=0.04;
int f;
Double_t electronCnt[2];
TString dataName[2];
TString latexFile;
TH1D * dist[2];

/* `distName` is the name of the distribution (no need to specify path);
 *  if empty, it will instead use whatever's currently at the `dist[]` pointers
 * `varTex` is the variable name in latex
 * `distTitle` if !="" will overwrite the distribution title
 */
void CompareDist(TString distName, TString varTex, TString distTitle="") {
  TH1D * rat;

  // distributions
  for(f=0;f<2;f++) {
    if(distName!="") dist[f] = (TH1D*) infile[f]->FindObjectAny(distName);
    else distName=dist[f]->GetName();
    printf("compare %s\n",distName.Data());
    if(dist[f]==NULL) { printf("...not found...\n"); return; };
    distName(TRegexp("^.*/"))="";
    if(distTitle!="") dist[f]->SetTitle(distTitle);
    dist[f]->Scale(1/electronCnt[f]); // normalize by electron yield
    //dist[f]->Sumw2(); // (already done, redundant)
    dist[f]->SetMarkerStyle(f==0?kFullTriangleUp:kFullTriangleDown);
    dist[f]->SetMarkerSize(2.0);
    dist[f]->SetMarkerColor(f==0?kGreen+1:kViolet+2);
    dist[f]->SetLineColor(f==0?kGreen+1:kViolet+2);
    dist[f]->SetLineWidth(2);
    dist[f]->GetXaxis()->SetTitleSize(textSize);
    dist[f]->GetXaxis()->SetLabelSize(textSize);
    dist[f]->GetYaxis()->SetTitleSize(textSize);
    dist[f]->GetYaxis()->SetLabelSize(textSize);
  };

  // ratio
  rat = (TH1D*) dist[0]->Clone();
  rat->Divide(dist[1]); // dist[0] / dist[1]
  rat->SetLineColor(kBlack);
  rat->SetMarkerColor(kBlack);
  rat->SetMarkerStyle(kFullCircle);
  rat->SetMarkerSize(0.5);
  rat->GetYaxis()->SetRangeUser(0.5,2);
  TString ratT = rat->GetTitle();
  ratT(TRegexp("distribution")) = dataName[0]+"/"+dataName[1];
  rat->SetTitle(ratT);

  // draw canvas
  canv = new TCanvas(
    TString(distName+"_canv"),TString(distName+"_canv"),1600,800);
  canv->Divide(2,1);
  canv->cd(1); canv->GetPad(1)->SetGrid(1,1);
  dist[0]->Draw("P"); dist[1]->Draw("PSAME");
  canv->cd(2); canv->GetPad(2)->SetGrid(1,1);
  rat->Draw("E");
  TString imgFile = "diagcomp/"+TString(distName)+".png";
  canv->Print(imgFile);

  // latex code
  Tools::LatexImage(
      latexFile,
      "img/"+imgFile,
      "Left panel: comparison of " + varTex + " distributions for data sets ``" +
        dataName[0]+"'' (green upward triangles) and ``" +
        dataName[1]+"'' (purple downward triangles). " +
        "Right panel: ratio of data sets."
        ,
      "diagnosticComp_"+distName,
      "h",
      0.7
      );
};


// compare 1D projections of 2D histograms
// TODO: doesn't quite work yet... seems like somehow dist[0] == dist[1] ...
void CompareDist2Dproj(TString distName, TString xvarTex, TString yvarTex, TString xdistTitle="", TString ydistTitle="") {
  if(distName!="") printf("compare %s\n",distName.Data());
  // x-projection comparison
  for(f=0;f<2;f++) dist[f] = ((TH2D*)infile[f]->FindObjectAny(distName))->ProjectionX();
  CompareDist("",xvarTex,xdistTitle);
  // y-projection comparison
  for(f=0;f<2;f++) dist[f] = ((TH2D*)infile[f]->FindObjectAny(distName))->ProjectionY();
  CompareDist("",yvarTex,ydistTitle);
};



void CompareDist2D(TString distName) {
  TH2D * dist[2];
  TH2D * rat;
  printf("compare %s\n",distName.Data());

  // distributions
  for(f=0;f<2;f++) {
    dist[f] = (TH2D*) infile[f]->FindObjectAny(distName);
    if(dist[f]==NULL) { printf("...not found...\n"); return; };
    distName(TRegexp("^.*/"))="";
    dist[f]->Scale(1/electronCnt[f]); // normalize by electron yield
    //dist[f]->Sumw2(); // (already done, redundant)
    dist[f]->GetXaxis()->SetTitleSize(textSize);
    dist[f]->GetXaxis()->SetLabelSize(textSize);
    dist[f]->GetYaxis()->SetTitleSize(textSize);
    dist[f]->GetYaxis()->SetLabelSize(textSize);
  };

  // ratio
  rat = (TH2D*) dist[0]->Clone();
  rat->Divide(dist[1]); // dist[0] / dist[1]
  rat->GetZaxis()->SetRangeUser(0.1,2.1);

  // draw canvas
  canv = new TCanvas(
    TString(distName+"_canv"),TString(distName+"_canv"),2400,800);
  canv->Divide(3,1);
  canv->cd(1); canv->GetPad(1)->SetGrid(1,1); dist[0]->Draw("COLZ");
  canv->cd(2); canv->GetPad(2)->SetGrid(1,1); dist[1]->Draw("COLZ");
  canv->cd(3); canv->GetPad(3)->SetGrid(1,1); rat->Draw("COLZ");
  TString imgFile = "diagcomp/"+TString(distName)+".png";
  canv->Print(imgFile);
};

/*
void DrawDist2D(TString distName) {
  TH2D * dist = (TH2D*) infile->Get(distName);
  dist->GetXaxis()->SetTitleSize(textSize);
  dist->GetXaxis()->SetLabelSize(textSize);
  dist->GetYaxis()->SetTitleSize(textSize);
  dist->GetYaxis()->SetLabelSize(textSize);
  canv = new TCanvas(TString(distName+"_canv"),TString(distName+"_canv"),1200,800);
  canv->SetGrid(1,1);
  dist->Draw("colz");
  canv->Print(TString(distName)+".png");
};

void DrawCanv(TString distName) {
  canv = (TCanvas*) infile->Get(distName);
  canv->SetGrid(1,1);
  canv->Draw();
  canv->SetWindowSize(1200,1200);
  canv->Show();
  canv->Print(TString(distName)+".png");
};
*/

///////////////////////////////////////////////////
///////////////////////////////////////////////////
///////////////////////////////////////////////////
  
void CompareDiagnosticsDists(
  TString infile0N="plots.inbending.root",
  TString infile1N="plots.rga_spring19.root"
) {
  infile[0] = new TFile(infile0N,"READ");
  infile[1] = new TFile(infile1N,"READ");
  gStyle->SetOptStat(0);
  gStyle->SetPalette(kGreenPink);

  TString outdir="diagcomp";
  gROOT->ProcessLine(".! mkdir -p "+outdir);
  latexFile = outdir+"/img.tex";
  gSystem->RedirectOutput(latexFile,"w");
  printf("%% ############ begin generated tex ############\n");
  gSystem->RedirectOutput(0);

  // get data set name
  for(f=0;f<2;f++) {
    dataName[f] = infile[f]->GetName();
    dataName[f](TRegexp("^plots\\."))="";
    dataName[f](TRegexp("\\.root$"))="";
    printf("dataName %d = %s\n",f,dataName[f].Data());
  };
  

  // get electron yields, for normalizations
  for(f=0;f<2;f++) {
    electronCnt[f] = ((TH1D*)infile[f]->Get("dihadronCntDist"))->GetEntries();
  };

  CompareDist("Q2Dist","$Q^2$");
  CompareDist("XDist","$x$");
  CompareDist("WDist","$W$");

  CompareDist("MhDist","$M_h$");
  CompareDist("ZpairDist","$z$");
  CompareDist("PhPerpDist","$P_h^\\perp$");
  CompareDist("thetaDist","$\\theta$");
  CompareDist("MmissDist","$M_X$");

  CompareDist("PhiHDist","$\\phi_h$");
  CompareDist("PhiRDist","$\\phi_R$");

  CompareDist("piPlushadPhiHDist",  "$\\phi_h(\\pi^+)$",  "#phi_{h}(#pi^{+}) distribution");
  CompareDist("piMinushadPhiHDist", "$\\phi_h(\\pi^-)$",  "#phi_{h}(#pi^{-}) distribution");

  CompareDist("eleVzDist","electron $v_z$");

  /*
  CompareDist2Dproj(
      "vzDiffEleHad",
      "$v_z(\\pi^-)-v_z(e^-)$",
      "$v_z(\\pi^+)-v_z(e^-)$",
      "v_{z}(#pi^{-})-v_{z}(e^{-})",
      "v_{z}(#pi^{+})-v_{z}(e^{-})"
      );
  */
  /*
  CompareDist2D("Q2vsX");
  CompareDist2D("eleVxyDist");
  CompareDist2D("eleEvsPhi");
  CompareDist2D("vzDiffEleHad");
  CompareDist2D("PhiHvsPhiR");
  */

  gSystem->RedirectOutput(latexFile,"a");
  printf("%% ############ end generated tex ############\n");
  gSystem->RedirectOutput(0);
  printf("latex file: %s\n",latexFile.Data());

};
