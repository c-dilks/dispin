TH3D * histo;
TH2D * histo2;
TH1D * histo1;
int mXtmp,mYtmp;
TLine *sXline,*sYline;

void SlicePlot(
  TString infileN="plots.root",
  TString histoN="MhVsYHcorr",
  Int_t mode=1 /* 1=1Dto2D, 2=2Dto1D */
) {
  // open root file and histogram
  TFile * infile = new TFile(infileN,"READ");
  histo = (TH3D*) infile->Get(histoN);

  // mouseover canvas
  TCanvas * canvMouse = new TCanvas("canvMouse","canvMouse",800,600);

  sXline = new TLine();
  sYline = new TLine();
  sXline->SetLineWidth(2);
  sXline->SetLineColor(kBlack);
  sYline->SetLineWidth(2);
  sYline->SetLineColor(kBlack);

  if(mode==1) {
    // 1D to 2D
    //histo->Rebin3D(1,1,2);
    histo1 = (TH1D*) histo->ProjectionZ();
    histo1->SetTitle(TString(
      TString(histo->GetZaxis()->GetTitle()) + " distribution"));
    histo1->SetStats(0);
    histo1->SetFillColor(kRed);
    histo1->SetLineColor(kBlack);
    histo1->Draw("B");
    canvMouse->AddExec("slice","DrawSlice1to2()");
  }

  else if(mode==2) {
    // 2D to 1D
    histo->Rebin3D(2,2,1);
    histo2 = (TH2D*) histo->Project3D("yx");
    histo2->SetTitle(TString(
      TString(histo->GetYaxis()->GetTitle()) + " vs. " +
      TString(histo->GetXaxis()->GetTitle()) ));
    canvMouse->AddExec("slice","DrawSlice2to1()");
    histo2->Draw("colz");
  };
}

// ==================================================
// mouseover Execs
// adapted from ROOT tutorial hist/DynamicSlice.C
// ==================================================

void DrawSlice1to2()
{
  TObject * histSelected = gPad->GetSelected();
  /*
  if(!histSelected) return;
  if(!histSelected->InheritsFrom(TH1::Class())) {
    gPad->SetUniqueID(0);
    return;
  }
  TH1 * hist1D = (TH1*) histSelected;
  */
  TH1 * hist1D = (TH1*) histo1;
  gPad->GetCanvas()->FeedbackMode(kTRUE);

  // line drawing
  int mX = gPad->GetEventX();
  float uYmin = gPad->GetUymin();
  float uYmax = gPad->GetUymax();
  int Ymin = gPad->YtoAbsPixel(uYmin);
  int Ymax = gPad->YtoAbsPixel(uYmax);
  if(mXtmp) gVirtualX->DrawLine(mXtmp,Ymin,mXtmp,Ymax);
  gVirtualX->DrawLine(mX,Ymin,mX,Ymax);
  mXtmp = mX;
  Float_t uX = gPad->AbsPixeltoX(mX);
  Float_t X = gPad->PadtoX(uX);


  // define slice canvas
  TVirtualPad * vPad = gPad;
  TCanvas * canvSlice = 
    (TCanvas*) gROOT->GetListOfCanvases()->FindObject("canvSlice");
  if(canvSlice) delete canvSlice->GetPrimitive("Slice");
  else 
    canvSlice = new TCanvas("canvSlice","canvSlice",800,600);
  canvSlice->SetGrid();
  canvSlice->cd();

  // draw slice
  Int_t Xbin = hist1D->GetXaxis()->FindBin(X);
  histo->GetZaxis()->SetRange(Xbin,Xbin);
  TH2D * histSlice = (TH2D*) histo->Project3D("yx");
  histSlice->SetName("Slice");
  TString histSliceT = Form("slice at %s = %.2f",
    hist1D->GetXaxis()->GetTitle(),X);
  histSlice->SetTitle(histSliceT);
  histSlice->Draw("colz");

  // mean crosshair line drawing
  sXline->SetX1(histSlice->GetMean(1));
  sXline->SetX2(histSlice->GetMean(1));
  sXline->SetY1(histSlice->GetYaxis()->GetXmin());
  sXline->SetY2(histSlice->GetYaxis()->GetXmax());
  sXline->Draw();

  sYline->SetX1(histSlice->GetXaxis()->GetXmin());
  sYline->SetX2(histSlice->GetXaxis()->GetXmax());
  sYline->SetY1(histSlice->GetMean(2));
  sYline->SetY2(histSlice->GetMean(2));
  sYline->Draw();
  canvSlice->Update();

  vPad->cd();
}


void DrawSlice2to1()
{
  TObject * histSelected = gPad->GetSelected();
  /*
  if(!histSelected) return;
  if(!histSelected->InheritsFrom(TH1::Class())) {
    gPad->SetUniqueID(0);
    return;
  }
  TH2 * hist2D = (TH2*) histSelected;
  */
  TH2 * hist2D = (TH2*) histo2;
  gPad->GetCanvas()->FeedbackMode(kTRUE);

  // crosshair line drawing
  int mX = gPad->GetEventX();
  int mY = gPad->GetEventY();
  float uXmin = gPad->GetUxmin();
  float uXmax = gPad->GetUxmax();
  float uYmin = gPad->GetUymin();
  float uYmax = gPad->GetUymax();
  int Xmin = gPad->XtoAbsPixel(uXmin);
  int Xmax = gPad->XtoAbsPixel(uXmax);
  int Ymin = gPad->YtoAbsPixel(uYmin);
  int Ymax = gPad->YtoAbsPixel(uYmax);
  if(mYtmp) gVirtualX->DrawLine(Xmin,mYtmp,Xmax,mYtmp);
  if(mXtmp) gVirtualX->DrawLine(mXtmp,Ymin,mXtmp,Ymax);
  gVirtualX->DrawLine(Xmin,mY,Xmax,mY);
  gVirtualX->DrawLine(mX,Ymin,mX,Ymax);
  mYtmp = mY;
  mXtmp = mX;
  Float_t uX = gPad->AbsPixeltoX(mX);
  Float_t X = gPad->PadtoX(uX);
  Float_t uY = gPad->AbsPixeltoY(mY);
  Float_t Y = gPad->PadtoY(uY);

  // define slice canvas
  TVirtualPad * vPad = gPad;
  TCanvas * canvSlice = 
    (TCanvas*) gROOT->GetListOfCanvases()->FindObject("canvSlice");
  if(canvSlice) delete canvSlice->GetPrimitive("Slice");
  else 
    canvSlice = new TCanvas("canvSlice","canvSlice",800,600);
  canvSlice->SetGrid();
  canvSlice->cd();

  // draw slice
  Int_t Xbin = hist2D->GetXaxis()->FindBin(X);
  Int_t Ybin = hist2D->GetYaxis()->FindBin(Y);
  TH1D * histSlice = (TH1D*) histo->ProjectionZ("_pz",Xbin,Xbin,Ybin,Ybin);
  histSlice->SetName("Slice");
  TString histSliceT = Form("slice at %s = %.2f,  %s = %.2f",
    hist2D->GetXaxis()->GetTitle(),X,
    hist2D->GetYaxis()->GetTitle(),Y);
  histSlice->SetTitle(histSliceT);
  histSlice->SetFillColor(kRed);
  histSlice->SetLineColor(kBlack);
  histSlice->Draw();
  canvSlice->Update();
  vPad->cd();
}
