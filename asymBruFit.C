R__LOAD_LIBRARY(DiSpin)
#include "BruAsymmetry.h"

// IMPORTANT: run with `brufit -b -q asymBruFit.C`
/* - minimizer: optimizer algorithm; see src/Constants.h MinimizerStrToEnum
 *              for available minimizer strings
 */

void asymBruFit(
    TString dataTree="catTreeData.rga.bibending.all.idx.root", // data catTree
    TString mcTree="catTreeMC.mc.bibending.all.idx.root", // MC catTree (leave empty to disable)
    TString bruDir="bruspin.weightTest.rga", // output directory
    TString minimizer="minuit", // minimizer (see comments above)
    TString weightDir="catTreeWeights/catTreeData.rga.bibending.all;catTreeWeights/catTreeMC.mc.bibending.all", // weights directory or directories (viz. sWeights, via outDir from `sPlotBru.C`); leave empty string if not using; use a semicolon and a second directory to specify MC weights for the MC usage in the PDF normalization approximation
    TString binschemeVersion="PM", // binning scheme version; see `Binning::SetSchemeVersion`
    Int_t binschemeIVtype=2, // binning scheme (execute `buildSpinroot.exe` and see usage of `-i`)
    Int_t nbins0=6,  // number of bins for each dimension
    Int_t nbins1=-1, // - example: binschemeIVtype=32, nbins0=6, nbins1=3, runs fit in 6 bins of z (iv=3) for 3 bins of Mh (iv=2)
    Int_t nbins2=-1, // - leave `-1` to use defaults defined in `src/Binning.cxx`
    Int_t whichSpinMC=-1 // if >=0, use helicity from injected asymmetry (branch "SpinMC_`whichSpinMC`_idx")
) {

  // set PROOF sandbox (where log files etc. are written)
  // - you may need to create or symlink this directory (`pwd`/farmout)
  TString sandbox = TString(gSystem->Getenv("PWD")) + "/farmout/" + bruDir + "/prooflog";
  gEnv->SetValue("ProofLite.Sandbox",sandbox.Data());
  printf("proof sandbox = %s\n",gEnv->GetValue("ProofLite.Sandbox","ERROR"));

  // instantiate brufit
  BruAsymmetry * B = new BruAsymmetry(bruDir,minimizer,whichSpinMC);


  // set binning scheme ------------------------------------------------------------------------------------
  Binning * BS = new Binning();
  BS->SetSchemeVersion(binschemeVersion);
  BS->SetScheme(binschemeIVtype,nbins0,nbins1,nbins2);
  B->Bin(BS);
  B->PrintBinScheme();


  // build modulations -----------------------------------------------------------------------------
  /* // 7 amps (PRL)
  B->AddNumerMod(new Modulation(3,0,0));
  B->AddNumerMod(new Modulation(2,1,1));
  B->AddNumerMod(new Modulation(3,1,1));
  B->AddNumerMod(new Modulation(3,1,-1));
  B->AddNumerMod(new Modulation(2,2,2));
  B->AddNumerMod(new Modulation(3,2,2));
  B->AddNumerMod(new Modulation(3,2,-2));
  */
  ///* // all 12 PWs up to L=Lmax
  const Int_t Lmax = 2;
  for(int L=0; L<=Lmax; L++) {
    for(int M=0; M<=L; M++) {
      for(int T=2; T<=3; T++) {
        if(T==2 && M==0) continue;
        B->AddNumerMod(new Modulation(T,L,M,0,true));
        if(T==3 && M>0) B->AddNumerMod(new Modulation(T,L,-M,0,true));
      };
    };
  };
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


  // load data and MC catTrees -------------------------------------------------------------------------------
  if(weightDir=="") B->LoadDataSets( dataTree, mcTree  );
  else {
    // load weights
    TObjArray *weightDirTokens = weightDir.Tokenize(";");
    //// data weights
    TString weightDir_data = ((TObjString*)weightDirTokens->At(0))->GetString(); // before ';' delimiter (if applicable)
    TString weightClass = weightDir_data.Contains("splot") ? // weight class name
      "Signal" :   // signal sWeights
      "IO";        // inbending/outbending weights for bibending
    B->LoadDataSets( dataTree, mcTree, weightDir_data+"/Tweights.root", weightClass );
    //// MC weights (for PDF normalizer)
    if(weightDirTokens->GetEntries()>1) {
      TString weightDir_mc = ((TObjString*)weightDirTokens->At(1))->GetString(); // after ';' delimiter
      B->LoadPDFweights( weightDir_mc+"/Tweights.root", weightClass, "HSsWeights" );
    };
  };


  // build full PDF ----------------------------------------------------------------------------------------
  B->BuildPDF();


  // MCMC hyperparameters ------------------------------------------------------------------------------------
  // - chain 1
  B->MCMC_iter   = 3000; // number of samples
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


  // perform fit -----------------------------------------------------------------------------------------------
  B->Fit();

  // print acceptance rates
  TString cmd;
  cmd = Form(".! ./mcmcAcceptanceRate.rb farmout/%s",bruDir.Data());
  printf("\nEXECUTE: %s\n\n",cmd.Data());
  gSystem->RedirectOutput(B->GetLogName());
  gROOT->ProcessLine(cmd.Data());
  gSystem->RedirectOutput(0);

  // print PROOF errors
  cmd = Form(".! ./errorPrintProof.rb farmout/%s",bruDir.Data());
  printf("\nEXECUTE: %s\n\n",cmd.Data());
  gSystem->RedirectOutput(B->GetLogName());
  gROOT->ProcessLine(cmd.Data());
  gSystem->RedirectOutput(0);

  // draw residuals and pulls for each bin
  for(int d=0; d<BS->dimensions; d++) {
    cmd = Form(".! ./brufit -b -q 'DrawResiduals.C(\"%s\",\"%s\",\"%s\")'",bruDir.Data(),BS->GetIVname(d).Data(),minimizer.Data());
    printf("\nEXECUTE: %s\n\n",cmd.Data());
    gROOT->ProcessLine(cmd);
  };

  // draw asymmetries
  cmd = Form(".! ./brufit -b -q 'drawBru.C(\"%s\",\"%s\")'",bruDir.Data(),minimizer.Data());
  printf("\nEXECUTE: %s\n\n",cmd.Data());
  gROOT->ProcessLine(cmd);
};
