R__LOAD_LIBRARY(DiSpin)
void testBinning() {

  Binning * BS = new Binning();
  Tools::PrintSeparator(50,"o");

  /*
  BS->SetScheme(2,2);
  BS->SetScheme(42);
  BS->SetScheme(421,6,4,12);
  BS->SetScheme(2,20);
  BS->SetScheme(1,1);
  */
  BS->SetScheme(32);

  Tools::PrintSeparator(50,"o");

  printf("dump TArrayD Binning::binArray\n");
  for(int d=0; d<BS->dimensions; d++) {
    printf("d=%d\n",d);
    printf("size=%d\n",BS->GetBinArray(d)->GetSize());
    for(int b=0; b<BS->GetNbins(d)+1; b++) {
      printf(" b=%d val=%f\n",b,BS->GetBinArray(d)->GetArray()[b]);
    };
  };

  Tools::PrintSeparator(50,"o");

  BruAsymmetry * bru = new BruAsymmetry("tmpdir");
  bru->Bin(BS);
  bru->PrintBinScheme();
  gROOT->ProcessLine(".! rm -r tmpdir");


};
