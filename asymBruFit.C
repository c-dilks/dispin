R__LOAD_LIBRARY(DiSpin)
#include "BruAsymmetry.h"

// IMPORTANT: run with `brufit -b -q asymBruFit.C`
/* - minimizer: optimizer algorithm; see src/Constants.h MinimizerStrToEnum
 *              for available minimizer strings
 */

void asymBruFit(
    TString bruDir="bruspin", // output directory; several files will be written
    TString minimizer="minuit", // minimizer (see comments above)
    TString sPlotDir="", // sPlot directory (leave empty string if not using, otherwise use outDir from `sPlotBru.C`)
    Int_t binschemeIVtype=2, // binning scheme (execute `buildSpinroot.exe` and see usage of `-i`)
    Int_t nbins0=-1, Int_t nbins1=-1, Int_t nbins2=-1, // number of bins for each dimension
           /* example: binschemeIVtype=32, nbins0=6, nbins1=3, runs fit in 6 bins of z (iv=3) for 3 bins of Mh (iv=2) */
    Int_t whichSpinMC=-1 // if >=0, use helicity from injected asymmetry (branch "SpinMC_`whichSpinMC`_idx")
) {

  // set PROOF sandbox (where log files etc. are written)
  TString sandbox = TString(gSystem->Getenv("PWD")) + "/" + bruDir + "/prooflog";
  gEnv->SetValue("ProofLite.Sandbox",sandbox.Data());
  printf("proof sandbox = %s\n",gEnv->GetValue("ProofLite.Sandbox","ERROR"));

  // instantiate brufit
  BruAsymmetry * B = new BruAsymmetry(bruDir,minimizer,whichSpinMC);

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
  ///* // all 12 PWs up to L=2
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
  //B->LoadDataSets("catTreeData.rga_inbending_all.0x34.idx.root","catTreeMC.mc.PRL.0x34.idx.root"); // RGA
  //B->LoadDataSets("catTreeData.rgb_inbending_all.0x34.idx.root","catTreeMC.mc.PRL.0x34.idx.root"); // RGB
  /* pi+, pi+ */
  //B->LoadDataSets("catTreeData.0x33.XFgt0.idx.root","catTreeMC.0x33.spinAbsent.XFgt0.idx.root"); // pi+pi+
  /* pi+, p */
  //B->LoadDataSets("spinroot.proton.pion/catTreeData.idx.root","catTreeMC.proton.pion.idx.root");
  /* pi+, pi0 */
  //B->LoadDataSets("catTreeData.rga_inbending_all.0x35.idx.root","catTreeMC.mc.PRL.0x35.idx.root"); // pi0 sig window
  //B->LoadDataSets("catTreeData.rga_inbending_all.0x3c.idx.root","catTreeMC.mc.PRL.0x3c.idx.root"); // pi0 bg window
  /* pi+, pi0 sFit
   * - NOTE: use `TrimCatTree.C` and `sPlotBru.C` to produce the required files
   */
  // B->LoadDataSets(
  //     "catTreeData.rga_inbending_all.0x3b.idx.trimmed.root",
  //     "catTreeMC.mc.PRL.0x3b.idx.trimmed.root",
  //     sPlotDir+"/Tweights.root",
  //     "Signal"
  //     );
  /* MC injection tests */
  // B->LoadDataSets("catTreeMC.mc.PRL.DIS.0x34.inj_x.idx.root",""); // MC: 1D linear injection along x
  B->LoadDataSets("catTreeMC.mc.PRL.DIS.0x34.inj_zm.idx.root",""); // MC: 2D linear injection along {z,Mh}

  // MCMC hyperparameters
  // - chain 1
  B->MCMC_iter   = 2000; // number of samples
  B->MCMC_burnin = 0.1 * ((Double_t)B->MCMC_iter); // number to burn
  B->MCMC_norm   = 1.0 / 0.015; // ~ 1/stepsize
  // - chain 2 (for minimizer=="mcmccov")
  B->MCMC_cov_iter   = 15000; // number of samples
  B->MCMC_cov_burnin = 0.1 * ((Double_t)B->MCMC_iter); // number to burn
  B->MCMC_cov_norm   = 1.0 / 0.03; // ~ 1/stepSize
  // - acceptance rate locks
  B->MCMC_lockacc_target = 0.234; // standard "optimal" acceptance rate
  B->MCMC_lockacc_min    = B->MCMC_lockacc_target - 0.02;
  B->MCMC_lockacc_max    = B->MCMC_lockacc_target + 0.02;

  // perform fit
  B->Fit();

  // print acceptance rates
  TString cmd;
  gSystem->RedirectOutput(B->GetLogName());
  cmd = Form(".! mcmcAcceptanceRate.rb %s",bruDir.Data());
  gROOT->ProcessLine(cmd.Data());
  gSystem->RedirectOutput(0);

  // print PROOF errors
  gSystem->RedirectOutput(B->GetLogName());
  cmd = Form(".! errorPrintProof.rb %s",bruDir.Data());
  gROOT->ProcessLine(cmd.Data());
  gSystem->RedirectOutput(0);

  // draw residuals and pulls for each bin
  for(int d=0; d<BS->dimensions; d++) {
    cmd = Form(".x DrawResiduals.C(\"%s\",\"%s\",\"%s\")",bruDir.Data(),BS->GetIVname(d).Data(),minimizer.Data());
    printf("\nEXECUTE: %s\n\n",cmd.Data());
    gROOT->ProcessLine(cmd);
  };

  // draw asymmetries
  cmd = Form(".x drawBru.C(\"%s\",\"%s\")",bruDir.Data(),minimizer.Data());
  printf("\nEXECUTE: %s\n\n",cmd.Data());
  gROOT->ProcessLine(cmd);
};
