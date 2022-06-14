R__LOAD_LIBRARY(DiSpin)

void drawEleTheta(TString infiles, TString outFileN, Int_t whichPair=0x34) {

  TFile *outFile = new TFile(outFileN,"RECREATE");
  EventTree *ev = new EventTree(infiles,whichPair);
  TH1F *eleThetaDist = new TH1F("eleThetaDist","electron #theta [deg]",200,0,40);

  // event loop
  for(Long64_t i=0; i<ev->ENT; i++) {
    ev->GetEvent(i);
    if(ev->Valid()) {
      eleThetaDist->Fill(ev->eleTheta);
    };
  };

  outFile->cd();
  eleThetaDist->Write();
  outFile->Close();

};
