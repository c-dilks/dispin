R__LOAD_LIBRARY(DiSpin)
#include "GraphParameters.C" // $BRUFIT/macros
//
// IMPORTANT: run with `brufit sPlotBru.C`
//

void sPlotBru(
    TString outDir="splot",
    Int_t binschemeIVtype=2,
    Int_t nbins0=-1, Int_t nbins1=-1, Int_t nbins2=-1
    ) {

  // input file name and tree name
  TString infileN="catTreeData.rga_inbending_all.0x3b.idx.root";
  TString treeName = "tree";

  // determine fit range
  TFile *infile = new TFile(infileN,"READ");
  TTree *inTr = (TTree*) infile->Get(treeName);
  TH1D * Mdist = new TH1D("Mdist","Mdist",100,0.04,0.45);
  inTr->Project("Mdist","diphM");
  // - lower bound: somewhere between pi0 window and low-M peak
  Double_t fitLB = 0.08;
  // - upper bound: get maximum diphoton mass bin with nonzero entries (trying
  //   to fit beyond that point will cause problems)
  Int_t bb = Mdist->FindBin(0.135);
  while(Mdist->GetBinContent(bb)>0 && bb<=Mdist->GetNbinsX()) bb++;
  Double_t MggMax = Mdist->GetBinCenter(bb);
  Double_t fitUB = TMath::Min( 0.2, 0.95*MggMax); // stay low, don't overfit
  cout << "FIT RANGE: " << fitLB << " < diphM < " << fitUB << endl;
  infile->Close();

  // setup sPlot
  sPlot SP;
  SP.SetUp().SetOutDir(outDir);
  SP.SetUp().LoadVariable(Form("diphM[%f,%f]",fitLB,fitUB)); // diphoton invariant mass
  SP.SetUp().SetIDBranchName("Idx");

  // signal PDF
  SP.SetUp().FactoryPDF("Gaussian::Signal( diphM, pi0mu[0.135,0,2], pi0sigma[0.02,0.001,0.1] )");
  SP.SetUp().LoadSpeciesPDF("Signal",1);

  // background PDF
  TString chebychevParams = "{";
  chebychevParams +=  "b0[0,-1,1]"; // b0*x
  chebychevParams += ",b1[0,-1,1]"; // b1*(2x^2-1)
  //chebychevParams += ",b2[0,-1,1]";
  //chebychevParams += ",b3[0,-1,1]";
  chebychevParams += "}";
  SP.SetUp().FactoryPDF(Form("Chebychev::BG(diphM,%s)",chebychevParams.Data()));
  SP.SetUp().LoadSpeciesPDF("BG",1);

  // set binning scheme
  Binning *BS = new Binning();
  BS->SetScheme(binschemeIVtype,nbins0,nbins1,nbins2);
  Int_t nBins=0;
  for(int d=0; d<BS->dimensions; d++) {
    nBins += BS->GetNbins(d);
    SP.Bins().LoadBinVar(
        BS->GetIVname(d),
        BS->GetNbins(d),
        BS->GetBinArray(d)->GetArray()
        );
  };

  // load data tree (which includes `Idx`)
  SP.LoadData(treeName,infileN);

  // determine how many parallel threads (PROOF)
  Int_t nThreads = (Int_t) std::thread::hardware_concurrency();
  if(nThreads<1) nThreads=1;
  //if(nThreads>6) nThreads=6; // max limit
  Int_t nWorkers = TMath::Min(nThreads,nBins);
  printf("---- fit with %d parallel PROOF workers\n",nWorkers);

  // perform the fit, and determine sWeights
  //Here::Go(&SP); // single-thread
  Proof::Go(&SP,nWorkers); // PROOF

  // draw result
  new TCanvas;
  SP.DrawWeighted("diphM>>(100,0,1)","Signal");
  //compare to true signal
  //FiledTree::Read("MyModel","Data.root")->Tree()->Draw("M1","Sig==1","same");

  // make sure weighted tree is written properly
  SP.DeleteWeightedTree();

  // draw parameters vs. bin
  for(int d=0; d<BS->dimensions; d++) GraphParameters(outDir+"/",BS->GetIVname(d));
};
