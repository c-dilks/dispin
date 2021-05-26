// apply BG correction

class AsymGr : public TObject {
  public:
    TString name;
    TGraphErrors * gr;

    // -constructor
    AsymGr(TGraphErrors * gr_) {
      gr = gr_;
      name = gr->GetName();
    };
};

void Corrector(
  TGraphErrors *g1, TGraphErrors *g2, TGraphErrors *corr);

//..................................................//
//..................................................//
//..................................................//


void bgCorrector(
  /*
  TString bruFileSig="bruspin.sig/asym.root",
  TString bruFileBG="bruspin.bg/asym.root"
  */
  TString bruFileSig="bruspin.XFtest.XFfull.mh/asym_minuit_BL0.root",
  TString bruFileBG="bruspin.XFtest.XFgt0.mh/asym_minuit_BL0.root"
) {

  // open asym.root files
  enum fileEnum {sg,bg};
  int f;
  TString bruFileN[2] = {bruFileSig,bruFileBG};
  TFile *bruFile[2];
  for(f=0; f<2; f++) bruFile[f] = new TFile(bruFileN[f],"READ");


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
  AsymGr * agr[2];
  Bool_t found;
  Int_t idx[2];

  // loop through `sg` list, looking for matches in `bg` list
  idx[sg]=0;
  while((agr[sg] = (AsymGr*) nextAsymGr[sg]())) {
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
  TMultiGraph * mgr;
  TCanvas * compCanv = new TCanvas("compCanv","compCanv",600*nCol,300*nRow);
  compCanv->Divide(nCol,nRow);
  TString gTitle,xTitle,yTitle;
  TLine * zeroLine;
  Double_t xmin,xmax;
  for(int n=0; n<nLinks; n++) {

    // draw comparison
    compCanv->cd(n+1);
    compCanv->GetPad(n+1)->SetGrid(0,1);
    mgr = new TMultiGraph();
    for(int f=0; f<2; f++) {
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
        //if(!(TString(agr[f]->GetName()).Contains("Yld"))) {
          //mgr->SetMinimum(-0.07);
          //mgr->SetMaximum(0.07);
        //};
      };
    };
    mgr->Draw("APE");
    zeroLine = new TLine(xmin,0,xmax,0);
    zeroLine->SetLineColor(kBlack);
    zeroLine->SetLineWidth(2);
    zeroLine->SetLineStyle(kDashed);
    zeroLine->Draw();
    mgr->GetXaxis()->SetLimits(xmin,xmax);
  };

  // note: no need to calculate purity uncertainty, which is likely
  // suppressed when propagated to BG-corrected asym uncertainty

};
