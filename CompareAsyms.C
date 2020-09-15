// compare asymmetries from two different asym*.root files

void CompareAsyms(TString infile0name="spinroot_final_4/asym_42_chi2.root",
                  TString infile1name="spinroot_final_4/asym_42_mlm.root") {

  // read asymmetry graphs
  TFile * infile[2];
  infile[0] = new TFile(infile0name,"READ");
  infile[1] = new TFile(infile1name,"READ");

  TListIter nextKey(infile[0]->GetListOfKeys());
  TString keyname;
  TObjArray * asymArr = new TObjArray();
  TGraphAsymmErrors * gr[2];

  TH1D * diffDist = new TH1D("diffDist","difference distribution",
    100,-0.03,0.03);

  Double_t x[2];
  Double_t y[2];
  Double_t ex[2];
  Double_t ey[2];
  Double_t xx,yy,exx,eyy;

  while(TKey * key = (TKey*) nextKey()) {
    keyname = TString(key->GetName());
    // read asymmetry graph
    if(keyname.Contains(TRegexp("^kindepMA")) &&
           !keyname.Contains("Canv")) {
      gr[0] = (TGraphAsymmErrors*) key->ReadObj();
      gr[1] = (TGraphAsymmErrors*) infile[1]->Get(keyname);
      
      for(int i=0; i<gr[0]->GetN(); i++) {
        for(int g=0; g<2; g++) {
          gr[g]->GetPoint(i,x[g],y[g]);
          ex[g] = gr[g]->GetErrorX(i); // (parabolic error)
          ey[g] = gr[g]->GetErrorY(i); // (parabolic error)
        };

        xx = x[0];
        yy = y[0]-y[1];
        exx = ex[0];

        diffDist->Fill(yy);

        // correlated error (assumes dataset in infile[0] is a subset or
        // equal to the dataset in infile[1], or vice versa)
        eyy = TMath::Sqrt(TMath::Abs(ey[0]*ey[0]-ey[1]*ey[1]));
        gr[0]->SetPoint(i,xx,yy);
        gr[0]->SetPointError(i,exx,exx,eyy,eyy);
      };
      asymArr->AddLast(gr[0]);
    };
  };


  TObjArrayIter nextGr(asymArr);
  TString grT;
  Int_t nrow = 1+(asymArr->GetEntries()-1)/4;
  TCanvas * canv = new TCanvas("canv","canv",4*300,nrow*300);
  canv->Divide(4,nrow);
  Int_t p=0;
  while(TGraphAsymmErrors * gr = (TGraphAsymmErrors*) nextGr()) {
    canv->cd(++p);
    if(p==8) { canv->cd(++p); }; // hack to re-align multidim plots
    canv->GetPad(p)->SetGrid(1,1);
    grT = gr->GetTitle();
    grT = "difference for " + grT;
    gr->GetYaxis()->SetRangeUser(-0.03,0.03);
    gr->SetTitle(grT);
    gr->SetMarkerColor(kBlack);
    gr->SetLineColor(kBlack);
    gr->Draw("APE");
    //gr->GetYaxis()->SetRangeUser(-1e-3,1e-3);
  };

  TCanvas * canv2 = new TCanvas("canv2","canv2",800,800);
  diffDist->SetFillColor(kBlack);
  diffDist->Draw();
};
