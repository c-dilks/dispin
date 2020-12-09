void SlicePlot(
  TString infileN="plots.root",
  TString histoN="YHvsMh_piPlus"
) {
  // open root file and histogram
  TFile * infile = new TFile(infileN,"READ");
  TH2D * histo = (TH2D*) infile->Get(histoN);

  // 2D canvas
  TCanvas * canvMouse = new TCanvas("canvMouse","canvMouse",800,600);
  histo->SetStats(0);
  histo->Draw("colz");

  // assign DrawSlice() to mouseover events
  canvMouse->AddExec("slice","DrawSlice()");
}

// mouseover Exec
// adapted from ROOT tutorial hist/DynamicSlice.C
void DrawSlice()
{
  TObject * histSelected = gPad->GetSelected();
  if(!histSelected) return;
  if(!histSelected->InheritsFrom(TH2::Class())) {
    gPad->SetUniqueID(0);
    return;
  }
  TH2 * hist2D = (TH2*) histSelected;
  gPad->GetCanvas()->FeedbackMode(kTRUE);

  // line drawing
  int mYold = gPad->GetUniqueID();
  int mY = gPad->GetEventY();
  float uXmin = gPad->GetUxmin();
  float uXmax = gPad->GetUxmax();
  int Xmin = gPad->XtoAbsPixel(uXmin);
  int Xmax = gPad->XtoAbsPixel(uXmax);
  if(mYold) gVirtualX->DrawLine(Xmin,mYold,Xmax,mYold);
  gVirtualX->DrawLine(Xmin,mY,Xmax,mY);
  gPad->SetUniqueID(mY);
  Float_t uY = gPad->AbsPixeltoY(mY);
  Float_t Y = gPad->PadtoY(uY);

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
  Int_t Ybin = hist2D->GetYaxis()->FindBin(Y);
  TH1D * histSlice = hist2D->ProjectionX("",Ybin,Ybin);
  histSlice->SetFillColor(kRed);
  histSlice->SetName("Slice");
  TString histSliceT = Form("slice at %s = %.2f",
    hist2D->GetYaxis()->GetTitle(),Y);
  histSlice->SetTitle(histSliceT);
  histSlice->Draw();
  canvSlice->Update();
  vPad->cd();
}
