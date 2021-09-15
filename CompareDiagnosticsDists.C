R__LOAD_LIBRARY(DiSpin)
// CompareDiagnosticsDists ------------------------------
// draw kinematic distributions from diagnostics plots.root
// - useful for presentations

TFile * infile[2];
TString infileT[2];
TCanvas * canv;
Float_t textSize=0.04;
int f;
Double_t electronCnt[2];
TString dataName[2];
TString latexFile;
TH1D * dist[2];
TString outDir;

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
    if(distTitle!="") {
      dist[f]->SetTitle(distTitle+" distribution");
      dist[f]->GetXaxis()->SetTitle(distTitle);
    };
    dist[f]->Scale(1/electronCnt[f]); // normalize by electron yield
    //dist[f]->Sumw2(); // (already done, redundant)
    dist[f]->SetMarkerStyle(kFullCircle/*f==0?kFullTriangleUp:kFullTriangleDown*/);
    dist[f]->SetMarkerSize(0.5);
    dist[f]->SetMarkerColor(f==0?kRed-7:kBlue+3);
    dist[f]->SetLineColor(f==0?kRed-7:kBlue+3);
    dist[f]->SetLineWidth(2);
    dist[f]->GetXaxis()->SetTitleSize(textSize);
    dist[f]->GetXaxis()->SetLabelSize(textSize);
    dist[f]->GetYaxis()->SetTitleSize(textSize);
    dist[f]->GetYaxis()->SetLabelSize(textSize);

    if(TString(dist[f]->GetName()).Contains("ECOUT")) dist[f]->GetXaxis()->SetRangeUser(0.1,0.4);
    if(TString(dist[f]->GetName()).Contains("SampFrac")) dist[f]->GetXaxis()->SetRangeUser(0.1,0.4);
  };

  // ratio
  rat = (TH1D*) dist[0]->Clone();
  rat->Divide(dist[1]); // dist[0] / dist[1], assumes UNCORRELATED DATA for error bars
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
  for(int pad=1; pad<=2; pad++) {
    canv->GetPad(pad)->SetBottomMargin(0.15);
    canv->GetPad(pad)->SetLeftMargin(0.15);
    canv->GetPad(pad)->SetGrid(1,1);
  };
  canv->cd(1); dist[0]->Draw("P"); dist[1]->Draw("PSAME");
  canv->cd(2); rat->Draw("E");
  TString imgFile = outDir+"/"+TString(distName)+".png";
  canv->Print(imgFile);

  // latex code
  Tools::LatexImage(
      latexFile,
      "img/"+imgFile,
      "Left panel: comparison of " + varTex + " distributions from data sets ``" +
        dataName[0]+"'' (light red points) and ``" +
        dataName[1]+"'' (dark blue points). " +
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
  TString imgFile = outDir+"/"+TString(distName)+".png";
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
  TString infile1N="plots.rga_spring19.root",
  TString outDir_="diagcomp",
  TString infileTitle0="",
  TString infileTitle1=""
) {
  infile[0] = new TFile(infile0N,"READ");
  infile[1] = new TFile(infile1N,"READ");
  infileT[0] = infileTitle0;
  infileT[1] = infileTitle1;
  gStyle->SetOptStat(0);
  gStyle->SetPalette(kGreenPink);

  outDir=outDir_;
  gROOT->ProcessLine(".! mkdir -p "+outDir);
  latexFile = outDir+"/img.tex";
  gSystem->RedirectOutput(latexFile,"w");
  printf("%% ############ begin generated tex ############\n");
  gSystem->RedirectOutput(0);

  // get data set name
  for(f=0;f<2;f++) {
    if(infileT[f]=="") {
      dataName[f] = infile[f]->GetName();
      dataName[f](TRegexp("^plots\\."))="";
      dataName[f](TRegexp("\\.root$"))="";
      printf("dataName %d = %s\n",f,dataName[f].Data());
    } else {
      dataName[f] = infileT[f];
    };
  };
  

  // get electron yields, for normalizations
  for(f=0;f<2;f++) {
    electronCnt[f] = ((TH1D*)infile[f]->Get("dihadronCntDist"))->GetEntries();
  };


  // local function for running `CompareDist` over two hadrons of the dihadron
  auto CompareDistHadron = [&](
      TString distN,
      TString varLatex,
      TString varRoot,
      TString varLatexExtra="",
      TString varRootExtra=""
      ) {
    TString had_name[2] = {"piPlus","piMinus"};
    TString had_latex[2] = {"\\pi^+","\\pi^-"};
    TString had_root[2] = {"#pi^{+}","#pi^{-}"};
    for(int h=0; h<2; h++) {
      CompareDist(
          had_name[h] + distN,
          TString("$") + varLatex + TString("\\left(") + had_latex[h] + TString("\\right)") + varLatexExtra + TString("$"),
          varRoot + TString("(") + had_root[h] + TString(")") + varRootExtra
          );
    };
  };



  // make comparison plots ////////////////////////////

  // DIS
  CompareDist("Q2Dist","$Q^2$","Q^{2}");
  CompareDist("XDist","$x$","x");
  CompareDist("WDist","$W$","W");
  CompareDist("YDist","$y$","y");

  // dihadron
  CompareDist("MhDist","$M_h$","M_{h}");
  CompareDist("ZpairDist","$z$","z_{pair}");
  CompareDist("PhPerpDist","$P_h^\\perp$","p_{T}");
  CompareDist("MmissDist","$M_X$","M_{X}");
  CompareDist("PhiHDist","$\\phi_h$","#phi_{h}");
  CompareDist("PhiRDist","$\\phi_R$","#phi_{R}");
  CompareDist("thetaDist","$\\theta$","#theta");
  CompareDistHadron("hadPhiHDist","\\phi_h","#phi_{h}");

  // fragmentation region
  CompareDistHadron("hadXFDist","x_F","x_{F}");

  // vertex
  CompareDist("eleVzDist","$v_z\\left(e^-\\right)$","v_{z}(e^{-})");
  CompareDistHadron("hadEleVzDiffDist","v_z","v_{z}","-v_z\\left(e^-\\right)","-v_{z}(e^{-})");

  // PID
  CompareDist("eleThetaDist","$\\theta_{lab}\\left(e^-\\right)$","#theta_{lab}(e^{-})");
  CompareDist("elePDist","$p\\left(e^-\\right)$","p(e^{-})");
  CompareDist("elePCALenDist","$E_{PCAL}\\left(e^-\\right)$","E_{PCAL}(e^{-})");
  CompareDist("eleECINenDist","$E_{ECIN}\\left(e^-\\right)$","E_{ECIN}(e^{-})");
  CompareDist("eleECOUTenDist","$E_{ECOUT}\\left(e^-\\right)$","E_{ECOUT}(e^{-})");
  CompareDist("eleSampFracDist","electron sampling fraction","S.F.(e^{-})");
  CompareDistHadron("hadThetaDist","\\theta_{lab}","#theta_{lab}");
  CompareDistHadron("hadPDist","p","p");
  CompareDistHadron("hadChi2pidDist","\\chipid","#chi^{2}_{pid}");


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
