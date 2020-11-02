// compare asymmetries from two different asym*.root files
// - infile0name and infile1name are the asym*.root files to compare
// - predictShift: draw attempts of prediction of asymmetry difference
// - newTitle: change title of asymmetry difference plots
// - pngName: if newTitle!="", will print PNG of difference plots with
//            the specified name

R__LOAD_LIBRARY(DiSpin)
#include "Binning.h"

void CompareAsyms(TString infile0name="spinroot_pw_2/asym_9_mlm.root",
                  TString infile1name="spinroot_final_2/asym_42_mlm.root",
                  Bool_t predictShift=false,
                  TString newTitle="",
                  TString pngName="diff.png") {

  /// OPTIONS /////////////////
  Float_t diffMax = 0.02; // scale for difference plots
  /////////////////////////////


  // open asym*.root files
  TFile * infile[2];
  infile[0] = new TFile(infile0name,"READ");
  infile[1] = new TFile(infile1name,"READ");

  // read pointers
  TListIter nextKey(infile[0]->GetListOfKeys());
  TString keyname;
  TGraphAsymmErrors * gr[2];
  TObjArray * grArr[2];
  for(int g=0; g<2; g++) grArr[g] = new TObjArray();
  TMultiGraph * grMulti;

  // output plots
  TH1D * diffDist = new TH1D("diffDist","difference distribution",
    100,-diffMax,diffMax);
  TGraphAsymmErrors * diffGr;
  TGraphAsymmErrors * ratGr;
  TObjArray * diffGrArr = new TObjArray();
  TObjArray * ratGrArr = new TObjArray();

  TFile * distFile;
  if(newTitle!="") {
    TString distFileName = pngName;
    distFileName.ReplaceAll("png","root");
    distFile = new TFile(distFileName,"RECREATE");
  };
    

  Double_t x[2];
  Double_t y[2];
  Double_t ex[2];
  Double_t ey[2];
  Double_t ay[2];
  Double_t xx,exx;
  Double_t xdif,exdif;
  Double_t xrat,exrat;

  while(TKey * key = (TKey*) nextKey()) {
    keyname = TString(key->GetName());
    // read asymmetry graph
    if(keyname.Contains(TRegexp("^kindepMA")) &&
           !keyname.Contains("Canv")) {
      gr[0] = (TGraphAsymmErrors*) key->ReadObj();
      gr[1] = (TGraphAsymmErrors*) infile[1]->Get(keyname);
      diffGr = new TGraphAsymmErrors();
      ratGr = new TGraphAsymmErrors();
      
      for(int i=0; i<gr[0]->GetN(); i++) {
        for(int g=0; g<2; g++) {
          gr[g]->GetPoint(i,x[g],y[g]);
          ex[g] = gr[g]->GetErrorX(i); // (parabolic error)
          ey[g] = gr[g]->GetErrorY(i); // (parabolic error)
          ay[g] = fabs(y[g]);
        };
        gr[1]->SetPoint(i,x[1]+0.01,y[1]); // horizontal offset

        // compute difference
        xx = x[0];
        xdif = y[0]-y[1];
        xrat = ay[1]>0 ? ay[0]/ay[1] : 1;

        // correlated error (assumes dataset in infile[0] is a subset or
        // equal to the dataset in infile[1], or vice versa)
        exdif = TMath::Sqrt(TMath::Abs(ey[0]*ey[0]-ey[1]*ey[1]));
        exrat = ey[0]>0 && ey[1]>0 ?
                (ay[0]/ay[1])*TMath::Sqrt(
                  ey[0]*ey[0]/(ay[0]*ay[0])
                 +ey[1]*ey[1]/(ay[1]*ay[1])
                 -2*ey[0]*ey[0]/(ay[0]*ay[1])
                ) : 0;
        exx = ex[0]; // (not used)

        diffDist->Fill(xdif);

        diffGr->SetPoint(i,xx,xdif);
        diffGr->SetPointError(i,exx,exx,exdif,exdif);
        ratGr->SetPoint(i,xx,xrat);
        ratGr->SetPointError(i,exx,exx,exrat,exrat);
      };

      for(int g=0; g<2; g++) grArr[g]->AddLast(gr[g]);
      diffGrArr->AddLast(diffGr);
      ratGrArr->AddLast(ratGr);
    };
  };


  // draw
  Int_t nAmp = grArr[0]->GetEntries();
  TString grT;
  Int_t nrow = 1+(nAmp-1)/4;
  TCanvas * asymCanv = new TCanvas("asymGrCanv","asymGrCanv",4*300,nrow*300);
  TCanvas * diffCanv = new TCanvas("diffGrCanv","diffGrCanv",4*300,nrow*300);
  TCanvas * ratCanv = new TCanvas("ratGrCanv","ratGrCanv",4*300,nrow*300);
  asymCanv->Divide(4,nrow);
  diffCanv->Divide(4,nrow);
  ratCanv->Divide(4,nrow);
  Int_t pad;
  for(int i=0; i<nAmp; i++) {
    pad = i+1;
    if(pad>=8) pad = i+2; // (to re-align multidim plots)

    // draw asym graphs
    asymCanv->cd(pad);
    asymCanv->GetPad(pad)->SetGrid(1,1);
    for(int g=0; g<2; g++) {
      gr[g] = (TGraphAsymmErrors*) grArr[g]->At(i);
      gr[g]->GetYaxis()->SetRangeUser(-0.07,0.07);
      gr[g]->SetMarkerSize(0.75);
    };
    gr[0]->SetMarkerColor(kGreen+2);
    gr[0]->SetLineColor(kGreen+2);
    gr[1]->SetMarkerColor(kRed+2);
    gr[1]->SetLineColor(kRed+2);
    grT = gr[0]->GetTitle();
    grMulti = new TMultiGraph();
    grMulti->SetTitle(TString("comparison of "+grT));
    for(int g=0; g<2; g++) grMulti->Add(gr[g]);
    grMulti->GetYaxis()->SetRangeUser(-0.07,0.07);
    grMulti->Draw("APE");

    // draw diffGr
    diffCanv->cd(pad);
    diffCanv->GetPad(pad)->SetGrid(1,1);
    diffGr = (TGraphAsymmErrors*) diffGrArr->At(i);
    diffGr->GetYaxis()->SetRangeUser(-diffMax,diffMax);
    if(newTitle=="") diffGr->SetTitle(TString("difference for "+grT));
    else diffGr->SetTitle(TString(newTitle+" "+grT));
    diffGr->SetMarkerStyle(kFullCircle);
    diffGr->SetLineWidth(2);
    diffGr->SetMarkerColor(kBlack);
    diffGr->SetLineColor(kBlack);
    diffGr->Draw("APE");
    /*
    if(i==1) {
      diffGr->Fit("pol0");
      printf("%f); g->SetPointError(i++,0,%f);\n",
        diffGr->GetFunction("pol0")->GetParameter(0),
        diffGr->GetFunction("pol0")->GetParError(0));
    };
    */

    // draw ratGr
    ratCanv->cd(pad);
    ratCanv->GetPad(pad)->SetGrid(1,1);
    ratGr = (TGraphAsymmErrors*) ratGrArr->At(i);
    ratGr->GetYaxis()->SetRangeUser(0,5);
    ratGr->SetTitle(TString("ratio for "+grT));
    ratGr->SetMarkerStyle(kFullCircle);
    ratGr->SetLineWidth(2);
    ratGr->SetMarkerColor(kBlack);
    ratGr->SetLineColor(kBlack);
    ratGr->Draw("APE");
  };
  if(newTitle!="") diffCanv->Print(pngName,"png");

  TCanvas * canv2 = new TCanvas("diffDistCanv","diffDistCanv",800,800);
  diffDist->SetFillColor(kBlack);
  diffDist->Draw();
  if(newTitle!="") {
    distFile->cd();
    diffDist->Write();
  };



  // draw shift prediction points (computed in Orthogonality.C)
  TTree * predTr = new TTree();
  Int_t binnum,ampnum;
  Double_t pred,iv,nop;
  TGraphAsymmErrors * predGr;
  TMultiGraph * predMulti;
  TCanvas * predCanv;
  if(predictShift) {
    predCanv = new TCanvas("predGrCanv","predGrCanv",4*300,nrow*300);
    predCanv->Divide(4,nrow);
    predTr->ReadFile("prediction.dat","binnum/I:ampnum/I:pred/D");
    predTr->SetBranchAddress("binnum",&binnum);
    predTr->SetBranchAddress("ampnum",&ampnum);
    predTr->SetBranchAddress("pred",&pred);
    for(int i=0; i<nAmp; i++) {
      pad = i+1;
      if(pad>=8) pad = i+2; // (to re-align multidim plots)
      predGr = new TGraphAsymmErrors();
      predGr->SetMarkerStyle(41);
      predGr->SetMarkerColor(kViolet+1);
      predGr->SetMarkerSize(1.5);
      for(int j=0; j<predTr->GetEntries(); j++) {
        predTr->GetEntry(j);
        if(ampnum==i) {
          ((TGraphAsymmErrors*)(grArr[0]->At(i)))->GetPoint(binnum,iv,nop);
          predGr->SetPoint(binnum,iv,pred);
        };
      };
      predMulti = new TMultiGraph();
      diffGr = (TGraphAsymmErrors*) diffGrArr->At(i);
      predMulti->SetTitle(diffGr->GetTitle());
      predMulti->Add(diffGr);
      predMulti->Add(predGr);
      predCanv->cd(pad);
      predCanv->GetPad(pad)->SetGrid(1,1);
      predMulti->GetYaxis()->SetRangeUser(-diffMax,diffMax);
      predMulti->Draw("APE");
    };
  };

};
