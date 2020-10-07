// FitDelta
// - takes a grid of MC asymmetry injection results, one result for a value
//   of amplitudes (A,B), and plots the difference "delta" in the fit result
//   between (A,B) and (A,0)
// - aim to parameterize and understand the impact on LU amplitudes ("A") from
//   nonzero UU amplitudes ("B")


// class to hold information from each MC injection result
// - each new instance of Injection should have a unique index `idx`,
//   corresponding to the index in `helicityMC` from `injectHelicityMC.cpp`
// - delta is calculated from two of these, linked by the `idxComp` variable
TObjArray * Grid; // store all pointers to Injection objects
class Injection : public TObject {
  public: 
    int idx;
    float A,B;
    int idxComp;
    Injection(int idx_, float A_, float B_) {
      idx = idx_;
      A = A_;
      B = B_;
      idxComp = -1;
    };
    TString PrintVars() {
      return Form("(%d,%.2f,%.2f)",idx,A,B);
    };
};

// return the Injection pointer for a given index
Injection * FindInjection(int idx_) {
  TObjArrayIter nxt(Grid);
  while(Injection * in = (Injection*) nxt()) { if(in->idx==idx_) return in; };
  return NULL;
};



//=============================================================//


void FitDelta(Int_t amp=0, Int_t binnum=0) {
  Grid = new TObjArray();

  // generate TObjArray of `Injection` objects, to store injection index,
  // A, B, etc.
  // - this table is obtained from injectHelicityMC.cpp
  int ii=27;
  for(float AA=-0.21; AA<=0.21; AA+=0.03) {
    for(float BB=-0.5; BB<=0.5; BB+=0.1) {
      Grid->AddLast(new Injection(ii,AA,BB));
      ii++;
    };
  };


  // assign `idxComp` to each Injection
  // - compare injection J with (A_J,B_J) to 
  //           injection I with (A_I=A_J,B_I=0)
  // of the injection to which we want to compare for calculating delta
  // - for a given (A,B) from injection 
  TObjArrayIter nextInj(Grid);
  TObjArrayIter nextInjTmp(Grid);
  enum injEnum {I,J};
  Injection * inj[2];
  while((inj[I] = (Injection*) nextInj())) {
    if(fabs(inj[I]->B)<1e-5) {
      while((inj[J] = (Injection*) nextInjTmp())) {
        if(inj[J]->A==inj[I]->A) inj[J]->idxComp = inj[I]->idx;
      };
      nextInjTmp.Reset();
    };
  };
  nextInj.Reset();


  // - calculate deltas (I minus J)
  //   - injection I with (A_I=A_J,B_I=0)
  //   - injection J with (A_J,B_J)
  TString infileN[2];
  TFile * infile[2];
  TGraphAsymmErrors * asymGr[2];
  TGraph2DErrors * deltaGr = new TGraph2DErrors();
  Int_t deltaGrCnt=0;
  Double_t x[2];
  Double_t y[2];
  Double_t ex[2];
  Double_t ey[2];
  Double_t xx,exx,yy,eyy;
  TString asymGrN = Form("kindepMA_A%d_M",amp);
  while((inj[J] = (Injection*) nextInj())) {
    inj[I] = FindInjection(inj[J]->idxComp);
    cout << "Compare " << inj[I]->PrintVars() << 
             " minus " << inj[J]->PrintVars() << endl;
    for(int k=0; k<2; k++) {
      infileN[k] = Form("spinroot_inj_%d/asym_42.root",inj[k]->idx);
      infile[k] = new TFile(infileN[k],"READ");
      asymGr[k] = (TGraphAsymmErrors*) infile[k]->Get(asymGrN);
      asymGr[k]->GetPoint(binnum,x[k],y[k]);
      ex[k] = asymGr[k]->GetErrorX(binnum);
      ey[k] = asymGr[k]->GetErrorY(binnum);
    };
    xx = x[I];
    yy = y[I]-y[J];
    exx = 0;
    eyy = TMath::Sqrt(TMath::Abs(ey[I]*ey[I]-ey[J]*ey[J]));
    deltaGr->SetPoint(deltaGrCnt,inj[J]->A,inj[J]->B,yy);
    deltaGr->SetPointError(deltaGrCnt,0,0,eyy);
    deltaGrCnt++;
  };
  nextInj.Reset();

  // draw delta(A,B)
  new TCanvas();
  deltaGr->SetTitle("#delta(A,B);A;B;#delta");
  deltaGr->SetMarkerColor(kBlack);
  deltaGr->SetMarkerStyle(kFullCircle);
  deltaGr->Draw("APE");
  deltaGr->Fit("x*y*[0]/(1+y*[0])","","");
};
