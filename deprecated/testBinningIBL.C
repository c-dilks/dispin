// test Binning::BinNumToIBL
R__LOAD_LIBRARY(DiSpin)
void testBinningIBL() {
  Binning *BS = new Binning();
  Int_t I,BL;

  BS->SetScheme(2,12); // 1D
  BS->SetScheme(42,6,3); // 2D
  //BS->SetScheme(421,6,3,4); // 3D

  for(Int_t bn : BS->binVec) {
    BS->BinNumToIBL(bn,I,BL);
    printf("bn=0x%03x I=%d BL=%d\n",bn,I,BL);
  };
};
