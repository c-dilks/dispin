R__LOAD_LIBRARY(DiSpin)
#include "Constants.h"

Double_t getDepolarization(BruBin *B, Int_t version, Int_t twist);

/////////////////////////////////////////////////////

void drawBru(
  TString bruDir = "bruspin",
  TString minimizer_ = "minuit",
  Bool_t logscale = true, /* plot sample# on log scale */
  Float_t asymPlotMin_ = -10, /* units: % */
  Float_t asymPlotMax_ =  10  /* units: % */
) {

  Float_t asymPlotMin = asymPlotMin_ / 100.0;
  Float_t asymPlotMax = asymPlotMax_ / 100.0;
  gStyle->SetTitleSize(0.08,"T");
  Float_t axisTitleSize = 0.05;
  gStyle->SetOptStat(0);

  // get minimizer type
  Int_t minimizer = MinimizerStrToEnum(minimizer_);
  if(minimizer<0) return;

  // get binning scheme
  TFile * binFile = new TFile(bruDir+"/DataBinsConfig.root","READ");
  HS::FIT::Bins * HSbins = (HS::FIT::Bins*) binFile->Get("HSBins");
  binFile->Close();

  // build arrays of BruBin objects
  // - BruBinList: bins for the first dimension (IV0)
  // - BruBinSuperList: array of BruBinLists, one for each higher-dimensional bin
  // -- get number of bins for each dimension:
  TString hTitle = "";
  TString vTitle = "";
  TObjArray * BruBinList;
  TObjArray * BruBinSuperList;
  int bn[3];
  int bnMax[3];
  for(int i=0; i<3; i++) {
    bnMax[i] = (i<HSbins->GetNAxis()) ? HSbins->GetVarAxis()[i].GetNbins() : 1;
  };
  // -- create BruBin objects, and fill TObjArrays
  BruBinSuperList = new TObjArray();
  for(bn[2]=1; bn[2]<=bnMax[2]; bn[2]++) {
    for(bn[1]=1; bn[1]<=bnMax[1]; bn[1]++) {
      BruBinList = new TObjArray(); // create new BruBinList before looping over dimension 0
      for(bn[0]=1; bn[0]<=bnMax[0]; bn[0]++) {
        // create new BruBin object
        BruBinList->AddLast(new BruBin(bruDir,HSbins,bn[0],bn[1],bn[2]));
        if(hTitle=="") hTitle = ((BruBin*)BruBinList->At(0))->GetIvName(0);
      }
      BruBinSuperList->AddLast(BruBinList);
    };
  };


  // define BruBin iterator, run BruBin actions
  BruBin * BB;
  TObjArray * BBlist;
  TObjArrayIter nextBin(BruBinList);
  TObjArrayIter nextBinList(BruBinSuperList);
  while((BBlist = (TObjArray*) nextBinList())) {
    nextBin = TObjArrayIter(BBlist);
    while((BB = (BruBin*) nextBin())) {
      BB->CalculateStats();
      if( ! BB->OpenResultFile(minimizer)) return;
    };
  };


  // get parameter values from results trees
  Bool_t first = true;
  RooDataSet * paramSet;
  TString paramList[BruBin::nParamsMax];
  Modulation * moduList[BruBin::nParamsMax];
  Int_t nParams;
  TString paramName;
  Double_t paramval[BruBin::nParamsMax];
  Double_t nll;
  Long64_t entry;
  nextBinList = TObjArrayIter(BruBinSuperList);
  while((BBlist = (TObjArray*) nextBinList())) {
    nextBin = TObjArrayIter(BBlist);
    while((BB = (BruBin*) nextBin())) {

      // get parameter tree
      paramSet = (RooDataSet*) BB->GetResultFile()->Get("FinalParameters");

      // get parameter list
      if(first) {
        nParams = 0;
        for(int i=0; i<paramSet->get()->size(); i++) {
          if(nParams>BruBin::nParamsMax) {
            fprintf(stderr,"ERROR: too many params\n"); return; };
          paramName = (*(paramSet->get()))[i]->GetName();
          if(paramName=="NLL") continue;
          if(paramName.Contains("Yld")) moduList[nParams] = nullptr;
          else moduList[nParams] = new Modulation(paramName);
          paramList[nParams] = paramName;
          printf("param %d:  %s\n",nParams,paramList[nParams].Data());
          nParams++;
        };
        Tools::PrintSeparator(30);
        first = false;
      };

      // get parameter values
      if(BB->GetResultTree()->GetEntries()!=1)
        fprintf(stderr,"WARNING: ResultTree does not have 1 entry\n");
      for(int i=0; i<nParams; i++) {
        BB->GetResultTree()->SetBranchAddress(paramList[i],&(BB->paramVal[i]));
        BB->GetResultTree()->SetBranchAddress(paramList[i]+"_err",&(BB->paramErr[i]));
        BB->GetResultTree()->GetEntry(0);
      };

      // if MCMC was used, fill param vs sample graphs
      if(IsMCMC(minimizer)) {
        BB->GetMcmcTree()->SetBranchAddress("entry",&entry);
        BB->GetMcmcTree()->SetBranchAddress("nll_MarkovChain_local_",&nll);
        for(int i=0; i<nParams; i++) {
          BB->GetMcmcTree()->SetBranchAddress(paramList[i],&paramval[i]);
        };
        for(int i=0; i<nParams; i++) {
          vTitle = moduList[i] ? moduList[i]->AsymmetryTitle() : "N";
          BB->GetParamVsSampleHist(i)->SetTitle(
            vTitle+" vs. MCMC sample"/*;sample;"+vTitle*/);
          BB->GetParamVsSampleHist(i)->GetXaxis()->SetLabelSize(axisTitleSize);
          BB->GetParamVsSampleHist(i)->GetYaxis()->SetLabelSize(axisTitleSize);
        };
        BB->GetNllVsSampleHist()->SetTitle("-ln(L) vs. MCMC sample");
        BB->GetNllVsSampleHist()->GetXaxis()->SetLabelSize(axisTitleSize);
        BB->GetNllVsSampleHist()->GetYaxis()->SetLabelSize(axisTitleSize);
        for(Long64_t e=0; e<BB->GetMcmcTree()->GetEntries(); e++) {
          BB->GetMcmcTree()->GetEntry(e);
          for(int i=0; i<nParams; i++) {
            BB->GetParamVsSampleHist(i)->Fill(entry+1,paramval[i]);
          };
          BB->GetNllVsSampleHist()->Fill(entry+1,nll);
        };
      };

    };
  };



  // build graphs and canvases
  TString outfileN;
  TFile * outFile;
  TGraphErrors * paramGr[BruBin::nParamsMax];
  Int_t cnt;
  TCanvas * paramCanv;
  TCanvas * paramVsSampleCanv;
  TCanvas * cornerCanv;
  TCanvas * autocorrCanv;
  Float_t xMin,xMax,yMin,yMax;
  TLine * zeroLine;
  Int_t nrow,ncol;
  Int_t binListCnt=0;
  TString blStr;
  nextBinList = TObjArrayIter(BruBinSuperList);
  while((BBlist = (TObjArray*) nextBinList())) {

    // setup output file
    blStr = Form("_BL%d",binListCnt); // BL = Bin List
    outfileN = bruDir+Form("/asym_%s%s.root",minimizer_.Data(),blStr.Data());
    outFile = new TFile(outfileN,"RECREATE");


    Tools::PrintSeparator(30,"-");
    printf("%s will have bins:\n",outfileN.Data());
    nextBin = TObjArrayIter(BBlist);
    while((BB = (BruBin*) nextBin())) BB->PrintInfo();
    Tools::PrintSeparator(30,"-");

    // paramter vs. bin mean graphs, for each parameter
    for(int i=0; i<nParams; i++) {

      // define graph
      paramGr[i] = new TGraphErrors();
      paramGr[i]->SetName("gr_"+paramList[i]+blStr);
      vTitle = moduList[i] ? moduList[i]->AsymmetryTitle() : "N";
      paramGr[i]->SetTitle(vTitle+" vs. "+hTitle/*+";"+hTitle+";"+vTitle*/);
      paramGr[i]->GetXaxis()->SetLabelSize(axisTitleSize);
      paramGr[i]->GetYaxis()->SetLabelSize(axisTitleSize);
      paramGr[i]->SetMarkerStyle(kFullCircle);
      paramGr[i]->SetMarkerColor(kAzure);
      paramGr[i]->SetLineColor(kAzure);

      // add points to graph and write
      cnt=0;
      nextBin = TObjArrayIter(BBlist);
      Int_t twist = moduList[i] ? moduList[i]->GetTw() : 0; // yield param is "twist 0"
      while((BB = (BruBin*) nextBin())) {
        Double_t asymVal = BB->paramVal[i];
        Double_t asymErr = BB->paramErr[i];
        /////////////////////////////////////////////////////////
        //// test depolarization factor usage downstream of likelihood fit
        //Double_t depol = getDepolarization(BB,0,twist);
        //asymVal /= depol;
        //asymErr /= depol;
        /////////////////////////////////////////////////////////
        paramGr[i]->SetPoint(cnt,BB->GetIvMean(0),asymVal);
        paramGr[i]->SetPointError(cnt,0,asymErr);
        cnt++;
      };
      paramGr[i]->Write();
    };

    // canvas for paramter vs. bin mean graphs
    ncol=4; nrow=(nParams-1)/ncol+1;
    paramCanv = new TCanvas("canvAsym"+blStr,"canvAsym"+blStr,600*ncol,300*nrow);
    paramCanv->Divide(ncol,nrow);
    for(int i=0; i<nParams; i++) {
      paramCanv->cd(i+1);
      paramCanv->GetPad(i+1)->SetGrid(1,1);
      yMin = asymPlotMin;
      yMax = asymPlotMax;
      if(paramGr[i]->GetYaxis()->GetXmin() < yMin)
        yMin = paramGr[i]->GetYaxis()->GetXmin();
      if(paramGr[i]->GetYaxis()->GetXmax() > yMax)
        yMax = paramGr[i]->GetYaxis()->GetXmax();
      paramGr[i]->GetYaxis()->SetRangeUser(yMin,yMax);
      paramGr[i]->Draw("APE");
      xMin = paramGr[i]->GetXaxis()->GetXmin();
      xMax = paramGr[i]->GetXaxis()->GetXmax();
      if(!paramList[i].Contains("Yld")) {
        zeroLine = new TLine(xMin,0,xMax,0);
        zeroLine->SetLineColor(kBlack);
        zeroLine->SetLineWidth(2);
        zeroLine->SetLineStyle(kDashed);
        zeroLine->Draw();
      };
    };
    paramCanv->Draw();
    paramCanv->Write();

    // parameter vs. sample
    if(IsMCMC(minimizer)) {
      nrow=nParams/ncol+1; // (update for NLL)
      nextBin = TObjArrayIter(BBlist);
      while((BB = (BruBin*) nextBin())) {
        paramVsSampleCanv = new TCanvas(
          Form("paramVsSample_%d",BB->GetBruIdx())+blStr,
          Form("paramVsSample_%d",BB->GetBruIdx())+blStr,
          600*ncol,300*nrow);
        paramVsSampleCanv->Divide(ncol,nrow);
        for(int i=0; i<nParams; i++) {
          paramVsSampleCanv->cd(i+1);
          if(logscale) gPad->SetLogx();
          if(!paramList[i].Contains("Yld"))
            BB->GetParamVsSampleHist(i)->GetYaxis()->SetRangeUser(
              asymPlotMin,asymPlotMax);
          BB->GetParamVsSampleHist(i)->Draw("HIST");
        };
        paramVsSampleCanv->cd(nParams+1);
        if(logscale) gPad->SetLogx();
        BB->GetNllVsSampleHist()->Draw("HIST");
        paramVsSampleCanv->Write();
        paramVsSampleCanv->Close();

        cornerCanv = (TCanvas*) BB->GetResultFile()->Get("Corner Full Plot")->Clone();
        //cornerCanv = (TCanvas*) BB->GetResultFile()->Get("Corner Plot")->Clone();
        cornerCanv->Write(Form("cornerCanv_%d"+blStr,BB->GetBruIdx()));

        autocorrCanv = (TCanvas*) BB->GetResultFile()->Get("Autocorrelation Plot")->Clone();
        autocorrCanv->Write(Form("autocorrCanv_%d"+blStr,BB->GetBruIdx()));
      };
    };

    binListCnt++;
    BBlist->Write("BruBinList",TObject::kSingleKey);
    outFile->Close();
    printf("produced %s\n",outfileN.Data());
  };

  // cleanup
  // nextBinList = TObjArrayIter(BruBinSuperList);
  // while((BBlist = (TObjArray*) nextBinList())) {
  //   nextBin = TObjArrayIter(BBlist);
  //   while((BB = (BruBin*) nextBin())) {
  //    BB->GetResultFile()->Close();
  //   };
  // };

};



/////////////////////////////////////////////////////



Double_t getDepolarization(BruBin *B, Int_t version, Int_t twist) {
  Double_t meanDepolA, meanDepolC, meanDepolW, meanDepol2, meanDepol3;
  if(version>0) {
    meanDepolA = B->GetIvMean("DepolA");
    meanDepolC = B->GetIvMean("DepolC");
    meanDepolW = B->GetIvMean("DepolW");
    meanDepol2 = B->GetIvMean("Depol2");
    meanDepol3 = B->GetIvMean("Depol3");
  };
  switch(version) {
    case 0: { // ignore depolization (either you already have it in the likelihood, or you do not want it at all)
      return 1.0;
      break;
    }
    case 1: { // <C>/<A> and <W>/<A>
      if(meanDepolA==0) {
        fprintf(stderr,"ERROR: meanDepolA=0, setting depol=1.0 for this bin\n");
        return 1.0;
      } else {
        switch(twist) {
          case 0: return 1.0; break;
          case 2: return meanDepolC / meanDepolA; break;
          case 3: return meanDepolW / meanDepolA; break;
        };
      };
      break;
    }
    case 2: { // <C/A> and <W/A>
      switch(twist) {
        case 0: return 1.0; break;
        case 2: return meanDepol2; break;
        case 3: return meanDepol3; break;
      };
      break;
    }
    case 3: { // C(<x>,<y>,<Q2>)/A(<x>,<y>,<Q2>) and W(<x>,<y>,<Q2>)/A(<x>,<y>,<Q2>)
      Double_t meanQ2 = B->GetIvMean("Q2");
      Double_t meanX  = B->GetIvMean("X");
      Double_t meanY  = B->GetIvMean("Y");
      Double_t gamma = 2*PartMass(kP)*meanX / TMath::Sqrt(meanQ2);
      Double_t epsilon = ( 1 - meanY - TMath::Power(gamma*meanY,2)/4 ) /
                         ( 1 - meanY + meanY*meanY/2 + TMath::Power(gamma*meanY,2)/4 );
      Double_t depolA = meanY*meanY / (2 - 2*epsilon); // A(x,y)
      Double_t depolB = depolA * epsilon;
      Double_t depolC = depolA * TMath::Sqrt(1-epsilon*epsilon);
      Double_t depolV = depolA * TMath::Sqrt(2*epsilon*(1+epsilon));
      Double_t depolW = depolA * TMath::Sqrt(2*epsilon*(1-epsilon));
      if(depolA==0) {
        fprintf(stderr,"ERROR: depolA=0, setting depol=1.0 for this bin\n");
        return 1.0;
      } else {
        switch(twist) {
          case 0: return 1.0; break;
          case 2: return depolC / depolA; break;
          case 3: return depolW / depolA; break;
        };
      };
      break;
    }
    default: {
      fprintf(stderr,"ERROR: unknown version in getDepolarization\n");
      return 1.0;
    }
  };
  fprintf(stderr,"ERROR: impossible error in getDepolarization\n");
  return 1.0;
};
