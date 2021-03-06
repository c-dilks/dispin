// prints columns [phiR,phiH,asym,err] from the TGraph2DErrors in asym*.root

void PrintAsymGr(TString varname="M", Int_t binnum=8) {

  TFile * infile = new TFile("spinroot_final_2/asym_42_mlm.root","READ");
  TString plotname = Form("asym_%s%d",varname.Data(),binnum);
  TGraph2DErrors * g = (TGraph2DErrors*) infile->Get(plotname);

  // x=phiR,  y=phiH,  z=asym
  Double_t phiH,phiR,asym,err;
  for(int i=0; i<g->GetN(); i++) {
    g->GetPoint(i,phiR,phiH,asym);
    err = g->GetErrorZ(i);
    //gSystem->RedirectOutput("asymPrint.txt",i==0?"w":"a");
    printf("%.5f %.5f %.5f %.5f\n",phiR,phiH,asym,err);
    //gSystem->RedirectOutput(0);
  };

};
