R__LOAD_LIBRARY(DiSpin)
#include "BruAsymmetry.h"

void asymBruFit(TString bruDir="bruspin", TString minimizer="mcmc",
  Int_t binschemeIVtype=2,
  Int_t nbins0=-1, Int_t nbins1=-1, Int_t nbins2=-1
) {

  // load macros needed for PROOF
  TString BRUCODE=gSystem->Getenv("BRUFIT");
  TString macpath=BRUCODE+"/macros";
  gROOT->SetMacroPath(
    Form("%s:%s",gROOT->GetMacroPath(),(macpath).Data()));

  // instantiate brufit
  BruAsymmetry * B = new BruAsymmetry(bruDir);

  // build modulations
  /* // 7 amps (PRL)
  B->AddNumerMod(new Modulation(3,0,0));
  B->AddNumerMod(new Modulation(2,1,1));
  B->AddNumerMod(new Modulation(3,1,1));
  B->AddNumerMod(new Modulation(3,1,-1));
  B->AddNumerMod(new Modulation(2,2,2));
  B->AddNumerMod(new Modulation(3,2,2));
  B->AddNumerMod(new Modulation(3,2,-2));
  */
  ///* // all 12 PWs up to L=2 (dnp2020)
  B->AddNumerMod(new Modulation(3,0,0,0,true));
  B->AddNumerMod(new Modulation(3,1,0,0,true));
  B->AddNumerMod(new Modulation(2,1,1,0,true));
  B->AddNumerMod(new Modulation(3,1,1,0,true));
  B->AddNumerMod(new Modulation(3,1,-1,0,true));
  B->AddNumerMod(new Modulation(3,2,0,0,true));
  B->AddNumerMod(new Modulation(2,2,1,0,true));
  B->AddNumerMod(new Modulation(3,2,1,0,true));
  B->AddNumerMod(new Modulation(3,2,-1,0,true));
  B->AddNumerMod(new Modulation(2,2,2,0,true));
  B->AddNumerMod(new Modulation(3,2,2,0,true));
  B->AddNumerMod(new Modulation(3,2,-2,0,true));
  //*/
  /* // DSIDIS
  B->AddNumerMod(new Modulation(2,0,0,0,false,Modulation::kDSIDIS)); // sin(PhiD)
  B->AddNumerMod(new Modulation(2,0,0,1,false,Modulation::kDSIDIS)); // sin(2*PhiD)
  */
  /* // denominators
  B->AddDenomMod(new Modulation(2,1,1,0,false,Modulation::kUU)); // cos(phiH-phiR)
  B->AddDenomMod(new Modulation(3,0,0,0,false,Modulation::kUU)); // cos(phiH)
  B->AddDenomMod(new Modulation(3,1,1,0,false,Modulation::kUU)); // cos(phiR)
  */

  // build full PDF
  B->BuildPDF();

  // set binning scheme
  Binning * BS = new Binning();
  BS->SetScheme(binschemeIVtype,nbins0,nbins1,nbins2);
  B->Bin(BS);
  B->PrintBinScheme();

  // load data and MC trees
  //B->LoadDataSets("catTreeData.root"); // disable MC integration
  B->LoadDataSets("catTreeData.root","catTreeMC.spinAbsent.PRL.root"); // enable MC integration // DIS
  //B->LoadDataSets("spinroot.proton.pion/catTreeData.root","catTreeMC.proton.pion.root");

  // MCMC settings
  B->MCMC_iter = 30000; // number of samples
  B->MCMC_burnin = ((Double_t)B->MCMC_iter)/10.0; // number of initial samples to drop
  B->MCMC_norm = 1.0 / 0.03; // 1/stepsize

  // perform fit
  B->Fit(minimizer);

  // draw
  TString cmd = Form(".x drawBru.C(\"%s\",\"%s\")",bruDir.Data(),minimizer.Data());
  gROOT->ProcessLine(cmd);
};
