#include "BruAsymmetry.h"

ClassImp(BruAsymmetry)

BruAsymmetry::BruAsymmetry(TString outdir_) {

  printf("construct BruAsymmetry\n");
  FM = new HS::FIT::FitManager();
  outdir = outdir_;
  FM->SetUp().SetOutDir(outdir); // calls mkdir automatically


  // variables
  FM->SetUp().LoadVariable(TString("PhiH")+Form("[%f,%f]",-PIe,PIe));
  FM->SetUp().LoadVariable(TString("PhiR")+Form("[%f,%f]",-PIe,PIe));
  FM->SetUp().LoadVariable(TString("PhiD")+Form("[%f,%f]",-PIe,PIe));
  FM->SetUp().LoadVariable(TString("Theta")+Form("[%f,%f]",-0.1,PIe));
  FM->SetUp().LoadVariable(TString("Pol")+Form("[%f,%f]",-1.0,1.0));


  // category for spin
  FM->SetUp().LoadCategory(
    TString("Spin_idx") + Form("[SpinP=%d,SpinM=%d]",sP,sM) );


  // MCMC settings
  MCMC_iter = 1000;
  MCMC_burnin = 200;
  MCMC_norm= 200;


  // misc vars
  numerList = "";

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

  // modulation, including polarization scale factor and spin sign
  FM->SetUp().LoadFormula(formuName+"=@Pol[]*@Spin_idx[]*"+modu->FormuBru());

  // append to list for FactoryPDF
  if(numerList!="") numerList += ":";
  numerList += ampName+";"+formuName;
};


// build full PDF
void BruAsymmetry::BuildPDF() {

  // build PDFstr
  PDFstr = "RooComponentsPDF::PWfit(1,"; // PDF class::name ("+1" term ,
  PDFstr += "{PhiH,PhiR,Theta,Pol,Spin_idx},"; // observables list
  PDFstr += "=" + numerList + ")"; // sum_i { pol * spin * amp_i * mod_i }

  // construct the extended likelihood
  FM->SetUp().FactoryPDF(PDFstr);
  FM->SetUp().LoadSpeciesPDF("PWfit",1); /* second arg is lower bound of
                                          * yield parameter */

  // print PDF
  this->PrintFitter();
};


// load data and MC events into FitManager, by converting RooDataSets to
// disk-resident TTrees
// - RooDataSets expected to be from Asymmetry::rfData
void BruAsymmetry::LoadDataSets(RooDataSet * rooData, RooDataSet * rooMC) {

  // data
  TFile * treeFile = new TFile(outdir+"/treeData.root","RECREATE");
  rooData->convertToTreeStore();
  trData = (TTree*) rooData->tree();
  trData->Write("tree");
  AddUIDbranch(trData);
  treeFile->Close();

  // MC
  treeFile = new TFile(outdir+"/treeMC.root","RECREATE");
  rooMC->convertToTreeStore();
  trMC = (TTree*) rooMC->tree();
  trMC->Write("tree");
  AddUIDbranch(trMC);
  treeFile->Close();

  // load trees into FitManager
  FM->LoadData("tree",outdir+"/treeData.root");
  FM->LoadSimulated("tree",outdir+"/treeMC.root","PWfit");
};
  

// perform the fit
void BruAsymmetry::Fit() {

  // number of parallel threads
  nThreads = (Int_t) std::thread::hardware_concurrency();
  if(nThreads<1) nThreads=1;
  if(nThreads>12) nThreads=12; // max (to not clobber shared nodes)
  nWorkers = TMath::Min(nThreads,this->GetNbins()); // for PROOF
  printf("---- fit with %d parallel threads\n",nWorkers);



  // roofit settings:
  // -optimize: calculate and cache formulas; suitable for RooComponentsPDF
  // -number of parellel threads
  FM->SetUp().AddFitOption(RooFit::Optimize(1));
  FM->SetUp().AddFitOption(RooFit::NumCPU(nWorkers));


  // MINOS uncertainty estimation
  //FM->SetUp().AddFitOption(RooFit::Minos(kTRUE));


  // set minimizer
  FM->SetMinimiser(new HS::FIT::RooMcmcSeq(MCMC_iter,MCMC_burnin,MCMC_norm));

  // perform the fit
  //HS::FIT::PROCESS::Here::Go(FM);
  HS::FIT::PROCESS::Proof::Go(FM,nWorkers);
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

// add ID branch to TTree
void BruAsymmetry::AddUIDbranch(TTree * tr) {
  /*
  UID=0;
  tr->Branch("UID",&UID,"UID/D");
  for(Long64_t i=0; i<tr->GetEntries(); i++) { tr->Fill(); UID++; };
  */
  UIDbr = tr->Branch("UID",&UID,"UID/I");
  for(Long64_t i=0; i<tr->GetEntries(); i++) { UIDbr->Fill(); UID++; };

};
