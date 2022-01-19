#include "BruAsymmetry.h"

ClassImp(BruAsymmetry)

BruAsymmetry::BruAsymmetry(TString outdir_, TString minimizer_, Int_t whichSpinMC_)
  : outdir(outdir_)
  , whichSpinMC(whichSpinMC_)
{
  // get minimizer enum
  minimizer = MinimizerStrToEnum(minimizer_);
  if(minimizer<0) return;

  // start FitManager and output logs
  printf("construct BruAsymmetry\n");
  FM = new HS::FIT::FitManager();
  outdir = outdir_;
  FM->SetUp().SetOutDir(outdir); // calls mkdir automatically
  outlog = outdir+"/out."+minimizer_+".log";
  gSystem->RedirectOutput(outlog,"w");
  gSystem->RedirectOutput(0);


  // load PDF variables
  FM->SetUp().LoadVariable(TString("PhiH")+Form("[%f,%f]",-PI,PI));
  FM->SetUp().LoadVariable(TString("PhiR")+Form("[%f,%f]",-PI,PI));
  FM->SetUp().LoadVariable(TString("PhiD")+Form("[%f,%f]",-PI,PI));
  FM->SetUp().LoadVariable(TString("Theta")+Form("[%f,%f]",0.0,PI));
  FM->SetUp().LoadVariable(TString("Pol")+Form("[%f,%f]",-1.0,1.0));
  FM->SetUp().LoadVariable(TString("Depol2")+Form("[%f,%f]",0.0,2.5));
  FM->SetUp().LoadVariable(TString("Depol3")+Form("[%f,%f]",0.0,2.5));

  // load independent variables (IVs) 
  /* - we can load them here now, although some will also be loaded by the binning scheme later
   * - useful if we want *all* IVs stored in binned trees, but will cause warnings about
   *   certain variables being ignored
   * - doing this is optional, if we don't, then binning scheme will load the ones we need
   */
  if(1) { // enable/disable switch
    // definitions kept consistent with Binning.cxx, but doesn't really have to be
    FM->SetUp().LoadVariable( TString("Mh")+Form("[%f,%f]",     0.0, 2.0  ));
    FM->SetUp().LoadVariable( TString("X")+Form("[%f,%f]",      0.0, 1.0  ));
    FM->SetUp().LoadVariable( TString("Z")+Form("[%f,%f]",      0.0, 1.0  ));
    FM->SetUp().LoadVariable( TString("PhPerp")+Form("[%f,%f]", 0.0, 3.0  ));
    FM->SetUp().LoadVariable( TString("DY")+Form("[%f,%f]",     0.0, 4.0  ));
    FM->SetUp().LoadVariable( TString("Q2")+Form("[%f,%f]",     0.0, 12.0 ));
    FM->SetUp().LoadVariable( TString("XF")+Form("[%f,%f]",     0.0, 1.0  ));
  };

  // category for spin
  spinBranch = whichSpinMC<0 ? "Spin_idx" : Form("SpinMC_%d_idx",whichSpinMC);
  FM->SetUp().LoadCategory(spinBranch+"[SpinP=1,SpinM=-1,SpinOff=0]");
  
  // unique ID variable
  FM->SetUp().SetIDBranchName("Idx");


  // default MCMC hyperparameters
  MCMC_iter   = 1000;
  MCMC_burnin = 200;
  MCMC_norm   = 200;
  MCMC_cov_iter   = 1000;
  MCMC_cov_burnin = 200;
  MCMC_cov_norm   = 200;
  MCMC_lockacc_min    = -1; // leave this set to -1 to disable locks; if both min and max are >=0, enable locks
  MCMC_lockacc_max    = -1;
  MCMC_lockacc_target = 0.234; // standard "optimal" acceptance rate


  // misc vars
  numerList = "";
  numerFormu = "";
  denomFormu = "";
  ampNameList = "";
  formuNameList = "";
  nDenomParams = 0;

  printf("constructed BruAsymmetry\n");

};


// add numerator modulation to PDF
void BruAsymmetry::AddNumerMod(Modulation * modu) {

  // set amplitude and modulation names
  TString ampName = modu->AmpName();
  TString formuName = ampName;
  formuName.ReplaceAll("Amp","Mod");

  // amplitude parameter
  FM->SetUp().LoadParameter(ampName+"[0.0,-1,1]");

  // determine which depolarization factor to use
  // - assumes LU, or DSIDIS twist 2
  TString depolVar;
  switch(modu->GetTw()) {
    case 2: depolVar = "@Depol2[]"; break;
    case 3: depolVar = "@Depol3[]"; break;
    default: 
      fprintf(stderr,"unknown depolarization factor; setting to 1\n");
      depolVar = "1";
  };

  TString polVar = "@Pol[]";
  TString spinVar = "@"+spinBranch+"[]";

  // modulation, including polarization, depolarization, and spin sign
  formu = polVar+"*"+depolVar+"*"+spinVar+"*"+modu->FormuBru();
  this->PrintLog(formuName+" = "+formu);
  FM->SetUp().LoadFormula(formuName+"="+formu);

  // append to list for RooComponentsPDF
  if(numerList!="") numerList += ":";
  numerList += ampName+";"+formuName;

  // append to numerator formula string for EXPR
  if(numerFormu!="") numerFormu += "+";
  numerFormu += ampName+"*"+formuName;
  if(ampNameList!="") ampNameList += ",";
  ampNameList += ampName;
  if(formuNameList!="") formuNameList += ",";
  formuNameList += formuName;
};


// add denominator modulation to PDF
void BruAsymmetry::AddDenomMod(Modulation * modu) {

  // set amplitude and modulation names
  TString ampName = modu->AmpName();
  TString formuName = ampName;
  formuName.ReplaceAll("Amp","Mod");

  // amplitude parameter
  FM->SetUp().LoadParameter(ampName+"[0.0,-1,1]");

  // modulation
  // TODO: move UU depolarization factors to here, if denom
  // amps are possible to constrain
  formu = modu->FormuBru();
  this->PrintLog(formuName+" = "+formu);
  FM->SetUp().LoadFormula(formuName+"="+formu);

  // append to denominator formula string for EXPR
  if(denomFormu!="") denomFormu += "+";
  denomFormu += ampName+"*"+formuName;
  if(ampNameList!="") ampNameList += ",";
  ampNameList += ampName;
  if(formuNameList!="") formuNameList += ",";
  formuNameList += formuName;

  nDenomParams++;
};


// build full PDF
void BruAsymmetry::BuildPDF() {

  // build PDFstr
  TString obsList = "PhiH,PhiR,PhiD,Theta,Pol,Depol2,Depol3,"+spinBranch;
  if(nDenomParams==0) {
    // if PDF has numerator amplitudes only, we can use RooComponentsPDF
    PDFstr = "RooComponentsPDF::PWfit(1,"; // PDF class::name ("+1" term ,
    PDFstr += "{"+obsList+"},"; // observables list
    PDFstr += "=" + numerList + ")"; // sum_i { pol *depol*spin * amp_i * mod_i }
    // alternatively, use EXPR
    //PDFstr = "EXPR::PWfit('1+"+numerFormu+"'";
    //PDFstr += ","+obsList+","+ampNameList+","+formuNameList+")";
  } else {
    // if PDF has denominator amplitudes, must use EXPR
    PDFstr = "EXPR::PWfit('1+("+numerFormu+")/(1+"+denomFormu+")'";
    PDFstr += ","+obsList+","+ampNameList+","+formuNameList+")";
  };


  // construct the extended likelihood
  this->PrintLog("construct PDF "+PDFstr);
  FM->SetUp().FactoryPDF(PDFstr);
  FM->SetUp().LoadSpeciesPDF("PWfit",1); /* second arg is lower bound of
                                          * yield parameter */

  // print PDF
  this->PrintFitter();
};


// load data and MC events into FitManager
void BruAsymmetry::LoadDataSets(
    TString dataFileN,
    TString mcFileN,
    TString weightFileN,
    TString weightName,
    TString treeName
    )
{

  // load data tree
  FM->LoadData(treeName,dataFileN);

  // load MC data, for normalization integral
  if(mcFileN=="") {
    this->PrintLog("MC INTEGRATION DISABLED");
    useMCint = false;
  } else {
    this->PrintLog(Form("MC INTEGRATION ENABLED, using %s",mcFileN.Data()));
    useMCint = true;
    FM->LoadSimulated(treeName,mcFileN,"PWfit");
  };

  // load weights (a Tweights.root file, likely from sPlot)
  if(weightFileN=="") {
    useWeights = false;
  } else {
    this->PrintLog(Form("WEIGHTS ENABLED, using %s",weightFileN.Data()));
    useWeights = true;
    FM->Data().LoadWeights(weightName,weightFileN);
  };

};


// bin the data (and MC) according to specified binning scheme
// ( see Binning::SetScheme )
void BruAsymmetry::Bin(Binning * binscheme) {
  for(int d=0; d<binscheme->dimensions; d++) {
    FM->Bins().LoadBinVar(
      binscheme->GetIVname(d),
      binscheme->GetNbins(d),
      binscheme->GetBinArray(d)->GetArray()
    );
  };
};

// perform the fit
void BruAsymmetry::Fit() {

  // number of parallel threads
  nThreads = (Int_t) std::thread::hardware_concurrency();
  if(nThreads<1) nThreads=1;
  if(nThreads>6) nThreads=6; // max (to not clobber shared nodes)
  nWorkers = TMath::Min(nThreads,this->GetNbins()); // for PROOF
  printf("---- fit with %d parallel threads\n",nWorkers);

  // determine whether to enable acceptance locks
  Bool_t useAccLocks = MCMC_lockacc_min>=0 && MCMC_lockacc_max>MCMC_lockacc_min;

  // define minimizer algorithm
  this->PrintLog("");
  if(minimizer==mkMCMCseq) { // sequential MCMC --------------------------
    this->PrintLog("OPTIMIZER: MCMCseq");
    if(useAccLocks) mcmcAlgo = new HS::FIT::RooMcmcSeqHelper( MCMC_iter, MCMC_burnin, MCMC_norm );
    else            mcmcAlgo = new HS::FIT::RooMcmcSeq(       MCMC_iter, MCMC_burnin, MCMC_norm );
    useMCMC = true;
  } else if(minimizer==mkMCMCcov) { // sequential-then-cov MCMC ----------
    this->PrintLog("OPTIMIZER: MCMCcov");
    mcmcAlgo = new HS::FIT::RooMcmcSeqThenCov(
        MCMC_iter,     MCMC_burnin,     MCMC_norm,
        MCMC_cov_iter, MCMC_cov_burnin, MCMC_cov_norm
        );
    useMCMC = true;
  } else if(minimizer==mkMinuit) { // Minuit -----------------------------
    this->PrintLog("OPTIMIZER: Minuit");
    minuitAlgo = new HS::FIT::Minuit2();
  } else { // ------------------------------------------------------------
    fprintf(stderr,"ERROR: unknown minimizer in BruAsymmetry::Fit()\n");
    return;
  };

  // additioal settings for MCMC algos
  if(useMCMC) {
    // print hyperparameters to log file
    this->PrintLog( Form("MCMC seq chain: iter,burnin,stepsize = %d, %d, %f",MCMC_iter,MCMC_burnin,1.0/MCMC_norm));
    if(minimizer==mkMCMCcov) this->PrintLog( Form("MCMC cov chain: iter,burnin,stepsize = %d, %d, %f",MCMC_cov_iter,MCMC_cov_burnin,1.0/MCMC_cov_norm));
    // enable additional plots for MCMC performance evaluation
    FM->SetPlotOptions("MCMC:CORNERFULL:CORNERZOOM:AUTOCORR");
    // set acceptance rate locks (if desired)
    if(useAccLocks) {
      this->PrintLog(Form("MCMC acceptance locks enabled: min,max,target = %f, %f, %f",MCMC_lockacc_min,MCMC_lockacc_max,MCMC_lockacc_target));
      mcmcAlgo->SetDesiredAcceptance(MCMC_lockacc_min,MCMC_lockacc_max,MCMC_lockacc_target);
    } else this->PrintLog("MCMC acceptance locks disabled");
  };

  // stage minmizer algorithm to fit manager
  if(useMCMC) FM->SetMinimiser(mcmcAlgo);
  else        FM->SetMinimiser(minuitAlgo);


  // =====================================================================


  // fit settings:
  // -optimize: calculate and cache formulas; suitable for RooComponentsPDF
  // -number of parellel threads for the likelihood calculation (choose either
  //  this multi-threading method, or use PROOF below for one fit = one thread; don't
  //  choose both methods, unless you have a lot of threads available)
  FM->SetUp().AddFitOption(RooFit::Optimize(1));
  //FM->SetUp().AddFitOption(RooFit::NumCPU(nWorkers));


  // MINOS uncertainty estimation
  //FM->SetUp().AddFitOption(RooFit::Minos(kTRUE));


  // perform the fit
  //HS::FIT::PROCESS::Here::Go(FM); // do not use PROOF
  HS::FIT::PROCESS::Proof::Go(FM,nWorkers); // use PROOF
};


// get number of bins and dimensions configured in FitManager
Int_t BruAsymmetry::GetNbins() {
  Int_t nb = 1;
  for(auto axis : FM->Bins().GetBins().GetVarAxis())
    nb *= axis.GetNbins();
  return nb;
};
Int_t BruAsymmetry::GetNdim() {
  return FM->Bins().GetBins().GetNAxis();
};

// print bin information
void BruAsymmetry::PrintBinScheme() {
  printf("\nBin Scheme defined in BruAsymmetry FM:\n");
  Tools::PrintSeparator(50,"=");
  printf("- nBins = %d\n- nDims = %d\n",
    this->GetNbins(), this->GetNdim() );
  for(auto axis : FM->Bins().GetBins().GetVarAxis()) {
    Tools::PrintSeparator(50,"-");
    printf("%s axis has %d bins:\n",
      axis.GetName(), axis.GetNbins() );
    for(int ib=1; ib<=axis.GetNbins(); ib++) {
      printf("  bin %d:  %.3f to %.3f\n",
        ib, axis.GetBinLowEdge(ib), axis.GetBinUpEdge(ib) );
    };
  };
  Tools::PrintSeparator(50,"=");
};

BruAsymmetry::~BruAsymmetry() {
};

