// apply BG correction
R__LOAD_LIBRARY(DiSpin)

// global vars
enum fileEnum {sg,bg};
TFile *fitFile;
Binning *BS;
TTree *purTr;
Int_t binnum_tr;
Double_t purity_tr,purityErr_tr,iv_tr;
Int_t bl;
Bool_t mismatchBins;

//..................................................//

class AsymGr : public TObject {
  public:
    TString name;
    TGraphErrors *gr;

    // -constructor
    AsymGr(TGraphErrors *gr_) {
      gr = gr_;
      name = gr->GetName();
    };
};

//..................................................//

void Corrector(
  TGraphErrors *gsg, TGraphErrors *gbg, TGraphErrors *corr);

//..................................................//
//..................................................//
//..................................................//


void bgCorrector(
  /*
  TString bruFileSig="bruspin.sig/asym.root",
  TString bruFileBG="bruspin.bg/asym.root",
  */
  TString bruFileSig="bruspin.XFtest.XFfull.mh/asym_minuit_BL0.root",
  TString bruFileBG="bruspin.XFtest.XFgt0.mh/asym_minuit_BL0.root",
  TString fitFileN="diagdiph.rga_inbending_sp19/fit__outroot.rga_inbending_sp19__1__0x3b.root"
) {

  // open asym.root files
  int f;
  TString bruFileN[2] = {bruFileSig,bruFileBG};
  TFile *bruFile[2];
  for(f=0; f<2; f++) bruFile[f] = new TFile(bruFileN[f],"READ");

  // get binlist (BL) number by parsing file names
  TString blStr[2];
  Int_t blTmp[2];
  for(f=0; f<2; f++) {
    blStr[f] = bruFileN[f];
    blStr[f](TRegexp("^.*_")) = "";
    blStr[f](TRegexp("\\.root$")) = "";
    sscanf(blStr[f].Data(),"BL%d",&blTmp[f]);
  };
  if(blTmp[sg]!=blTmp[bg]) {
    fprintf(stderr,"ERROR: BL number differs\n");
    return;
  };
  bl = blTmp[sg];

  // open fitFile
  fitFile = new TFile(fitFileN,"READ");
  BS = (Binning*) fitFile->Get("BS");
  purTr = (TTree*) fitFile->Get("purTr");
  purTr->SetBranchAddress("binnum",&binnum_tr);
  purTr->SetBranchAddress("purity",&purity_tr);
  purTr->SetBranchAddress("purityErr",&purityErr_tr);
  purTr->SetBranchAddress("iv",&iv_tr);


  // build lists of asymmetry graphs
  TObjArray *AsymGrList[2];
  TString keyname;
  for(f=0; f<2; f++) {
    AsymGrList[f] = new TObjArray();
    TListIter nextKey(bruFile[f]->GetListOfKeys());
    while(TKey *key = (TKey*) nextKey()) {
      keyname = TString(key->GetName());
      if(keyname.Contains(TRegexp("^gr_"))) {
        AsymGrList[f]->AddLast(
          new AsymGr((TGraphErrors*)key->ReadObj()));
      };
    };
  };


  // links array: `links` is a 2D array, seen as a list of
  // pairs of index values, where the index is of the two
  // AsymGrLists; later we will loop through `links` and plot
  // the pairs of plots correpsonding to the index pair
  const int nmax = 256;
  Int_t links[nmax][2];
  int nLinks=0;

  // define iterators and indices
  TObjArrayIter nextAsymGr[2] = {
    TObjArrayIter(AsymGrList[sg]),
    TObjArrayIter(AsymGrList[bg])
  };
  AsymGr *agr[2];
  Bool_t found;
  Int_t idx[2];

  // loop through `sg` list, looking for matches in `bg` list
  idx[sg]=0;
  while((agr[sg] = (AsymGr*) nextAsymGr[sg]())) {
    if(TString(agr[sg]->gr->GetName()).Contains("Yld")) continue;
    found=false;
    idx[bg]=0;
    while((agr[bg] = (AsymGr*) nextAsymGr[bg]())) {
      if(agr[bg]->name==agr[sg]->name) {
        // match was found:
        for(f=0; f<2; f++) links[nLinks][f] = idx[f];
        nLinks++;
        found=true;
      };
      idx[bg]++;
    };
    nextAsymGr[bg].Reset();
    idx[bg]=0;
    if(!found) {
      // match was not found:
      links[nLinks][sg] = idx[sg];
      links[nLinks][bg] = -1;
      nLinks++;
    };
    idx[sg]++;
  };
  nextAsymGr[sg].Reset(); nextAsymGr[bg].Reset();

  // vice versa loop: loop through `bg` list, looking for
  // anything that has no match in the `sg` list, that the
  // previous loop missed
  idx[bg]=0;
  while((agr[bg] = (AsymGr*) nextAsymGr[bg]())) {
    if(TString(agr[bg]->gr->GetName()).Contains("Yld")) continue;
    found=false;
    idx[sg]=0;
    while((agr[sg] = (AsymGr*) nextAsymGr[sg]())) {
      if(agr[bg]->name==agr[sg]->name) {
        // match was found (but already listed in `links`)
        found=true;
      };
    };
    nextAsymGr[sg].Reset();
    if(!found) {
      // match was not found:
      links[nLinks][sg] = -1;
      links[nLinks][bg] = idx[bg];
      nLinks++;
    };
    idx[bg]++;
  };
  nextAsymGr[sg].Reset(); nextAsymGr[bg].Reset();
  idx[sg]=0; idx[bg]=0;

  // print links
  for(int n=0; n<nLinks; n++) {
    printf("signal %d  <->  background %d\n",
      links[n][sg],links[n][bg]);
  };

  // check for mismatching amplitudes; files must have the same
  // amplitudes in order to perform BG correction
  for(int n=0; n<nLinks; n++) {
    if(links[n][sg]<0 || links[n][bg]<0) {
      cerr << "ERROR: files contain different amplitudes" << endl;
      return;
    };
  };


  // format graphs
  gStyle->SetTitleSize(0.08,"T");
  gStyle->SetLabelSize(0.08,"X");
  gStyle->SetLabelSize(0.08,"Y");
  Color_t color[2] = {kGreen+1,kBlue};
  Style_t style[2] = {kFullTriangleUp,kFullTriangleDown};
  for(f=0; f<2; f++) {
    while((agr[f] = (AsymGr*) nextAsymGr[f]())) {
      agr[f]->gr->SetMarkerColor(color[f]);
      agr[f]->gr->SetLineColor(color[f]);
      agr[f]->gr->SetMarkerStyle(style[f]);
    };
    nextAsymGr[f].Reset();
  };


  // draw graphs
  Int_t nCol = 4;
  Int_t nRow = (nLinks-1)/nCol+1;
  TMultiGraph *mgr;
  TGraphErrors *corrGr;
  TCanvas *compCanv = new TCanvas("compCanv","compCanv",600*nCol,300*nRow);
  TCanvas *corrCanv = new TCanvas("corrCanv","corrCanv",600*nCol,300*nRow);
  compCanv->Divide(nCol,nRow);
  corrCanv->Divide(nCol,nRow);
  TString gTitle,xTitle,yTitle;
  TLine *zeroLine;
  Double_t xmin,xmax;
  mismatchBins = false;
  for(int n=0; n<nLinks; n++) {

    // draw comparison
    compCanv->cd(n+1);
    compCanv->GetPad(n+1)->SetGrid(0,1);
    mgr = new TMultiGraph();
    for(f=0; f<2; f++) {
      if(links[n][f]>=0) {
        agr[f] = (AsymGr*) AsymGrList[f]->At(links[n][f]);
        gTitle = agr[f]->gr->GetTitle();
        xTitle = agr[f]->gr->GetXaxis()->GetTitle();
        yTitle = agr[f]->gr->GetYaxis()->GetTitle();
        mgr->SetTitle(gTitle);
        mgr->GetXaxis()->SetTitle(xTitle);
        mgr->GetYaxis()->SetTitle(yTitle);
        mgr->Add(agr[f]->gr);
        xmin = agr[f]->gr->GetXaxis()->GetXmin();
        xmax = agr[f]->gr->GetXaxis()->GetXmax();
      };
    };
    mgr->Draw("APE");
    zeroLine = new TLine(xmin,0,xmax,0);
    zeroLine->SetLineColor(kBlack);
    zeroLine->SetLineWidth(2);
    zeroLine->SetLineStyle(kDashed);
    zeroLine->Draw();
    mgr->GetXaxis()->SetLimits(xmin,xmax);


    // perform BG correction
    if(links[n][sg]>=0 && links[n][bg]>=0) {
      corrCanv->cd(n+1);
      corrCanv->GetPad(n+1)->SetGrid(0,1);
      for(f=0; f<2; f++)
        agr[f] = (AsymGr*) AsymGrList[f]->At(links[n][f]);
      corrGr = new TGraphErrors();
      Corrector(agr[sg]->gr,agr[bg]->gr,corrGr);
      corrGr->Draw("APE");
      zeroLine = new TLine(
        corrGr->GetXaxis()->GetXmin(),0,corrGr->GetXaxis()->GetXmax(),0);
      zeroLine->SetLineColor(kBlack);
      zeroLine->SetLineWidth(2);
      zeroLine->SetLineStyle(kDashed);
      zeroLine->Draw();
    };
  };

  if(mismatchBins) {
    fprintf(stderr,"\n\nWARNING WARNING WARNING: diphoton fit file binning may not match\n");
    fprintf(stderr,"                         that of asymmetry graphs !!!!!\n");
    fprintf(stderr,"                         ... or maybe pairType differs between the files\n");
    fprintf(stderr,"\n\n");

  };

  // cleanup
  for(f=0; f<2; f++) bruFile[f]->Close();
  fitFile->Close();
};

//..................................................//

void Corrector(
  TGraphErrors *gsg, TGraphErrors *gbg, TGraphErrors *corr) {

  // check for equal binning
  TGraphErrors *g[2] = {gsg, gbg};
  if(g[sg]->GetN()!=g[bg]->GetN()) {
    fprintf(stderr,"ERROR: number of bins differ\n");
    return;
  };

  // format corr graph
  TString gT,xT,yT,gN;
  gN = g[sg]->GetName();
  gT = g[sg]->GetTitle();
  xT = g[sg]->GetXaxis()->GetTitle();
  yT = g[sg]->GetYaxis()->GetTitle();
  corr->SetName("corr_"+gN);
  corr->SetTitle(gT);
  corr->GetXaxis()->SetTitle(xT);
  corr->GetYaxis()->SetTitle(yT);
  corr->SetMarkerStyle(kFullCircle);
  corr->SetMarkerColor(kRed);
  corr->SetLineColor(kRed);

  // calculate amount to shift g[bg] to the right
  Double_t bump =
    TMath::Abs(g[bg]->GetXaxis()->GetXmax() - g[bg]->GetXaxis()->GetXmin()) /
    ( g[bg]->GetN() * 8 );
  bump = 0; // disable g[bg] shift


  // bg correction
  cout << "BG correction for " << g[sg]->GetName() << endl;
  Double_t x[2];
  Double_t y[2];
  Double_t ex[2];
  Double_t ey[2];
  Double_t xcorr,ycorr,excorr,eycorr;
  Double_t supp,term1,term2,term3;
  Double_t purity,purityErr;
  Int_t i_tr,bl_tr;
  for(int i=0; i<g[sg]->GetN(); i++) {

    // get asymmetry and IV values
    for(int t=0; t<2; t++) {
      g[t]->GetPoint(i,x[t],y[t]);
      ex[t] = g[t]->GetErrorX(i);
      ey[t] = g[t]->GetErrorY(i);
    };

    // get purity: loop through purTr and find the matching bin's purity
    purity=-1; purityErr=0;
    for(int e=0; e<purTr->GetEntries(); e++) {
      purTr->GetEntry(e);
      BS->BinNumToIBL(binnum_tr,i_tr,bl_tr);
      if(i_tr==i && bl_tr==bl) {
        purity = purity_tr;
        purityErr = purityErr_tr;
        if(TMath::Abs(iv_tr-x[sg])>0.001) {
          fprintf(stderr,"\nERROR: mismatch between graph IV and purTr IV\n");
          fprintf(stderr," binnum=%d i=%d bl=%d",binnum_tr,i,bl);
          fprintf(stderr," graphIV=%f purTrIV=%f\n\n",iv_tr,x[sg]);
          mismatchBins = true;
          //return;
        };
        break;
      };
    };
    if(purity<0) {
      fprintf(stderr,"ERROR: cannot find corresponding bin in purTr\n");
      return;
    };

    // BG CORRECTION //////////////////////////////////////
    /// - correct values
    xcorr = (1/purity)*x[sg] - ((1-purity)/purity)*x[bg];
    ycorr = (1/purity)*y[sg] - ((1-purity)/purity)*y[bg];
    /// - correct IV errors
    excorr = ex[sg]; // (no correction, since should be systematic)
    /// - correct asym errors
    supp = (y[bg]-y[sg])/(purity*purity);
    term1 = TMath::Power( (1/purity)*ey[sg], 2);
    term2 = TMath::Power( ((1-purity)/purity)*ey[bg], 2);
    term3 = TMath::Power( supp*purityErr, 2);
    eycorr = TMath::Sqrt( term1 + term2 + term3 );
    /// - print some numbers
    cout << " i=" << i
         << " purity=" << purity
         << " purityErr=" << purityErr
         << endl
         << " supp=" << supp
         << " term1=" << term1
         << " term2=" << term2
         << " term3=" << term3
         << endl
         << " eysg=" << ey[sg]
         << " eybg=" << ey[bg]
         << " eycorr=" << eycorr
         << endl << endl;
    ///////////////////////////////////////////////////////

    corr->SetPoint(i,xcorr,ycorr);
    corr->SetPointError(i,excorr,eycorr);
    g[bg]->SetPoint(i,x[bg]+bump,y[bg]); // bump g[bg] point to the right

  };
};

