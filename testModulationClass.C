// test new Modulation class

R__LOAD_LIBRARY(DihBsa)
#include "Tools.h"
#include "Modulation.h"

void testModu(Int_t tw, Int_t l, Int_t m, Int_t lev, Bool_t pw, Int_t pol) {
  TString titleStr = Form("|%d,%d>  twist=%d  level=%d",l,m,tw,lev);
  Tools::PrintTitleBox(titleStr);
  Modulation * modu = new Modulation(tw,l,m,lev,pw,pol);
  printf(" %s\n",(modu->StateTitle()).Data());
  printf(" base:     %s\n",(modu->GetBaseString()).Data());
  printf(" formuRF:  %s\n",(modu->FormuRF()).Data());
  printf(" formu:    %s\n",(modu->Formu()).Data());
  printf("\n");
};

void testModulationClass() {

  Int_t polarization = Modulation::kUT;
  Bool_t enablePW = false;
  Int_t twist = 3;
  Int_t level = 1;

  for(int L=0; L<=2; L++) {
    Tools::PrintSeparator(50);
    printf("L=%d\n",L);
    for(int M=-L; M<=L; M++) {
      testModu(twist,L,M,level,enablePW,polarization);
    };
  };
};
