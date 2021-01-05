// draw results from `drawAccidentals.cpp`, and estimate fraction of
// accidentals; be sure to specify the `species` number from
// `drawAccidentals.cpp`

void drawAccidentalsPlots(TString infileN="acc.root", Int_t species=1) {
  TFile * infile = new TFile(infileN,"READ");
  TString distN;
  switch(species) {
    case 1: distN="betaVsP1"; break;
    case 2: distN="betaVsP0"; break;
    case 3: distN="betaVsBeta"; break;
    default: fprintf(stderr,"fail\n"); return;
  };
  TH2D * dist = (TH2D*) infile->Get(distN);
  
  TH1D * dist1 = dist->ProjectionY();
  TCanvas * canv;
  gStyle->SetOptStat(0);

  Int_t a1,a2,b1,b2,c1,c2,d1,d2;
  Double_t numer,denom,frac;
  if(species==1 || species==2) {

    a1 = dist1->FindBin(0.98); // main bucket
    a2 = dist1->FindBin(1.02);
    denom = dist1->Integral(a1,a2);

    b1 = dist1->FindBin(1.14); // subsequent bucket
    b2 = dist1->FindBin(1.23);
    numer = dist1->Integral(b1,b2);

    canv = new TCanvas("canv","canv",2000,1000);
    canv->Divide(2,1);
    canv->cd(1);
    canv->GetPad(1)->SetLogz();
    dist->Draw("COLZ");
    dist->GetYaxis()->SetRangeUser(0.8,1.4);
    canv->cd(2);
    canv->GetPad(2)->SetLogy();
    dist1->Draw();
    dist1->GetXaxis()->SetRangeUser(0.8,1.4);

  } else {

    a1 = dist->GetXaxis()->FindBin(0.98); // main bucket
    a2 = dist->GetXaxis()->FindBin(1.02);
    b1 = dist->GetYaxis()->FindBin(0.98);
    b2 = dist->GetYaxis()->FindBin(1.02);
    denom = dist->Integral(a1,a2,b1,b2);

    c1 = dist->GetXaxis()->FindBin(1.14); // subsequent bucket
    c2 = dist->GetXaxis()->FindBin(1.23);
    d1 = dist->GetYaxis()->FindBin(1.14);
    d2 = dist->GetYaxis()->FindBin(1.23);
    numer = dist->Integral(c1,c2,d1,d2);

    canv = new TCanvas("canv","canv",1000,1000);
    canv->SetLogz();
    dist->Draw("COLZ");
    dist->GetXaxis()->SetRangeUser(0.8,1.4);
    dist->GetYaxis()->SetRangeUser(0.8,1.4);
  };

  frac = numer / denom;
  printf("accidentals fraction: %f / %f = %f\n",numer,denom,frac);
};
