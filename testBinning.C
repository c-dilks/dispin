R__LOAD_LIBRARY(DiSpin)
#include "Binning.h"
#include "Tools.h"
void testBinning() {

  Binning * BS = new Binning();
  Tools::PrintSeparator(50,"o");
  BS->SetScheme(32);
  Tools::PrintSeparator(50,"o");
  BS->SetScheme(42);
  Tools::PrintSeparator(50,"o");
  BS->SetScheme(421,6,4,12);
  Tools::PrintSeparator(50,"o");
  BS->SetScheme(2,20);
  Tools::PrintSeparator(50,"o");
  BS->SetScheme(1,1);

};
