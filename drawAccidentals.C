void drawAccidentals(TString infileN="accidentals/skim4_005052.hipo.root") {
  TFile * infile = new TFile(infileN,"READ");
  TTree * ditr = (TTree*) infile->Get("ditr");
  TH2D * betaVsP = new TH2D("betaVsP","#beta vs. p;p;#beta",
    100,0,11,100,0,3);

  Float_t pid[2];
  Float_t beta[2];
  Float_t px[2];
  Float_t py[2];
  Float_t pz[2];

  TString hadStr;
  for(int h=0; h<2; h++) {
    hadStr = h==0 ? "hadA" : "hadB";
    ditr->SetBranchAddress(TString(hadStr+"_Pid"),&pid[h]);
    ditr->SetBranchAddress(TString(hadStr+"_beta"),&beta[h]);
    ditr->SetBranchAddress(TString(hadStr+"_Px"),&px[h]);
    ditr->SetBranchAddress(TString(hadStr+"_Py"),&py[h]);
    ditr->SetBranchAddress(TString(hadStr+"_Pz"),&pz[h]);
  };

  bool proceed;
  for(Long64_t i=0; i<ditr->GetEntries(); i++) {
    ditr->GetEntry(i);
    for(int h=0; h<2; h++) {

      proceed = false;
      if(pid[h]>0) proceed=true; // +charge
      //if(pid[h]<0) proceed=true; // -charge

      if(!proceed) continue;

      betaVsP->Fill(
        TMath::Sqrt(px[h]*px[h]+py[h]*py[h]+pz[h]*pz[h]),
        beta[h]
      );
    };
  };

  new TCanvas();
  betaVsP->Draw("colz");
};
      
