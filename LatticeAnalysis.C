// procedure:
// - perform MC injection and fit
// - run Lattice1D.C, with desired B CUT, and desired injDir
// - move all the resulting difference_*.root files to the subdirectory `deltaset`
// - execute this script
//   - it's possible to specify an alternative directory instead of `deltaset`

void LatticeAnalysis(TString infileDir = "deltaset") {

  /// OPTIONS /////////////////
  Float_t deltaMax = 0.01; // scale for delta plots
  /////////////////////////////


  // get list of difference*.root files
  TString infileN;
  std::vector<TString> infileList;
  TSystemDirectory * sysDir = new TSystemDirectory(infileDir,infileDir);
  TList * sysFileList = sysDir->GetListOfFiles();
  TIter nxt(sysFileList);
  TSystemFile * sysFile;
  while(( sysFile = (TSystemFile*) nxt() )) {
    infileN = infileDir + "/" + TString(sysFile->GetName());
    if( !sysFile->IsDirectory() && 
        infileN.EndsWith(".root") &&
        infileN.Contains("difference_")
      ){
      infileList.push_back(infileN);
      printf("add %s\n",infileN.Data());
    };
  };

  TFile * outfile = new TFile(TString(infileDir+"/delta.root"),"RECREATE");

  TFile * infile;
  TGraphAsymmErrors * diffGr;

  infile = new TFile(infileList.at(0),"READ");
  const Int_t N_AMP = 7;
  TH2D * deltaDist[N_AMP];
  Double_t xmin,xmax;
  TString deltaDistN, deltaDistT, diffGrN;
  for(int a=0; a<N_AMP; a++) {
    diffGrN = Form("diffGr_%d",a);
    diffGr = (TGraphAsymmErrors*) infile->Get(diffGrN);
    deltaDistN = Form("deltaDist_%d",a);
    deltaDistT = diffGr->GetTitle();
    deltaDistT(TRegexp("delta\\[.*\\]")) = "delta distribution";
    xmin = diffGr->GetXaxis()->GetXmin();
    xmax = diffGr->GetXaxis()->GetXmax();
    deltaDist[a] = new TH2D(deltaDistN,deltaDistT,
      40, xmin, xmax, 30, -deltaMax, deltaMax);
  };

  Double_t x,y;
  for(TString diffFileN : infileList) {
    infile = new TFile(diffFileN,"READ");
    for(int a=0; a<N_AMP; a++) {
      diffGrN = Form("diffGr_%d",a);
      diffGr = (TGraphAsymmErrors*) infile->Get(diffGrN);
      for(int n=0; n<diffGr->GetN(); n++) {
        diffGr->GetPoint(n,x,y);
        deltaDist[a]->Fill(x,y);
      };
    };
    infile->Close();
  };

  TProfile * deltaProf[N_AMP];
  TString deltaProfT,deltaProfTX;
  for(int a=0; a<N_AMP; a++) {
    deltaProf[a] = deltaDist[a]->ProfileX("_pfx",1,-1,"s");
    deltaProfT = deltaDist[a]->GetTitle();
    deltaProfTX = deltaProfT;
    deltaProfT(TRegexp("#delta distribution")) = "average #delta";
    deltaProfT(TRegexp("#pi.*A_")) = "A_";
    deltaProfT(TRegexp("{perp}")) = "{ }^{#perp} ";
    deltaProfT(TRegexp(" ::")) = ", with";
    if(deltaProfT.Contains("with")) deltaProfT += " GeV";
    cout << deltaProfT << endl;
    deltaDist[a]->SetTitle(deltaProfT);

    deltaProfTX(TRegexp("^.*vs. ")) = "";
    deltaProfTX(TRegexp("::.*$")) = "";
    deltaProfTX(TRegexp("{perp}")) = "{ }^{#perp}";
    if(deltaProfTX.Contains("M") ||
       deltaProfTX.Contains("perp")) deltaProfTX += " [GeV]";
    deltaDist[a]->GetXaxis()->SetTitle(deltaProfTX);
    deltaDist[a]->GetYaxis()->SetTitle("#delta");
    deltaDist[a]->GetYaxis()->CenterTitle();
    deltaDist[a]->GetYaxis()->SetTitleOffset(1.1);
    deltaDist[a]->GetXaxis()->SetTitleOffset(1.1);
    deltaDist[a]->GetXaxis()->SetLabelSize(0.06);
    deltaDist[a]->GetYaxis()->SetLabelSize(0.06);
    deltaDist[a]->GetXaxis()->SetTitleSize(0.06);
    deltaDist[a]->GetYaxis()->SetTitleSize(0.06);

    deltaProf[a]->SetLineWidth(4);
    deltaProf[a]->SetLineColor(kBlack);
    //deltaProf[a]->Fit("pol0");
  };

  gStyle->SetOptStat(0);
  //gStyle->SetOptFit(1);
  TCanvas * deltaProfCanv = new TCanvas("deltaProfCanv","deltaProfCanv",2*1200,4*600);
  deltaProfCanv->Divide(2,4);
  for(int a=0; a<N_AMP; a++) {
    deltaProfCanv->cd(a+1);
    deltaProfCanv->GetPad(a+1)->SetGrid(1,1);
    deltaProfCanv->GetPad(a+1)->SetBottomMargin(0.15);
    deltaProfCanv->GetPad(a+1)->SetLeftMargin(0.15);
    deltaProf[a]->GetYaxis()->SetRangeUser(-deltaMax,deltaMax);
    //deltaProf[a]->Draw();
    deltaDist[a]->SetLineWidth(2);
    deltaDist[a]->SetLineColor(kRed);
    deltaDist[a]->Draw("box");
    deltaProf[a]->Draw("same");
  };


  outfile->cd();
  deltaProfCanv->Write();
  for(int a=0; a<N_AMP; a++) deltaDist[a]->Write();
  for(int a=0; a<N_AMP; a++) deltaProf[a]->Write();

  TString pngname = infileDir;
  pngname(TRegexp("^deltaset")) = "deltaProf";
  pngname += ".png";
  deltaProfCanv->Print(pngname,"png");
};
