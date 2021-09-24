// compare asymmetries from two brufit results
// - see `comparator.sh`, a helpful wrapper script


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

void DiffGraph(
  TGraphErrors * g1, TGraphErrors *g2, TGraphErrors * diff);


//..................................................//
//..................................................//
//..................................................//

void CompareBruAsym(
  TString titleBlue="blue fit result",
  TString bruFileNameBlue="bruspin.blue/asym.root",
  TString titleRed="red fit result",
  TString bruFileNameRed="bruspin.red/asym.root"
) {

  // open asym.root files
  enum fileEnum {azul,rojo};
  TString bruFileName[2] = {bruFileNameBlue,bruFileNameRed};
  TFile * bruFile[2];
  for(int f=0; f<2; f++) {
    if(!bruFileName[f].Contains(".root")) {
      bruFileName[f]+="/asym.root"; // legacy syntax
    };
    bruFile[f] = new TFile(bruFileName[f],"READ");
  };

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
  Color_t color[2] = {kCyan-2,kRed+3};
  Style_t style[2] = {kFullTriangleUp,kFullTriangleDown};
  for(int f=0; f<2; f++) {
    while((agr[f] = (AsymGr*) nextAsymGr[f]())) {
      agr[f]->gr->SetMarkerColor(color[f]);
      agr[f]->gr->SetLineColor(color[f]);
      agr[f]->gr->SetMarkerStyle(style[f]);
    };
    nextAsymGr[f].Reset();
  };

  // draw a legend
  TCanvas * canvLegend = new TCanvas("legend","legend",600,200);
  TLegend * legend = new TLegend(0.1,0.1,0.9,0.9);
  legend->AddEntry(
    ((AsymGr*)AsymGrList[azul]->At(0))->gr,titleBlue,"PE");
  legend->AddEntry(
    ((AsymGr*)AsymGrList[rojo]->At(0))->gr,titleRed,"PE");
  legend->Draw();

  // draw graphs
  Int_t nCol = 4;
  Int_t nRow = (nLinks-1)/nCol+1;
  TMultiGraph * mgr;
  TGraphErrors * diffGr;
  TCanvas * compCanv = new TCanvas("compCanv","compCanv",600*nCol,300*nRow);
  TCanvas * diffCanv = new TCanvas("diffCanv","diffCanv",600*nCol,300*nRow);
  compCanv->Divide(nCol,nRow);
  diffCanv->Divide(nCol,nRow);
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

    // draw difference
    if(links[n][azul]>=0 && links[n][rojo]>=0) {
      diffCanv->cd(n+1);
      diffCanv->GetPad(n+1)->SetGrid(0,1);
      for(int f=0; f<2; f++)
        agr[f] = (AsymGr*) AsymGrList[f]->At(links[n][f]);
      diffGr = new TGraphErrors();
      DiffGraph(agr[azul]->gr,agr[rojo]->gr,diffGr);
      diffGr->Draw("APE");
      zeroLine = new TLine(
        diffGr->GetXaxis()->GetXmin(),0,diffGr->GetXaxis()->GetXmax(),0);
      zeroLine->SetLineColor(kBlack);
      zeroLine->SetLineWidth(2);
      zeroLine->SetLineStyle(kDashed);
      zeroLine->Draw();
    };
  };

  // print images
  canvLegend->Print("compare_asym_legend.png");
  compCanv->Print("compare_asym_amps.png");
  diffCanv->Print("compare_asym_diffs.png");
};


//..................................................//
//..................................................//
//..................................................//

void DiffGraph(
  TGraphErrors * g1, TGraphErrors *g2, TGraphErrors * diff) {

  if(g1->GetN()!=g2->GetN()) {
    fprintf(stderr,"ERROR: number of bins differ\n");
    return;
  };

  // format difference graph
  TString gT,xT,yT,gN;
  gN = g1->GetName();
  gT = g1->GetTitle();
  xT = g1->GetXaxis()->GetTitle();
  yT = g1->GetYaxis()->GetTitle();
  diff->SetName("diff_"+gN);
  diff->SetTitle(gT);
  diff->GetXaxis()->SetTitle(xT);
  diff->GetYaxis()->SetTitle(yT);
  diff->SetMarkerStyle(kFullCircle);
  diff->SetMarkerColor(kRed);
  diff->SetLineColor(kRed);

  // calculate amount to shift (offset) g2 to the right
  Double_t bump =
    TMath::Abs(g2->GetXaxis()->GetXmax() - g2->GetXaxis()->GetXmin()) /
    ( g2->GetN() * 8 );
  bump = 0; // <--- DISABLE bump

  // compute difference and correlated error (assumes datasets
  // are equal, or at least one is a subset of the other)
  Double_t x1,x2, y1,y2, ey1,ey2;
  Double_t ydiff,eydiff;
  for(int i=0; i<g1->GetN(); i++) {
    g1->GetPoint(i,x1,y1);
    g2->GetPoint(i,x2,y2);
    ey1 = g1->GetErrorY(i);
    ey2 = g2->GetErrorY(i);
    ydiff = y1-y2; // difference
    int whichError = 2;
    switch(whichError) {
      case 1: 
        cout << "ASSUMING COMPLETELY UNCORRELATED ERRORS (datasets are disjoint)" << endl;
        eydiff = TMath::Sqrt(ey1*ey1+ey2*ey2); // uncorrelated error
        break;
      case 2: 
        cout << "ASSUMING COMPLETELY CORRELATED ERRORS (one set is subset of other)" << endl;
        eydiff = TMath::Sqrt(TMath::Abs(ey1*ey1-ey2*ey2)); // correlated error
        break;
      default:
        cerr << "ERROR: unknown whichError" << endl;
        eydiff = 0;
    };
    diff->SetPoint(i,x1,ydiff);
    diff->SetPointError(i,0,eydiff);
    g2->SetPoint(i,x2+bump,y2); // bump g2 point to the right
  };
};




