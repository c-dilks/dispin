// compare asymmetries from two brufit results


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



//..................................................//
//..................................................//
//..................................................//

void CompareBruAsym(
  /*
  TString titleBlue="PRL amps",
  TString bruDirBlue="bruspin.prl__init_0__ss_3",
  TString titleRed="PRL amps + 3 UU amps",
  TString bruDirRed="bruspin.denom.10amp"
  */
  ///*
  TString titleBlue="PW amps",
  TString bruDirBlue="bruspin.dnp2020.long__ss_1__burn_1000",
  TString titleRed="PW amps + DSIDIS",
  TString bruDirRed="bruspin.dsidis.long__ss_1__burn_1000"
  //*/
  /*
  TString titleBlue="PW amps + DSIDIS",
  TString bruDirBlue="bruspin.dsidis.long__ss_1__burn_1000",
  TString titleRed="PW amps",
  TString bruDirRed="bruspin.dnp2020.long__ss_1__burn_1000"
  */
) {

  // open asym.root files
  enum fileEnum {azul,rojo};
  TFile * bruFile[2];
  bruFile[azul] = new TFile(bruDirBlue+"/asym.root","READ");
  bruFile[rojo] = new TFile(bruDirRed+"/asym.root","READ");

  // build lists of asymmetry graphs
  TObjArray * AsymGrList[2];
  TString keyname;
  for(int f=0; f<2; f++) {
    AsymGrList[f] = new TObjArray();
    TListIter nextKey(bruFile[f]->GetListOfKeys());
    while(TKey * key = (TKey*) nextKey()) {
      keyname = TString(key->GetName());
      if(keyname.Contains(TRegexp("^gr_"))) {
        AsymGrList[f]->AddLast(
          new AsymGr((TGraphErrors*)key->ReadObj()));
      };
    };
  };


  //....................................

  // links array: `links` is a 2D array, seen as a list of
  // pairs of index values, where the index is of the two
  // AsymGrLists; later we will loop through `links` and plot
  // the pairs of plots correpsonding to the index pair
  const int nmax = 256;
  Int_t links[nmax][2];
  int nLinks=0;

  // define iterators and indices
  TObjArrayIter nextAsymGr[2] = {
    TObjArrayIter(AsymGrList[azul]),
    TObjArrayIter(AsymGrList[rojo])
  };
  AsymGr * agr[2];
  Bool_t found;
  Int_t idx[2];

  // loop through blue list, looking for matches in red list
  idx[azul]=0;
  while((agr[azul] = (AsymGr*) nextAsymGr[azul]())) {
    found=false;
    idx[rojo]=0;
    while((agr[rojo] = (AsymGr*) nextAsymGr[rojo]())) {
      // TODO: might need to regexp names from old files...
      if(agr[rojo]->name==agr[azul]->name) {
        // match was found:
        for(int f=0; f<2; f++) links[nLinks][f] = idx[f];
        nLinks++;
        found=true;
      };
      idx[rojo]++;
    };
    nextAsymGr[rojo].Reset();
    idx[rojo]=0;
    if(!found) {
      // match was not found:
      links[nLinks][azul] = idx[azul];
      links[nLinks][rojo] = -1;
      nLinks++;
    };
    idx[azul]++;
  };
  nextAsymGr[azul].Reset(); nextAsymGr[rojo].Reset();

  // vice versa loop: loop through red list, looking for
  // anything that has no match in the blue list, that the
  // previous loop missed
  idx[rojo]=0;
  while((agr[rojo] = (AsymGr*) nextAsymGr[rojo]())) {
    found=false;
    idx[azul]=0;
    while((agr[azul] = (AsymGr*) nextAsymGr[azul]())) {
      // TODO: might need to regexp names from old files...
      if(agr[rojo]->name==agr[azul]->name) {
        // match was found (but already listed in `links`)
        found=true;
      };
    };
    nextAsymGr[azul].Reset();
    if(!found) {
      // match was not found:
      links[nLinks][azul] = -1;
      links[nLinks][rojo] = idx[rojo];
      nLinks++;
    };
    idx[rojo]++;
  };
  nextAsymGr[azul].Reset(); nextAsymGr[rojo].Reset();
  idx[azul]=0; idx[rojo]=0;

  // print links
  for(int n=0; n<nLinks; n++) {
    printf("blue %d  <->  red %d\n",
      links[n][azul],links[n][rojo]);
  };

  //....................................

  
  // format graphs
  gStyle->SetTitleSize(0.08,"T");
  gStyle->SetLabelSize(0.08,"X");
  gStyle->SetLabelSize(0.08,"Y");
  Color_t color[2] = {kRed+3,kCyan-2};
  Style_t style[2] = {kFullTriangleUp,kFullTriangleDown};
  for(int f=0; f<2; f++) {
    while((agr[f] = (AsymGr*) nextAsymGr[f]())) {
      agr[f]->gr->SetMarkerColor(color[f]);
      agr[f]->gr->SetLineColor(color[f]);
      agr[f]->gr->SetMarkerStyle(style[f]);
    };
    nextAsymGr[f].Reset();
  };

  // initialize canvas
  Int_t nCol = 4;
  Int_t nRow = (nLinks-1)/nCol+1;
  TMultiGraph * mgr;
  TCanvas * canv = new TCanvas("canv","canv",600*nCol,300*nRow);
  canv->Divide(nCol,nRow);
  TString gTitle,xTitle,yTitle;
  for(int n=0; n<nLinks; n++) {
    canv->cd(n+1);
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
      };
    };
    mgr->Draw("APE");
  };
};
