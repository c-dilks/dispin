const int nD = 5; // number of D parameters
const int nA = 3; // number of asymmetry amplitudes
const int nB = 3; // number of bins for asymmetry

TCanvas * canvPlot(TH1D * dist[nA][nB]);

void analyse() {

  // get list of spinroot/asym*.root files
  TString rootDir = "spinroot";
  TString rootFileName;
  std::vector<TString> rootFileList;
  TSystemDirectory * sysDir = new TSystemDirectory(rootDir,rootDir);
  TList * sysFileList = sysDir->GetListOfFiles();
  TIter nxt(sysFileList);
  TSystemFile * sysFile;
  while(( sysFile = (TSystemFile*) nxt() )) {
    rootFileName = rootDir + "/" + TString(sysFile->GetName());
    if( !sysFile->IsDirectory() && 
        rootFileName.EndsWith(".root") &&
        rootFileName.Contains("asym_") ) {
      rootFileList.push_back(rootFileName);
    };
  };

  // define output plots
  TString plotN,plotT;
  TH1D * asymDist[nA][nB];
  TH1D * asymDistDiff[nA][nB];
  TH1D * asymDistRatio[nA][nB];
  for(int a=0; a<nA; a++) {
    for(int b=0; b<nB; b++) {
      plotN = Form("asymDist_A%d_bin%d",a,b);
      plotT = plotN;
      asymDist[a][b] = new TH1D(plotN,plotT,100,-0.07,0.07);
      plotN = Form("asymDistDiff_A%d_bin%d",a,b);
      plotT = plotN;
      asymDistDiff[a][b] = new TH1D(plotN,plotT,100,-0.07,0.07);
      plotN = Form("asymDistRatio_A%d_bin%d",a,b);
      plotT = plotN;
      asymDistRatio[a][b] = new TH1D(plotN,plotT,100,-3,3);
    };
  };
  TH1D * Ddist[nD];
  TH2D * DvsD3dist[nD];
  for(int d=0; d<nD; d++) {
    plotN = Form("Ddist_D%d",d);
    plotT = Form("Sampled fraction of D_{%d} values",d);
    Ddist[d] = new TH1D(plotN,plotT,5,-0.7,0.7);
    plotN = Form("DvsD3dist_D%d",d);
    plotT = Form("Sampled fraction of (D_{3},D_{%d}) values;D_{3};D_{%d}",d,d);
    DvsD3dist[d] = new TH2D(plotN,plotT,5,-0.7,0.7,5,-0.7,0.7);
  };


  // get un-modified asymmetry values (all D params == 0),
  // and set plot titles
  TGraphErrors * asymGr;
  TString asymGrN;
  Double_t asym,iv;
  Float_t asymBase[nA][nB];
  TFile * baseFile = new TFile(TString(rootDir+"/asym_200.root"),"READ");
  for(int a=0; a<nA; a++) {
    asymGrN = Form("kindepMA_A%d_M",a);
    asymGr = (TGraphErrors*) baseFile->Get(asymGrN);
    for(int b=0; b<nB; b++) {

      // get un-modified asymmetry values
      asymGr->GetPoint(b,iv,asym);
      asymBase[a][b] = asym;

      // set plot titles
      plotT = asymGr->GetTitle();
      plotT(TRegexp("vs..*;")) = "for ";
      plotT = Form("%s bin %d",plotT.Data(),b);
      asymDist[a][b]->SetTitle(TString("asymmetry values: "+plotT));
      asymDistDiff[a][b]->SetTitle(TString("asymmetry differences: "+plotT));
      asymDistRatio[a][b]->SetTitle(TString("asymmetry ratios: "+plotT));
    };
  };


  // loop over root files
  TFile * rootFile;
  Float_t Dval[nD];
  TString fname, token;
  Ssiz_t tf;
  int cnt;
  Bool_t badFit;
  for(TString rootFileN : rootFileList) {
    printf("opening %s\n",rootFileN.Data());
    rootFile = new TFile(rootFileN,"READ");
    
    // get D parameter values
    fname = rootFileN;
    fname(TRegexp("^.*/")) = "";
    fname(TRegexp("\\.root$")) = "";
    tf = 0;
    cnt = 0;
    for(int d=0; d<nD; d++) Dval[d]=0;
    while(fname.Tokenize(token,tf,"_")) {
      if(cnt>1) Dval[cnt-2] = token.Atof();
      cnt++;
    };
    for(int d=0; d<nD; d++) printf(" D%d = %.2f\n",d,Dval[d]);

    // CUT on D
    //if(TMath::Abs(Dval[3])>0) continue;
    //if(TMath::Abs(Dval[0])>0) continue;
    if(Dval[3]<0.5) continue;

    // CUT out bad fits
    badFit = false;
    for(int a=0; a<nA; a++) {
      asymGrN = Form("kindepMA_A%d_M",a);
      asymGr = (TGraphErrors*) rootFile->Get(asymGrN);
      for(int b=0; b<nB; b++) {
        asymGr->GetPoint(b,iv,asym);
        if(TMath::Abs(asym)<0.000001) badFit=true;
      };
    };
    if(badFit) continue;


    // fill distributions
    for(int a=0; a<nA; a++) {
      asymGrN = Form("kindepMA_A%d_M",a);
      asymGr = (TGraphErrors*) rootFile->Get(asymGrN);
      for(int b=0; b<nB; b++) {
        asymGr->GetPoint(b,iv,asym);


        // fill plots
        asymDist[a][b]->Fill(asym);
        asymDistDiff[a][b]->Fill(asym-asymBase[a][b]);
        asymDistRatio[a][b]->Fill(asym/asymBase[a][b]);
      };
    };
    for(int d=0; d<nD; d++) {
      Ddist[d]->Fill(Dval[d]);
      DvsD3dist[d]->Fill(Dval[3],Dval[d]);
    };
  };


  // plot on canvases
  TCanvas * asymDistCanv = canvPlot(asymDist);
  TCanvas * asymDistDiffCanv = canvPlot(asymDistDiff);
  TCanvas * asymDistRatioCanv = canvPlot(asymDistRatio);
  TCanvas * Dcanv[nD];
  TString DcanvN;
  for(int d=0; d<nD; d++) {
    DcanvN = Form("D%dcanv",d);
    Dcanv[d] = new TCanvas(DcanvN,DcanvN,600,600);
    Dcanv[d]->Divide(1,2);
    Dcanv[d]->cd(1);
    Ddist[d]->Scale(1/Ddist[d]->GetEntries());
    Ddist[d]->GetYaxis()->SetRangeUser(0,0.6);
    Ddist[d]->Draw();
    Dcanv[d]->cd(2);
    DvsD3dist[d]->Scale(1/DvsD3dist[d]->GetEntries());
    DvsD3dist[d]->Draw("boxtext");
  };
};


TCanvas * canvPlot(TH1D * dist[nA][nB]) {
  TString canvN = dist[0][0]->GetName();
  canvN(TRegexp("_.*$")) = "Canv";
  TCanvas * canv = new TCanvas(canvN,canvN,1000,1000);
  canv->Divide(nA,nB);
  for(int a=0; a<nA; a++) {
    for(int b=0; b<nB; b++) {
      canv->cd(a*nA+b+1);
      dist[a][b]->Draw();
    };
  };
  return canv;
};
