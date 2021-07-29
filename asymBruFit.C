R__LOAD_LIBRARY(DiSpin)
#include "BruAsymmetry.h"

/* arguments
 * - bruDir: name of output directory; several files will be written
 * - minimizer: optimizer algorithm, choose one of the following:
 *   - minuit: maximum likelihood using Minuit MIGRAD
 *   - mcmc: sample posterior using Metropolis-Hastings walk with
 *     gaussian proposal; set hyperparameters at the end of this macro
 *   - mcmcthencov: first run mcmc as above, then use the resulting
 *     covariance matrix in the gaussian proposal function of a second,
 *     subsequent mcmc walk
 * - binschemeIVtype: execute `buildSpinroot.exe` with no arguments, and
 *   see the usage for the `-i` flag; up to 3 dimension are supported
 * - nbins0,1,2: number of bins in each dimension
 *   - example: binschemeIVtype=32, nbins0=6, nbins1=3, runs fit in
 *     6 bins of z (iv=3) for 3 bins of Mh (iv=2)
 */

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
  /* pi+, pi- */
  //B->LoadDataSets("catTreeData.rga_inbending_all.0x34.root","catTreeMC.mc.PRL.0x34.root"); // DIS
  /* pi+, pi+ */
  //B->LoadDataSets("catTreeData.0x33.XFgt0.root","catTreeMC.0x33.spinAbsent.XFgt0.root"); // pi+pi+
  /* pi+, p */
  //B->LoadDataSets("spinroot.proton.pion/catTreeData.root","catTreeMC.proton.pion.root");
  /* pi+, pi0 */
  B->LoadDataSets("catTreeData.rga_inbending_all.0x35.root","catTreeMC.mc.PRL.0x35.root"); // pi0 sig window
  //B->LoadDataSets("catTreeData.rga_inbending_all.0x3c.root","catTreeMC.mc.PRL.0x3c.root"); // pi0 bg window

  // hyperparameters
  // - MCMC settings
  B->MCMC_iter = 1000; // number of samples
  B->MCMC_burnin = ((Double_t)B->MCMC_iter)/10.0; // number of initial samples to drop
  B->MCMC_norm = 1.0 / 0.03; // ~ 1/stepsize
  // - 2nd MCMC settings (if using MCMCthenCov algorithm)
  B->MCMC_cov_iter = 1000; // number of samples
  B->MCMC_cov_burnin = ((Double_t)B->MCMC_iter)/10.0; // number of initial samples to drop
  B->MCMC_cov_norm = 1.0 / 0.03; // ~ 1/stepSize

  // perform fit
  B->Fit(minimizer);

  // draw
  TString cmd = Form(".x drawBru.C(\"%s\",\"%s\")",bruDir.Data(),minimizer.Data());
  printf("\nEXECUTE: %s\n\n",cmd.Data());
  gROOT->ProcessLine(cmd);
};
