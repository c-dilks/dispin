// builds ortho.root, which contains yield distributions needed for modulation
// orthogonalty studies (see Orthogonality*.C)


/* 
code which gives a distribution of the bin contents;
use this to check if d3 is binned too finely 
void CountBinContents(TString fN = "ortho.root") {
  TFile * f = new TFile(fN,"READ");
  TH3D * d = (TH3D*) f->Get("d3");
  TH1D * cd = new TH1D("cd","bin content distribution",20,0,20);
  Double_t c;
  for(int x=1; x<d->GetNbinsX(); x++) {
    for(int y=1; y<d->GetNbinsY(); y++) {
      for(int z=1; z<d->GetNbinsZ(); z++) {
        c = d->GetBinContent(x,y,z);
        if(c>0) cd->Fill(c);
        //cd->Fill(c);
      };
    };
  };
  new TCanvas();
  cd->Draw();
};
*/

R__LOAD_LIBRARY(DiSpin)

#include "Constants.h"
#include "EventTree.h"
#include "Tools.h"

void BuildOrtho(TString inDir="outroot") {
  const Int_t NBINS = 50;
  TFile * outfile = new TFile("ortho.root","RECREATE");

  // yield distributions
  TH2D * d2 = new TH2D("d2","d2",NBINS,-PI,PI,NBINS,-PI,PI);
  TH3D * d3 = new TH3D("d3","d3",NBINS,-PI,PI,NBINS,-PI,PI,NBINS,0,PI);

  // test plots
  TH2D * pph[2];
  pph[0] = new TH2D("pphA","#phi_{h}^{#pi1#pi2} vs. #phi_{h}^{#pi1}",NBINS,-PI,PI,
                                                                     NBINS,-PI,PI);
  pph[1] = new TH2D("pphB","#phi_{h}^{#pi1#pi2} vs. #phi_{h}^{#pi2}",NBINS,-PI,PI,
                                                                     NBINS,-PI,PI);

  EventTree * ev = new EventTree(TString(inDir+"/*.root"),EncodePairType(kPip,kPim));
  Float_t pH,pR;
  for(int i=0; i<ev->ENT; i++) {
    ev->GetEvent(i);
    if(ev->Valid()) {
      pH = Tools::AdjAngle(ev->PhiH); // shift to [-pi,+pi]
      pR = Tools::AdjAngle(ev->PhiR); // shift to [-pi,+pi]
      d2->Fill(pR,pH);
      d3->Fill(pR,pH,ev->theta);
      for(int h=0; h<2; h++) {
        pph[h]->Fill(ev->GetDihadronObj()->GetSingleHadronPhiH(h),ev->PhiH);
      };
    };
  };
  d2->Write();
  d3->Write();
  for(int h=0; h<2; h++) pph[h]->Write();
};
