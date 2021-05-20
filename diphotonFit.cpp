#include <cstdlib>
#include <iostream>
#include <map>

// ROOT
#include <TFile.h>
#include <TString.h>
#include <TMath.h>
#include <TSystem.h>
#include <TROOT.h>
#include <TRegexp.h>
#include <TH1.h>
#include <TTree.h>
#include <TIterator.h>
#include <TGraphErrors.h>

// RooFit
#include <RooGlobalFunc.h>
#include <RooRealVar.h>
#include <RooAbsReal.h>
#include <RooDataSet.h>
#include <RooDataHist.h>
#include <RooGaussian.h>
#include <RooPolynomial.h>
#include <RooChebychev.h>
#include <RooGenericPdf.h>
#include <RooAddPdf.h>
#include <RooFitResult.h>
#include <RooPlot.h>
#include <RooStats/SPlot.h>

// dispin
#include <Binning.h>
#include <Constants.h>
#include <Tools.h>


using std::cout;
using std::cerr;
using std::endl;
using std::map;
using std::pair;

TString infileN, outfileN;
TFile *infile, *outfile;
TTree *tr;
Int_t ivType;
Int_t numBin[3];

Binning *BS;
Long64_t ENT;


// class for histograms, fit results, etc., for one bin of Binning scheme
class FitBin {
  public:
    TH1D *Mdist;
    TH1D *IVdist;
    Int_t binnum;
    TString binStr,boundStr,massdistN,modelN;
    Double_t pi0LB,pi0UB,pi0purity;
    RooFitResult *modelFit;
    RooPlot *plotFrame;
    RooRealVar mass;

    // constructor -------------------------------------
    FitBin(Int_t binn) {
      binnum = binn;
      binStr = Form("_%d",binnum);
      boundStr = "";
      for(int d=0; d<BS->dimensions; d++)
        boundStr += ", " + BS->GetBoundStr(binnum,d);
      Mdist = new TH1D("Mdist"+binStr,"Mdist"+binStr,
          100,0.04,0.45
          );
      IVdist = new TH1D("IVdist"+binStr,"IVdist"+binStr,
          500,BS->minIV[BS->ivVar[0]],BS->maxIV[BS->ivVar[0]]);
    };

    // fit algorithm -----------------------------------
    void Fit() {
      for(int i=0; i<3; i++) Tools::PrintSeparator(60,"=");
      cout << "[+++++] call fit on bin " << binnum << endl;

      // diphoton mass variable
      mass = RooRealVar(("Mgg"+binStr).Data(),"M_{#gamma#gamma}",0,3,"GeV");
      massdistN = "roo_" + TString(Mdist->GetName());
      RooDataHist massdist(massdistN.Data(),massdistN.Data(),
          mass,RooFit::Import(*Mdist));

      // pi0 signal
      // WARNING: do not use underscores in param names
      Double_t nmax = (Double_t)ENT;
      nmax /= BS->GetNbinsTotal();
      RooRealVar pi0n(("pi0n"+binStr).Data(),"#pi^{0} N", nmax/2.0, 0, nmax);
      RooRealVar pi0mu(("pi0mu"+binStr).Data(),"#pi^{0} M", PartMass(kPio), 0, 2);
      RooRealVar pi0sigma(("pi0sigma"+binStr).Data(),"#pi^{0} #sigma", 0.02, 0.001, 0.1);
      RooGaussian pi0Model(("pi0Model"+binStr).Data(),"pi0Model",mass,pi0mu,pi0sigma);

      // background
      RooRealVar bgN(("bgN"+binStr).Data(),"BG N", 0, nmax);
      RooRealVar bgP0(("bgP0"+binStr).Data(),"BG p_{0}", -1, 1);
      RooRealVar bgP1(("bgP1"+binStr).Data(),"BG p_{1}", -1, 1);
      RooRealVar bgP2(("bgP2"+binStr).Data(),"BG p_{2}", -1, 1);
      RooRealVar bgP3(("bgP3"+binStr).Data(),"BG p_{3}", -1, 1);
      RooRealVar bgP4(("bgP4"+binStr).Data(),"BG p_{4}", -1, 1);
      //RooPolynomial bgModel(("bgModel"+binStr).Data(),"bgModel",mass,RooArgSet(bgP0,bgP1,bgP2));
      //RooChebychev bgModel(("bgModel"+binStr).Data(),"bgModel",mass,RooArgSet(bgP0,bgP1,bgP2));
      //RooChebychev bgModel(("bgModel"+binStr).Data(),"bgModel",mass,RooArgSet(bgP0,bgP1,bgP2,bgP3));
      RooChebychev bgModel(("bgModel"+binStr).Data(),"bgModel",mass,RooArgSet(bgP0,bgP1,bgP2,bgP3,bgP4));

      // signal+bg
      modelN = "model"+binStr;
      RooAddPdf fullModel(modelN.Data(),"model",
          RooArgList(pi0Model,bgModel),
          RooArgList(pi0n,bgN)
          );

      // fit range
      // - get maximum diphoton mass bin with nonzero entries
      //   (trying to fit beyond that point will cause problems)
      Int_t bb = Mdist->FindBin(PartMass(kPio));
      while(Mdist->GetBinContent(bb)>0 && bb<=Mdist->GetNbinsX()) bb++;
      Double_t MggMax = Mdist->GetBinCenter(bb);
      // - set fit range
      mass.setRange(("fitRange"+binStr).Data(),
          0.08,
          TMath::Min( 0.4, 0.95*MggMax) /* ensure we're below eta region */
          );

      // perform fit
      modelFit = fullModel.fitTo(
          massdist,
          RooFit::Range(("fitRange"+binStr).Data()),
          RooFit::Extended(kTRUE),
          RooFit::Save(kTRUE)
          );


      // determine pi0 signal range
      Int_t nsigma = 2;
      pi0LB = pi0mu.getVal() - nsigma * pi0sigma.getVal();
      pi0UB = pi0mu.getVal() + nsigma * pi0sigma.getVal();
      // OVERRIDE signal range (fix pi0 cuts for all bins)
      pi0LB = 0.106801; // from single-bin fit
      pi0UB = 0.152718;
      mass.setRange("pi0range",pi0LB,pi0UB);

      
      // calculate pi0 purity
      // - integrate PDFs
      RooAbsReal *pi0ModelIntObj = pi0Model.createIntegral(
          mass,RooFit::NormSet(mass),RooFit::Range("pi0range"));
      RooAbsReal *bgModelIntObj = bgModel.createIntegral(
          mass, RooFit::NormSet(mass),RooFit::Range("pi0range"));
      RooAbsReal *fullModelIntObj = fullModel.createIntegral(
          mass,RooFit::NormSet(mass),RooFit::Range("pi0range"));
      Double_t pi0ModelInt  = pi0ModelIntObj->getVal();
      Double_t bgModelInt   = bgModelIntObj->getVal();
      Double_t fullModelInt = fullModelIntObj->getVal();
      // - integrate data histogram
      Int_t pi0LBbin = Mdist->FindBin(pi0LB);
      Int_t pi0UBbin = Mdist->FindBin(pi0UB);
      Double_t MdistInt = Mdist->Integral(pi0LBbin,pi0UBbin);
      // purity
      Double_t purity1 = pi0n.getVal() * pi0ModelInt / MdistInt; // pure, divide data
      Double_t purity2 = pi0n.getVal() * pi0ModelInt / fullModelInt; // pure, divide model
      Double_t purity3 = 1 - bgN.getVal() * bgModelInt / MdistInt; // 1-impurity, divide data
      Double_t purity4 = 1 - bgN.getVal() * bgModelInt / fullModelInt; // 1-impurity, divide model
      pi0purity = purity3; // <--- decide which purity calculation
      cout << "purity deltas: " /* print calculation differences */
           << " " << pi0purity - purity1
           << " " << pi0purity - purity2
           << " " << pi0purity - purity3
           << " " << pi0purity - purity4
           << endl;


      // plot
      TString massdistT = "M_{#gamma#gamma} distribution"+boundStr;
      plotFrame = mass.frame(
          RooFit::Title(massdistT)
          );
      massdist.plotOn(plotFrame,
          RooFit::Name(("massdistPlot"+binStr).Data()),
          RooFit::Invisible(kTRUE)
          );
      fullModel.plotOn(plotFrame,
          RooFit::Name(("modelPlot"+binStr).Data()),
          RooFit::Range(("fitRange"+binStr).Data()),
          RooFit::LineColor(kRed),
          RooFit::LineWidth(2)
          );
      fullModel.plotOn(plotFrame,
          RooFit::Name(("pi0ModelPlot"+binStr).Data()),
          RooFit::Components(("pi0Model"+binStr).Data()),
          RooFit::Range(("fitRange"+binStr).Data()),
          RooFit::LineColor(kGreen+1),
          RooFit::LineWidth(2)
          );
      fullModel.plotOn(plotFrame,
          RooFit::Name(("bgModelPlot"+binStr).Data()),
          RooFit::Components(("bgModel"+binStr).Data()),
          RooFit::Range(("fitRange"+binStr).Data()),
          RooFit::LineColor(kBlue),
          RooFit::LineWidth(2),
          RooFit::LineStyle(5)
          );
    };
};


////////////////////////////////////////
////////////////////////////////////////
////////////////////////////////////////
////////////////////////////////////////


int main(int argc, char** argv) {

  // ARGUMENTS
  if(argc<=2) {
    cerr << "USAGE: " << argv[0] << " [tree file] [IV type] [num bins]" << endl;
    BS = new Binning();
    cout << " - IV types:" << endl
         << "    1, 2, or 3 digit number specifying binning scheme;" << endl
         << "    available vars:" << endl;
    for(int i=0; i<Binning::nIV; i++)
      cout << "     " << i+1 << " = " << BS->IVtitle[i] << endl;
    cout << " - num bins: number of bins, listed for each IV," << endl
         << "   separated by spaces (default is single bin)" << endl;
    return 1;
  };
  infileN = TString(argv[1]);
  ivType = (Int_t)strtof(argv[2],NULL);
  for(int i=0; i<3; i++) numBin[i]=-1;
  if(argc>3) {
    int ab=0;
    for(int ar=3; ar<argc; ar++)
      numBin[ab++] = (Int_t)strtof(argv[ar],NULL);
  };
  cout << "infileN = " << infileN << endl
       << "ivType = " << ivType << endl
       << "numBin = [" << numBin[0] << ","
                       << numBin[1] << ","
                       << numBin[2] << "]" << endl;


  // set binning scheme
  BS = new Binning();
  BS->SetScheme(ivType,numBin[0],numBin[1],numBin[2]);


  // open TFiles
  infile = new TFile(infileN,"READ");
  outfileN = infileN;
  outfileN(TRegexp("cat__")) = "fit__";
  outfile = new TFile(outfileN,"RECREATE");


  // open tree
  Double_t ivVal[3] = {0,0,0};
  Double_t Mgg;
  tr = (TTree*)infile->Get("diphTr");
  for(int d=0; d<BS->dimensions; d++) {
    cout << "IV " << d << " is " << BS->GetIVname(d) << endl;
    tr->SetBranchAddress(BS->GetIVname(d),&ivVal[d]);
  };
  tr->SetBranchAddress("diphM",&Mgg);


  // create FitBin object for each bin
  map<Int_t,FitBin*> FitBinList;
  for(Int_t bn : BS->binVec) {
    FitBinList.insert(pair<Int_t,FitBin*>(bn,new FitBin(bn)));
  };


  // loop through tree
  FitBin *fb;
  ENT = tr->GetEntries();
  for(Long64_t e=0; e<ENT; e++) {
    if(e%10000==0) printf("[+] %.2f%%\n",100*(float)e/((float)ENT));
    tr->GetEntry(e);

    // find bin, and set FitBin pointer
    try {
      fb = FitBinList.at(BS->FindBin(ivVal[0],ivVal[1],ivVal[2]));
    } catch (const std::out_of_range &ex) {
      cerr << "ERROR: cannot find bin" << endl;
      continue;
    };

    // fill histograms
    fb->Mdist->Fill(Mgg);
    fb->IVdist->Fill(ivVal[0]);
  };


  // fit histograms
  for(Int_t bn : BS->binVec) FitBinList.at(bn)->Fit();


  // loop through fit parameters and results
  // TODO: paramGr is only good for 1D binning schemes;
  // quick fix: for multidim, could just plot params vs. binnum
  for(int i=0; i<3; i++) Tools::PrintSeparator(60,"=");
  Tools::PrintTitleBox("FIT RESULTS");
  Bool_t first = true;
  map<TString,TGraphErrors*> paramGrList;
  TGraphErrors *paramGr;
  TString parName;
  Int_t ptCnt = 0;
  Int_t nParam = 0;;
  for(Int_t bn : BS->binVec) {

    fb = FitBinList.at(bn);
    cout << "BIN " << fb->binnum
         << " -- " << BS->GetIVname(0)
         << " = " << fb->IVdist->GetMean()
         << endl;

    auto parList = fb->modelFit->floatParsFinal();
    parList.sort();
    TIterator *nextPar = parList.createIterator();

    // loop over fit params
    while(RooRealVar *par = (RooRealVar*) (*nextPar)()) {

      // print
      parName = TString(par->GetName());
      parName(TRegexp("_.*$"))="";
      cout << "  " << parName << " = "
                   << par->getVal() << " +/- "
                   << par->getError()
                   << endl;

      // instantiate graph
      if(first) {
        paramGr = new TGraphErrors();
        paramGr->SetName("paramGr_"+parName);
        paramGr->SetTitle(TString(par->GetTitle())+" vs. "+BS->GetIVtitle(0));
        paramGrList.insert(pair<TString,TGraphErrors*>(parName,paramGr));
        nParam++;
      } else {
        paramGr = paramGrList.at(parName);
      };

      paramGr->SetPoint(ptCnt,fb->IVdist->GetMean(),par->getVal());
      paramGr->SetPointError(ptCnt,0,par->getError());
    };
    first = false;
    ptCnt++;
  };

  // parameter canvas
  Int_t ncols = 3;
  Int_t nrows = (nParam-1)/ncols+1;
  TCanvas *paramCanv = new TCanvas("paramCanv","paramCanv",800*ncols,600*nrows);
  paramCanv->Divide(ncols,nrows);
  Int_t pad=1;
  for(auto & parKV : paramGrList) {
    paramGr = parKV.second;
    paramCanv->cd(pad);
    paramGr->SetMarkerStyle(kFullCircle);
    paramGr->SetMarkerColor(kBlack);
    paramGr->Draw("APE");
    pad++;
  };

  // fit results canvas
  nrows = (BS->GetNbinsTotal()-1)/ncols+1;
  TCanvas *fitCanv = new TCanvas("fitCanv","fitCanv",800*ncols,600*nrows);
  TLine *pi0Bline[2];
  fitCanv->Divide(ncols,nrows);
  pad=1;
  Double_t mx;
  for(Int_t bn : BS->binVec) {
    fb = FitBinList.at(bn);

    fitCanv->cd(pad);
    fitCanv->GetPad(pad)->SetBottomMargin(0.15);
    fitCanv->GetPad(pad)->SetLeftMargin(0.15);
    fb->plotFrame->Draw();
    fb->Mdist->SetLineWidth(1);
    fb->Mdist->SetLineColor(kBlack);
    fb->Mdist->SetMarkerSize(0.0);
    fb->Mdist->Draw("ESAME");

    mx = fb->Mdist->GetMaximum();
    pi0Bline[0] = new TLine(fb->pi0LB,0,fb->pi0LB,mx);
    pi0Bline[1] = new TLine(fb->pi0UB,0,fb->pi0UB,mx);
    for(int l=0; l<2; l++) {
      pi0Bline[l]->SetLineWidth(3);
      pi0Bline[l]->SetLineColor(kPink-9);
      pi0Bline[l]->Draw();
    };
    
    pad++;
  };


  // fit quality assessment
  // -chi2/ndf
  TGraph *chi2gr = new TGraph;
  TCanvas *chi2grCanv = new TCanvas("chi2grCanv","chi2grCanv",800,600);
  ptCnt=0;
  for(Int_t bn : BS->binVec) {
    fb = FitBinList.at(bn);
    chi2gr->SetPoint(
        ptCnt++,
        fb->IVdist->GetMean(),
        fb->plotFrame->chiSquare("modelPlot"+fb->binStr,"massdistPlot"+fb->binStr)
        );
  };
  chi2gr->SetTitle("#chi^{2}/NDF vs. "+BS->GetIVtitle(0));
  chi2gr->SetMarkerStyle(kFullCircle);
  chi2gr->Draw("AP");
  // -residuals & pulls
  RooPlot *residFrame;
  RooPlot *pullFrame;
  nrows = (BS->GetNbinsTotal()-1)/ncols+1;
  TCanvas *residCanv = new TCanvas("residCanv","residCanv",800*ncols,600*nrows);
  TCanvas *pullCanv = new TCanvas("pullCanv","pullCanv",800*ncols,600*nrows);
  residCanv->Divide(ncols,nrows);
  pullCanv->Divide(ncols,nrows);
  pad=1;
  for(Int_t bn : BS->binVec) {
    fb = FitBinList.at(bn);
    residFrame = fb->mass.frame(RooFit::Title("residuals"+fb->boundStr));
    pullFrame = fb->mass.frame(RooFit::Title("pulls"+fb->boundStr));
    residFrame->addPlotable(
        fb->plotFrame->residHist("massdistPlot"+fb->binStr,"modelPlot"+fb->binStr)
        );
    pullFrame->addPlotable(
        fb->plotFrame->pullHist("massdistPlot"+fb->binStr,"modelPlot"+fb->binStr)
        );
    residCanv->cd(pad);
    residCanv->GetPad(pad)->SetBottomMargin(0.15);
    residCanv->GetPad(pad)->SetLeftMargin(0.15);
    residFrame->Draw();
    pullCanv->cd(pad);
    pullCanv->GetPad(pad)->SetBottomMargin(0.15);
    pullCanv->GetPad(pad)->SetLeftMargin(0.15);
    pullFrame->Draw();
    pad++;
  };


  // print pi0 purity results
  cout << "----- pi0 purity -----" << endl;
  for(Int_t bn : BS->binVec) {
    fb = FitBinList.at(bn);
    cout << "bin " << bn << endl;
    cout << "  pi0 window: "
         << fb->pi0LB << " to " << fb->pi0UB << endl;
    cout << "  pi0 purity: " << fb->pi0purity << endl;
  };


  // write
  outfile->cd();
  fitCanv->Write();
  paramCanv->Write();
  chi2grCanv->Write();
  residCanv->Write();
  pullCanv->Write();
  outfile->mkdir("singlePlots");
  outfile->cd("singlePlots");
  for(Int_t bn : BS->binVec) {
    fb = FitBinList.at(bn);
    fb->Mdist->Write();
  };
  outfile->cd("/");


  // print canvas (for quick look)
  fitCanv->Print("diphotonFitResultCanv.png");
  paramCanv->Print("diphotonFitParamCanv.png");
  chi2grCanv->Print("diphotonFitChi2.png");
  residCanv->Print("diphotonFitResiduals.png");
  pullCanv->Print("diphotonFitPulls.png");


  // cleanup
  outfile->Close();
  infile->Close(); 
  return 0;
};
