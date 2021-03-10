R__LOAD_LIBRARY(DiSpin)
#include "BruAsymmetry.h"
void asymBruFit(TString bruDir="bruspin", TString minimizer="mcmc") {

  // load macros needed for PROOF
  TString BRUCODE=gSystem->Getenv("BRUFIT");
  TString macpath=BRUCODE+"/macros";
  gROOT->SetMacroPath(
    Form("%s:%s",gROOT->GetMacroPath(),(macpath).Data()));
   
  // instantiate
  BruAsymmetry * B = new BruAsymmetry(bruDir);

  // build modulations
  /*
  B->AddNumerMod(new Modulation(3,0,0)); // 7 amps (PRL)
  B->AddNumerMod(new Modulation(2,1,1));
  B->AddNumerMod(new Modulation(3,1,1));
  B->AddNumerMod(new Modulation(3,1,-1));
  B->AddNumerMod(new Modulation(2,2,2));
  B->AddNumerMod(new Modulation(3,2,2));
  B->AddNumerMod(new Modulation(3,2,-2));
  */
  /*
  B->AddNumerMod(new Modulation(3,0,0,0,true)); // all 12 PWs up to L=2 (dnp2020)
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
  B->AddNumerMod(new Modulation(2,0,0,0,false,Modulation::kDSIDIS)); // sin(PhiD)
  B->AddNumerMod(new Modulation(2,0,0,1,false,Modulation::kDSIDIS)); // sin(2*PhiD)
  */
  ///* // pion-pion, with denominators
  B->AddDenomMod(new Modulation(3,0,0,0,false,Modulation::kUU)); // cos(phiH)
  //B->AddDenomMod(new Modulation(3,1,1,0,false,Modulation::kUU)); // cos(phiR)
  B->AddNumerMod(new Modulation(2,1,1));
  B->AddNumerMod(new Modulation(3,1,1));
  //*/

  // build full PDF
  B->BuildPDF();

  // binning
  //--------------------------------------
  ///* // pion-pion
  const Int_t nBins = 12;
  Double_t MhBins[nBins+1] = {
    0.0,
    0.381,
    0.462,
    0.531,
    0.606,
    0.675,
    0.737,
    0.781,
    0.831,
    0.900,
    0.981,
    1.125,
    3.0
  };
  B->FM->Bins().LoadBinVar("Mh",nBins,MhBins);
  //*/
  /* // proton-pion
  const Int_t nBins = 10;
  Double_t xBins[nBins+1] = {
    0.0,
    0.13,
    0.15,
    0.18,
    0.20,
    0.23,
    0.26,
    0.30,
    0.34,
    0.41,
    1.0
  };
  B->FM->Bins().LoadBinVar("X",nBins,xBins);
  */

  B->PrintBinScheme();
  //--------------------------------------


  // load data and MC trees
  B->LoadDataSets("spinroot/catTree.root","catTreeMC.root");
  //B->LoadDataSets("spinroot.proton.pion/catTree.root","catTreeMC.proton.pion.root");

  // MCMC settings
  B->MCMC_iter = 1000; // number of samples
  B->MCMC_burnin = ((Double_t)B->MCMC_iter)/10.0; // number of initial samples to drop
  B->MCMC_norm = 1.0 / 0.01; // 1/stepsize

  // perform fit
  B->Fit(minimizer);
};
