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
    TFile * infile;
    Double_t x,y,ex,ey;
    TGraphAsymmErrors * asymGr;
    // - constructor
    Injection(int idx_, float A_, float B_) {
      idx = idx_;
      A = A_;
      B = B_;
      idxComp = -1;
      TString infileN = Form("spinroot_inj_%d/asym_42.root",idx);
      infile = new TFile(infileN,"READ");
    };
    // - get asymmetry for given amplitude and bin number
    void GetAsym(Int_t amp_,Int_t binnum_) {
      TString asymGrN = Form("kindepMA_A%d_M",amp_);
      asymGr = (TGraphAsymmErrors*) infile->Get(asymGrN);
      asymGr->GetPoint(binnum_,x,y);
      ex = asymGr->GetErrorX(binnum_);
      ey = asymGr->GetErrorY(binnum_);
    };
    // - printout
    TString PrintVars() { return Form("(%d,%.2f,%.2f)",idx,A,B); };
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
  TGraph2DErrors * deltaGr = new TGraph2DErrors();
  Int_t deltaGrCnt=0;
  Double_t iv,ivErr,delta,deltaErr;
  while((inj[J] = (Injection*) nextInj())) {
    inj[I] = FindInjection(inj[J]->idxComp);
    cout << "Compare " << inj[I]->PrintVars() << 
             " minus " << inj[J]->PrintVars() << endl;

    // get asymmetry values from I and J
    for(int k=0; k<2; k++) inj[k]->GetAsym(amp,binnum);

    // compute delta = I-J
    iv = inj[I]->x;
    delta = inj[I]->y - inj[J]->y;
    ivErr = 0;
    deltaErr = TMath::Sqrt(TMath::Abs(
      inj[I]->ey*inj[I]->ey - inj[J]->ey*inj[J]->ey
    ));

    // set deltaGr point
    deltaGr->SetPoint(deltaGrCnt,inj[J]->A,inj[J]->B,delta);
    deltaGr->SetPointError(deltaGrCnt,0,0,deltaErr);
    deltaGrCnt++;

  };
  nextInj.Reset();

  // draw delta(A,B) and fit
  new TCanvas();
  deltaGr->SetTitle("#delta(A,B);A;B;#delta");
  deltaGr->SetMarkerColor(kBlack);
  deltaGr->SetMarkerStyle(kFullCircle);
  deltaGr->Draw("P");
  TF2 * func = new TF2("func","x*y*[0]/(1+y*[0])",-1,1,-1,1);
  //TF2 * func = new TF2("func","x*y*[0]/(1+y*[1])",-1,1,-1,1);
  deltaGr->Fit(func,"","");
  func->Draw("surfsame");
  cout << "chi2/ndf = " << func->GetChisquare()/func->GetNDF() << endl;
};
