// check fit result of injected asymmetry against the injected model,
// given a fit result `asym_*.root` file and an injection model file `injection.root`
R__LOAD_LIBRARY(DiSpin)
#include "Constants.h"

void checkInjectionFit(
    TString fitResultFileN="bruspin.x.inj0/asym_minuit_BL0.root",
    TString injectionModelFileN="injection.root",
    Int_t injNum=0 // which injection model number
    )
{
  // set output file and directory
  TString outFileN = fitResultFileN;
  outFileN(TRegexp("\\.root$")) = ".injectionTest.root";
  TFile *outFile = new TFile(outFileN,"RECREATE");
  TString imgDir = fitResultFileN;
  imgDir(TRegexp("\\/asym")) = "/injectionResults";
  imgDir(TRegexp("\\.root$")) = "";
  cout << "mkdir " << imgDir << endl;
  gROOT->ProcessLine(".! mkdir -p "+imgDir);

  // get injection model
  TFile *injectionModelFile = new TFile(injectionModelFileN,"READ");
  InjectionModel *IM = (InjectionModel*) injectionModelFile->Get("IM");
  Int_t nDim = IM->GetBinning()->dimensions;

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
  TObjArray *bruBins = (TObjArray*) fitResultFile->Get("BruBinList");

  // canvases
  Int_t ncol = 4;
  Int_t nrow = (IM->GetNumModulations()-1)/ncol+1; // cf. `drawBru.C` values
  canvAsym->Draw();
  auto canvWidth = canvAsym->GetWindowWidth();
  auto canvHeight = canvAsym->GetWindowHeight();
  TCanvas *canvComp = new TCanvas("canvComp","canvComp",canvWidth,canvHeight);
  TCanvas *canvDiff = new TCanvas("canvDiff","canvDiff",canvWidth,canvHeight);
  TCanvas *canvPull = new TCanvas("canvPull","canvPull",canvWidth,canvHeight);
  canvComp->Divide(ncol,nrow);
  canvDiff->Divide(ncol,nrow);
  canvPull->Divide(ncol,nrow);

  // objects
  TLine *zeroLine;
  TLine *oneLine[2];
  TF1 *injFtn;
  TGraphErrors *paramGr,*injeGr,*diffGr,*pullGr;
  TMultiGraph *compareMgr;

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
        switch(nDim) {
          case 1: injFtn = (TF1*) IM->GetAmplitudeModel(moduName,injNum); break;
          case 2: injFtn = (TF2*) IM->GetAmplitudeModel(moduName,injNum); break;
          case 3: injFtn = (TF3*) IM->GetAmplitudeModel(moduName,injNum); break;
          default:
                  fprintf(stderr,"ERROR: unknown nDim\n");
                  return;
        };

        // graphs for residual and pull between fit result and model
        Double_t asym,iv,asymErr,diff,diffErr,pull,pullErr;
        injeGr = new TGraphErrors(); injeGr->SetName("inje_"+paramGrN); injeGr->SetTitle("injected "+paramGrT);
        diffGr = new TGraphErrors(); diffGr->SetName("diff_"+paramGrN); diffGr->SetTitle("#Delta"+paramGrT);
        pullGr = new TGraphErrors(); pullGr->SetName("pull_"+paramGrN); pullGr->SetTitle("Pull "+paramGrT);
        injeGr->SetMarkerStyle(kFullFourTrianglesX); injeGr->SetMarkerColor(kBlack);     injeGr->SetLineColor(kBlack);
        diffGr->SetMarkerStyle(kFullCircle);         diffGr->SetMarkerColor(kGreen+2);   diffGr->SetLineColor(kGreen+2);
        pullGr->SetMarkerStyle(kFullCircle);         pullGr->SetMarkerColor(kMagenta+2); pullGr->SetLineColor(kMagenta+2);
        injeGr->SetMarkerSize(2);
        Float_t axisTitleSize = 0.06;
        gStyle->SetTitleSize(0.06,"T");
        paramGr->GetXaxis()->SetLabelSize(axisTitleSize);    paramGr->GetYaxis()->SetLabelSize(axisTitleSize);
        injeGr->GetXaxis()->SetLabelSize(axisTitleSize);     injeGr->GetYaxis()->SetLabelSize(axisTitleSize);
        diffGr->GetXaxis()->SetLabelSize(axisTitleSize);     diffGr->GetYaxis()->SetLabelSize(axisTitleSize);
        pullGr->GetXaxis()->SetLabelSize(axisTitleSize);     pullGr->GetYaxis()->SetLabelSize(axisTitleSize);
        compareMgr = new TMultiGraph();
        compareMgr->Add(paramGr);
        compareMgr->Add(injeGr);
        compareMgr->SetTitle(paramGr->GetTitle());

        // fill graphs
        for(int k=0; k<paramGr->GetN(); k++) {
          paramGr->GetPoint(k,iv,asym);
          asymErr = paramGr->GetErrorY(k);

          // get BruBin means
          BruBin *bb = (BruBin*) bruBins->At(k); // assume graph point number = bruBins index
          Double_t ivMean[3];
          for(int d=0; d<nDim; d++) {
            ivMean[d] = bb->GetIvMean( IM->GetBinning()->GetIVname(d) );
            if(TMath::Abs(ivMean[d]-UNDEF) < 0.1)
              fprintf(stderr,"ERROR: bad mean for IV %s\n",IM->GetBinning()->GetIVname(d).Data());
          }
          if(TMath::Abs(iv-ivMean[0]) > 0.001) // cross check read mean with stored mean
            fprintf(stderr,"WARNING: plotted ivMean and BruBin ivMean differ\n");

          // evaluate injected value at <iv>
          Double_t injFtnVal;
          switch(nDim) {
            case 1: injFtnVal = injFtn->Eval(ivMean[0]); break;
            case 2: injFtnVal = injFtn->Eval(ivMean[0],ivMean[1]); break;
            case 3: injFtnVal = injFtn->Eval(ivMean[0],ivMean[1],ivMean[2]); break;
          };
          injeGr->SetPoint(k,iv,injFtnVal);

          // calculate residual
          diff = asym - injFtnVal;
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

        // draw asymmetry result compared with injected asym
        canvComp->cd(padN);
        compareMgr->Draw("APE");
        compareMgr->GetXaxis()->SetLabelSize(axisTitleSize); compareMgr->GetYaxis()->SetLabelSize(axisTitleSize);
        compareMgr->GetYaxis()->SetRangeUser(-0.2,0.2);
        // if(nDim==1) injFtn->Draw("SAME");

      }
    }
  }

  // draw
  canvComp->Draw();
  canvDiff->Draw();
  canvPull->Draw();

  // print
  canvComp->SaveAs(imgDir+"/asym.png");
  canvDiff->SaveAs(imgDir+"/diff.png");
  canvPull->SaveAs(imgDir+"/pull.png");

  // write
  outFile->cd();
  canvComp->Write();
  canvDiff->Write();
  canvPull->Write();

  // cleanup
  printf("wrote %s\n",outFileN.Data());
  outFile->Close();
  injectionModelFile->Close();
  fitResultFile->Close();
}

