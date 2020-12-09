TH3D * histo;
TH1D * histo1;

void SlicePlot2(
  TString infileN="plots.root",
  TString histoN="MhVsYHcorr"
) {
  // open root file and histogram
  TFile * infile = new TFile(infileN,"READ");
  histo = (TH3D*) infile->Get(histoN);

  // 1D canvas
  histo1 = (TH1D*) histo->ProjectionZ();
  histo1->SetTitle(TString(
    TString(histo->GetZaxis()->GetTitle()) + " distribution"));
  TCanvas * canvMouse = new TCanvas("canvMouse","canvMouse",800,600);
  histo1->SetStats(0);
  histo1->SetFillColor(kRed);
  histo1->SetLineColor(kBlack);
  histo1->Draw("B");

  // assign DrawSlice() to mouseover events
  canvMouse->AddExec("slice","DrawSlice()");
}

// mouseover Exec
// adapted from ROOT tutorial hist/DynamicSlice.C
void DrawSlice()
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
  int mXold = gPad->GetUniqueID();
  int mX = gPad->GetEventX();
  float uYmin = gPad->GetUymin();
  float uYmax = gPad->GetUymax();
  int Ymin = gPad->YtoAbsPixel(uYmin);
  int Ymax = gPad->YtoAbsPixel(uYmax);
  if(mXold) gVirtualX->DrawLine(mXold,Ymin,mXold,Ymax);
  gVirtualX->DrawLine(mX,Ymin,mX,Ymax);
  gPad->SetUniqueID(mX);
  Float_t uX = gPad->AbsPixeltoX(mX);
  Float_t X = gPad->PadtoX(uX);


  // define slice canvas
  TVirtualPad * vPad = gPad;
  TCanvas * canvSlice = 
    (TCanvas*) gROOT->GetListOfCanvases()->FindObject("canvSlice");
  if(canvSlice) delete canvSlice->GetPrimitive("Projection");
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
  canvSlice->Update();
  vPad->cd();
}
