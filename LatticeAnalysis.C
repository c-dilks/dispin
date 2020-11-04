// procedure:
// - perform MC injection and fit
// - run Lattice1D.C, with desired B CUT
// - move all the difference_*.root files to the subdirectory `deltaset`
// - execute this script

void LatticeAnalysis() {

  /// OPTIONS /////////////////
  Float_t deltaMax = 0.01; // scale for delta plots
  /////////////////////////////


  // get list of difference*.root files
  TString infileDir = "deltaset";
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

  TFile * outfile = new TFile("deltaset/delta.root","RECREATE");

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
    cout << deltaDistT << endl;
    xmin = diffGr->GetXaxis()->GetXmin();
    xmax = diffGr->GetXaxis()->GetXmax();
    deltaDist[a] = new TH2D(deltaDistN,deltaDistT,
      50, xmin, xmax, 50, -deltaMax, deltaMax);
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
  TString deltaProfT;
  for(int a=0; a<N_AMP; a++) {
    deltaProf[a] = deltaDist[a]->ProfileX();
    deltaProfT = deltaDist[a]->GetTitle();
    deltaProfT(TRegexp("#delta distribution")) = "average #delta";
    deltaProf[a]->SetTitle(deltaProfT);
    deltaProf[a]->SetLineWidth(3);
    deltaProf[a]->SetLineColor(kRed);
  };

  gStyle->SetOptStat(0);
  TCanvas * deltaProfCanv = new TCanvas("deltaProfCanv","deltaProfCanv",1600,1200);
  deltaProfCanv->Divide(4,2);
  for(int a=0; a<N_AMP; a++) {
    deltaProfCanv->cd(a+1);
    deltaProfCanv->GetPad(a+1)->SetGrid(1,1);
    deltaProf[a]->GetYaxis()->SetRangeUser(-deltaMax,deltaMax);
    //deltaProf[a]->Draw();
    deltaDist[a]->Draw("box");
    deltaProf[a]->Draw("same");
  };


  outfile->cd();
  deltaProfCanv->Write();
  for(int a=0; a<N_AMP; a++) deltaDist[a]->Write();
  for(int a=0; a<N_AMP; a++) deltaProf[a]->Write();
};
