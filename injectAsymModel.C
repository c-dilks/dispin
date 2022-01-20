// create an injection model for asymmetry amplitudes
R__LOAD_LIBRARY(DiSpin)

void injectAsymModel(
    int numInjections = 100,
    TString outFileN = "injection.root"
    )
{

  // OPTIONS //////////////////////
  // define binning scheme in order to store IV names; the number
  // of bins is irrelevant, just use single-bins
  const Int_t ivType = 32;
  // approximate scale of asymmetry amplitude to inject; check `maxAdist`,
  // etc. which are histograms filled with min and max amplitude values to
  // inject; these histos give a sense of the injected amplitude range
  const Double_t asymMax = 0.12;
  // other
  bool verbose = 0;
  /////////////////////////////////

  // output file
  TFile *outFile = new TFile(outFileN,"RECREATE");
  TH1D *maxAdist = new TH1D("maxAdist","max amp dist",200,-1,1);
  TH1D *minAdist = new TH1D("minAdist","min amp dist",200,-1,1);
  TH2D *maxVsMin = new TH2D("maxVsMin","max amp vs min amp;min amp;max amp",200,-1,1,200,-1,1);

  // injection model
  InjectionModel *IM = new InjectionModel();
  IM->FillModuList(0);
  IM->SetIVtype(ivType);
  Binning *BS = IM->GetBinning();

  // RNG; fix seed for reproducibility
  auto RNG = new TRandomMixMax(50314);

  Double_t s,a0;
  TString formu;
  TString xyz[3] = {"x","y","z"};
  TF1 *F;
  for(int i=0; i<numInjections; i++) { // injection loop
    if(verbose) printf("injection %d formulas:\n",i);
    for(auto modu : IM->GetModuList()) { // modulation loop

      // random a0, the value of the asymmetry at IV==0 (for each dimension)
      a0 = RNG->Uniform(-asymMax,asymMax);
      formu = Form("%f",a0);

      for(int d=0; d<BS->dimensions; d++) { // IV loop

        // random slope for this dimension, and append to formu
        s = RNG->Uniform(-asymMax,asymMax);
        formu += Form("+%f*%s",s,xyz[d].Data());

        // divide by IV range, to keep amplitude in desired |asymMax| range
        formu += Form("/%.1f",BS->maxIV[BS->ivVar[d]]-BS->minIV[BS->ivVar[d]]);
      }
      Tools::GlobalRegexp(formu,TRegexp("\\+-"),"-");

      // create amplitude model TF1 and insert into IM
      // - TF1 ranges are used later by GetMin/Maximum methods
      TString FN = modu->AmpName() + Form("__%d",i);
      if(verbose) printf("- %s\t%s\n",FN.Data(),formu.Data());
      switch(BS->dimensions) {
        case 1: 
          F = new TF1(
              FN,formu,
              BS->minIV[BS->ivVar[0]], BS->maxIV[BS->ivVar[0]]
              );
          break;
        case 2: 
          F = new TF2(
              FN,formu,
              BS->minIV[BS->ivVar[0]], BS->maxIV[BS->ivVar[0]],
              BS->minIV[BS->ivVar[1]], BS->maxIV[BS->ivVar[1]]
              );
          break;
        case 3: 
          F = new TF3(
              FN,formu,
              BS->minIV[BS->ivVar[0]], BS->maxIV[BS->ivVar[0]],
              BS->minIV[BS->ivVar[1]], BS->maxIV[BS->ivVar[1]],
              BS->minIV[BS->ivVar[2]], BS->maxIV[BS->ivVar[2]]
              );
          break;
      }
      IM->AddAmplitudeModel(modu,F);

      // fill min/maxA dists
      minAdist->Fill(F->GetMinimum());
      maxAdist->Fill(F->GetMaximum());
      maxVsMin->Fill(F->GetMinimum(),F->GetMaximum());
    }
  }

  // write output
  IM->WriteOut();
  maxAdist->Write();
  minAdist->Write();
  maxVsMin->Write();
  outFile->Close();
  printf("produced %s\n",outFileN.Data());
}
