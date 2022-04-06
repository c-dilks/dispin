R__LOAD_LIBRARY(DiSpin)
#include "Constants.h"
//
// IMPORTANT: run with `brufit -b -q sPlotBru.C`
//

void sPlotBru(
    TString infileN="catTreeData.rga_inbending_all.0x3b.idx.trimmed.root",
    TString outDir="splot",
    Int_t pairType=0x3b,
    Int_t ivType=2,
    Int_t nbins0=-1, Int_t nbins1=-1, Int_t nbins2=-1
    ) {

  // set PROOF sandbox (where log files etc. are written)
  // - you may need to create or symlink this directory (`pwd`/farmout)
  TString sandbox = TString(gSystem->Getenv("PWD")) + "/farmout/" + outDir + "/prooflog";
  gEnv->SetValue("ProofLite.Sandbox",sandbox.Data());
  printf("proof sandbox = %s\n",gEnv->GetValue("ProofLite.Sandbox","ERROR"));

  // input tree name
  TString treeName = "tree";

  // determine fit range
  // IMPORTANT: must match that in `TrimCatTree.C`
  TFile *infile = new TFile(infileN,"READ");
  TTree *inTr = (TTree*) infile->Get(treeName);
  TH1D * Mdist = new TH1D("Mdist","Mdist",100,0.04,0.45);
  inTr->Project("Mdist","diphM");
  // - lower bound: somewhere between pi0 window and low-M peak
  Double_t fitLB = 0.08;
  // - upper bound: get maximum diphoton mass bin with nonzero entries (trying
  //   to fit beyond that point will cause problems)
  /*
  Int_t bb = Mdist->FindBin(0.135);
  while(Mdist->GetBinContent(bb)>0 && bb<=Mdist->GetNbinsX()) bb++;
  Double_t MggMax = Mdist->GetBinCenter(bb);
  Double_t fitUB = TMath::Min( 0.2, 0.95*MggMax); // stay low, don't overfit
  */
  Double_t fitUB = 0.2; // use fixed upper bound instead
  cout << "FIT RANGE: " << fitLB << " < diphM < " << fitUB << endl;

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
  BS->SetScheme(pairType,ivType,nbins0,nbins1,nbins2);
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

  // draw sWeighted distributions
  TCanvas *canv;
  TString canvN;
  Int_t canv_colors[4] = {kBlack,kGreen-3,kRed+2,kAzure+10};
  TCut cut = Form("%f<diphM && diphM<%f",fitLB,fitUB);
  gStyle->SetPalette(4,canv_colors);
  gStyle->SetOptStat(0);
  TString bounds,varTitle;
  Double_t varMin,varMax,varRange;
  // - loop through variables for distributions
  for(TString varName : {"X","Mh","Z","PhPerp","Q2","XF","DY","DYsgn","PhiH","PhiR","Theta","Depol2","Depol3","diphM"}) {
    cout << "draw sWeighted " << varName << endl;

    // prepare canvas
    canvN = varName+"canv"; 
    canv = new TCanvas(canvN,canvN,1600,700);
    canv->Divide(2,1);
    for(int p=1; p<=2; p++) canv->GetPad(p)->SetGrid(1,1);

    // histogram range and binning
    varMin=inTr->GetMinimum(varName);   varMax=inTr->GetMaximum(varName);
    varRange=varMax-varMin;  varMin-=0.05*varRange;  varMax += 0.05*varRange;
    if(varName=="diphM") { varMin=fitLB; varMax=fitUB; };
    bounds = Form("_%s(200,%f,%f)",varName.Data(),varMin,varMax);

    // draw unweighted dists
    canv->cd(1);
    inTr->Draw(varName+">>swM"+bounds,cut,"PLC"); // fit region, black line
    if(varName!="diphM") inTr->Draw(varName+">>swSB"+bounds,"0.17<diphM && diphM<0.4","SAME PLC"); // sideband region, gray line

    // draw sWeighted dists
    SP.DrawWeighted(varName+">>swS"+bounds,"Signal","","SAME PLC"); // signal, green points
    SP.DrawWeighted(varName+">>swB"+bounds,"BG","","SAME PLC"); // background, blue points

    // draw correlation
    varTitle = varName;
    if(varTitle=="diphM") varTitle="M_{#gamma#gamma}";
    if(varTitle=="Mh") varTitle="M_{h}";
    if(varTitle=="PhPerp") varTitle="p_{T}";
    if(varTitle=="Q2") varTitle="Q^{2}";
    if(varName!="diphM") {
      canv->cd(2);
      TH2D *corr = new TH2D("corr_"+varName,varTitle+" vs. M_{#gamma#gamma}",70,0.07,0.2,70,varMin,varMax);
      inTr->Project(corr->GetName(),varName+":diphM","");
      corr->Draw("BOX");
      Tools::ApplyProfile(corr,1);
    };

    // save output canvas
    Tools::UnzoomVertical(canv->GetPad(1),varTitle+" comparison");
    canv->SaveAs(outDir+"/sWeighted__"+varName+"__dist.png");
    canv->SaveAs(outDir+"/sWeighted__"+varName+"__dist.root");
  };

  // make sure weighted tree is written properly
  SP.DeleteWeightedTree();

  // draw parameters vs. bin, and fit results
  TString cmd;
  for(int d=0; d<BS->dimensions; d++) {
    cmd = Form(".x GraphBruParameters.C(\"%s/\",\"%s\")",outDir.Data(),BS->GetIVname(d).Data());
    printf("\nEXECUTE: %s\n\n",cmd.Data());
    gROOT->ProcessLine(cmd);
  };

  // cleanup
  infile->Close();

};

