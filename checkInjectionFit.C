// check fit result of injected asymmetry against the injected model,
// given a fit result `asym_*.root` file and an injection model file `injection.root`
R__LOAD_LIBRARY(DiSpin)

void checkInjectionFit(
    TString fitResultFileN="bruspin.x.inj0/asym_minuit_BL0.root",
    TString injectionModelFileN="injection.root",
    Int_t injNum=0
    )
{
  // set output file and directory
  TString outFileN = fitResultFileN;
  outFileN(TRegexp("\\.root$")) = ".injectionTest.root";
  TFile *outFile = new TFile(outFileN,"RECREATE");
  TString imgDir = fitResultFileN;
  imgDir(TRegexp("\\/asym.*$")) = "/injectionResults";
  cout << "mkdir " << imgDir << endl;
  gROOT->ProcessLine(".! mkdir -p "+imgDir);

  // get injection model
  TFile *injectionModelFile = new TFile(injectionModelFileN,"READ");
  InjectionModel *IM = (InjectionModel*) injectionModelFile->Get("IM");

  // find fit result `canvAsym*`, assuming there is only one
  TFile *fitResultFile = new TFile(fitResultFileN,"READ");
  TListIter nextKey(fitResultFile->GetListOfKeys());
  TString keyN;
  TCanvas *canvAsym = nullptr;
  while(TKey *key = (TKey*) nextKey()) {
    keyN = TString(key->GetName());
    if(keyN.Contains(TRegexp("^canvAsym_"))) {
      printf("Found asymmetry canvas %s in %s\n",keyN.Data(),fitResultFileN.Data());
      canvAsym = (TCanvas*) key->ReadObj()->Clone();
      break;
    }
  }
  if(canvAsym==nullptr) {
    fprintf(stderr,"ERROR: cannot find canvAsym in %s\n",fitResultFileN.Data());
    return;
  }

  // canvases
  Int_t ncol = 4;
  Int_t nrow = IM->GetNumModulations()/ncol+1; // cf. `drawBru.C` values
  canvAsym->Draw();
  auto canvWidth = canvAsym->GetWindowWidth();
  auto canvHeight = canvAsym->GetWindowHeight();
  TCanvas *canvDiff = new TCanvas("canvDiff","canvDiff",canvWidth,canvHeight);
  TCanvas *canvPull = new TCanvas("canvPull","canvPull",canvWidth,canvHeight);
  canvDiff->Divide(ncol,nrow);
  canvPull->Divide(ncol,nrow);

  // objects
  TLine *zeroLine;
  TLine *oneLine[2];
  TF1 *injFtn;
  TGraphErrors *paramGr,*diffGr,*pullGr;

  // loop over pads of canvAsym
  Bool_t first;
  TListIter nextPad(canvAsym->GetListOfPrimitives());
  while(TPad *pad = (TPad*) nextPad()) {
    Int_t padN = pad->GetNumber();
    TListIter nextPrim(pad->GetListOfPrimitives());
    while(TObject *prim = (TObject*) nextPrim()) {
      if(prim->InheritsFrom(TGraph::Class())) {

        // get parameter graph
        if(TString(prim->GetName()).Contains("Yld")) continue; // skip yield param
        paramGr = (TGraphErrors*) prim;
        TString paramGrN = paramGr->GetName();
        TString paramGrT = paramGr->GetTitle();

        // draw lines
        zeroLine = new TLine(paramGr->GetXaxis()->GetXmin(),0,paramGr->GetXaxis()->GetXmax(),0);
        oneLine[0] = new TLine(paramGr->GetXaxis()->GetXmin(),1,paramGr->GetXaxis()->GetXmax(),1);
        oneLine[1] = new TLine(paramGr->GetXaxis()->GetXmin(),-1,paramGr->GetXaxis()->GetXmax(),-1);
        zeroLine->SetLineColor(kBlack);
        zeroLine->SetLineWidth(2);
        zeroLine->SetLineStyle(kDashed);
        for(int o=0; o<2; o++) {
          oneLine[o]->SetLineColor(kGray+2);
          oneLine[o]->SetLineWidth(1);
          oneLine[o]->SetLineStyle(kDashed);
        }

        // get injection model `injFtn`
        TString moduName = paramGrN;
        moduName(TRegexp("^gr_")) = "";
        moduName(TRegexp("_BL.*")) = "";
        printf("paramGrName=%s\tmodu=%s\n",paramGr->GetName(),moduName.Data());
        injFtn = (TF1*) IM->GetAmplitudeModel(moduName,injNum);

        // draw model on canvAsym
        pad->cd();
        paramGr->GetYaxis()->UnZoom();
        injFtn->Draw("SAME");

        // difference and pull between fit result and model
          /* note: when we do 2D injections, it won't be what the arguments of
           * injFtn->Eval() should be; try writing out TGraphs of the means of
           * each variable in catTree; the best place for this idea is in
           * drawBru.C; then here we can read these TGraphs for the mean values
           */
        Double_t asym,iv,asymErr,diff,diffErr,pull,pullErr;
        diffGr = new TGraphErrors(); diffGr->SetName("diff_"+paramGrN); diffGr->SetTitle("#Delta"+paramGrT);
        pullGr = new TGraphErrors(); pullGr->SetName("pull_"+paramGrN); pullGr->SetTitle("pull "+paramGrT);
        diffGr->SetMarkerStyle(kFullCircle); diffGr->SetMarkerColor(kGreen+2);   diffGr->SetLineColor(kGreen+2);
        pullGr->SetMarkerStyle(kFullCircle); pullGr->SetMarkerColor(kMagenta+2); pullGr->SetLineColor(kMagenta+2);
        for(int k=0; k<paramGr->GetN(); k++) {
          paramGr->GetPoint(k,iv,asym);
          asymErr = paramGr->GetErrorY(k);

          // calculate residual
          diff = asym - injFtn->Eval(iv);
          diffErr = asymErr;
          diffGr->SetPoint(k,iv,diff);
          diffGr->SetPointError(k,0,diffErr);

          // calculate pull
          pull = diff / asymErr;
          pullErr = diffErr / asymErr;
          pullGr->SetPoint(k,iv,pull);
          pullGr->SetPointError(k,0,pullErr);
        }

        // draw residual
        canvDiff->cd(padN);
        zeroLine->Draw();
        diffGr->Draw("APE");
        zeroLine->Draw();

        // draw pull
        canvPull->cd(padN);
        zeroLine->Draw();
        pullGr->Draw("APE");
        pullGr->GetYaxis()->SetRangeUser(-5,5);
        zeroLine->Draw();
        for(int o=0; o<2; o++) oneLine[o]->Draw();
      }
    }
  }

  // draw
  canvAsym->Draw();
  canvDiff->Draw();
  canvPull->Draw();

  // print
  canvAsym->SaveAs(imgDir+"/asym.png");
  canvDiff->SaveAs(imgDir+"/diff.png");
  canvPull->SaveAs(imgDir+"/pull.png");

  // write
  outFile->cd();
  canvAsym->Write();
  canvDiff->Write();
  canvPull->Write();

  // cleanup
  printf("wrote %s\n",outFileN.Data());
  outFile->Close();
  injectionModelFile->Close();
  fitResultFile->Close();
}

