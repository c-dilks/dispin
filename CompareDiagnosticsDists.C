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

  // define canvas
  canv = new TCanvas(
    TString(distName+"_canv"),TString(distName+"_canv"),1600,800);
  canv->Divide(2,1);
  for(int pad=1; pad<=2; pad++) {
    canv->GetPad(pad)->SetBottomMargin(0.15);
    canv->GetPad(pad)->SetLeftMargin(0.15);
    canv->GetPad(pad)->SetGrid(1,1);
  };

  // specific adjustments for specific histogram
  for(f=0;f<2;f++) {
    if(TString(dist[f]->GetName()).Contains("eleVzDist")) canv->GetPad(1)->SetLogy(1);
    if(TString(dist[f]->GetName()).Contains("hadEleVzDiff")) canv->GetPad(1)->SetLogy(1);
    if(TString(dist[f]->GetName()).Contains("elePCALenDist")) canv->GetPad(1)->SetLogy(1);
    if(TString(dist[f]->GetName()).Contains("eleECINenDist")) canv->GetPad(1)->SetLogy(1);
    if(TString(dist[f]->GetName()).Contains("eleECOUTenDist")) canv->GetPad(1)->SetLogy(1);
  };

  // draw
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
      1.0
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


// add a profile to 2D dist (cf. Tools::ApplyProfile)
TProfile *MakeProfile(TH2D * histo, Int_t whichAxis, Int_t id) {
  TProfile * prof;
  TString profN = Form("%s_%d_pf",histo->GetName(),id);
  switch(whichAxis) {
    case 1: prof = histo->ProfileX(profN); break;
    case 2: prof = histo->ProfileY(profN); break;
    default: 
            fprintf(stderr,"ERROR: bad whichAxis in MakeProfile\n");
            return nullptr;
  };
  prof->SetLineColor(kBlack);
  prof->SetLineWidth(3);
  return prof;
};



void CompareDist2D(TString distName, TString varTexX, TString varTexY) {
  TH2D * dist[2];
  TH2D * rat;
  printf("compare %s\n",distName.Data());

  // distributions
  TString distT[2];
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
    distT[f] = dist[f]->GetTitle();
    dist[f]->SetTitle(distT[f]+" :: "+dataName[f]);
  };

  // ratio
  rat = (TH2D*) dist[0]->Clone();
  rat->SetTitle("ratio "+dataName[0]+" / "+dataName[1]);
  rat->Divide(dist[1]); // dist[0] / dist[1]
  rat->GetZaxis()->SetRangeUser(0.1,2.1);

  // draw canvas
  canv = new TCanvas(
    TString(distName+"_canv"),TString(distName+"_canv"),2000,1600);
  canv->Divide(2,2);
  for(int pad=1; pad<=4; pad++) {
    canv->GetPad(pad)->SetBottomMargin(0.15);
    canv->GetPad(pad)->SetLeftMargin(0.15);
    canv->GetPad(pad)->SetRightMargin(0.15);
    canv->GetPad(pad)->SetGrid(1,1);
  };
  TProfile *distProf[2];
  for(int f=0;f<2;f++) {
    canv->cd(f+1);
    dist[f]->Draw("COLZ");
    distProf[f] = MakeProfile(dist[f],1,f);
    distProf[f]->Draw("SAME");
  };
  canv->cd(3); rat->Draw("COLZ");
  canv->cd(4); 
  TProfile *distProfClone[2];
  TString distProfT[2];
  for(int f=0;f<2;f++) {
    distProfClone[f] = (TProfile*)distProf[f]->Clone();
    distProfT[f] = TString("average ")+distProfClone[f]->GetTitle();
    distProfT[f](TRegexp(" ::.*$")) = "";
    distProfClone[f]->SetTitle(distProfT[f]);
  };
  distProfClone[0]->SetLineColor(kRed-7);
  distProfClone[1]->SetLineColor(kBlue+3);
  distProfClone[1]->SetLineWidth(1);
  distProfClone[0]->Draw();
  distProfClone[1]->Draw("SAME");
  TString imgFile = outDir+"/"+TString(distName)+".png";
  canv->Print(imgFile);

  // latex code
  Tools::LatexImage(
      latexFile,
      "img/"+imgFile,
      "Distribution of " + varTexY + " vs. " + varTexX + " from data set ``" +
      dataName[0]+"'' (top-left) and data set ``" +
      dataName[1]+"'' (top-right). Profiles are given by black points. The ratio " +
      "of the distributions is given in the bottom-left, and the comparison of the " +
      "profiles is in the bottom-right, where light red and dark blue respectively correspond to " +
      "the top-left and top-right profiles."
      ,
      "diagnosticComp_"+distName,
      "h",
      1.0
      );
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
  CompareDist2D("eleDiagonalSFdist","$E_{\\mbox{PCAL}}/p$","$E_{\\mbox{ECIN}}/p$");
  CompareDist2D("eleSFvsP","$p$","electron sampling fraction");
  CompareDist2D("eleECALvsPCALedep","$E_{\\mbox{PCAL}}$","$E_{\\mbox{ECAL}}$");
  CompareDist2D("piPlushadChi2pidVsP","$p$","$\\pi^+~\\chi^2_{\\mbox{pid}}$");
  CompareDist2D("piMinushadChi2pidVsP","$p$","$\\pi^-~\\chi^2_{\\mbox{pid}}$");

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
