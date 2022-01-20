R__LOAD_LIBRARY(DiSpin)
#include <TList.h>
#include <TFile.h>
#include <TString.h>
#include <RooFitResult.h>
#include <RooArgList.h>
#include <RooRealVar.h>
#include <TGraphErrors.h>

#include "Bins.h"

#include "Constants.h"

/*
 * ADAPTED FROM: dglazier:brufit/macros/GraphParameters.C, modified for dispin
 */

//usage brufit
//.L DrawResiduals.C
//DrawResiduals("out/","x") where x is a binned variable
//                           e.g. RF.Bins().LoadBinVar("x",4,3,4);
void DrawResiduals(TString DirName,TString Var,TString minimizer_){

  gROOT->ProcessLine(".! mkdir -p "+DirName+"/residuals");
  TFile* file=new TFile(DirName+"/DataBinsConfig.root");
  auto DataBins=(HS::FIT::Bins*)file->Get("HSBins");
  TString BinName=DataBins->GetBinName(0);
  // cout<<BinName<<endl;

  Int_t va= DataBins->GetAxisi(Var);
  TString AxisName=Var;

  // set results file name
  Int_t minimizer = MinimizerStrToEnum(minimizer_);
  if(minimizer<0) return;
  TString resultsFileN = BrufitResultsFileName(minimizer);

  for(Int_t ib=0;ib<DataBins->GetN();ib++){
    TString redName=DataBins->GetBinName(ib);
    //Strip the variable we are plotting from the name
    //so we can find the relevent graph
    TString axisBin=redName(redName.Index(AxisName),TString(redName(redName.Index(AxisName)+AxisName.Sizeof()-1,redName.Sizeof())).First("_")+AxisName.Sizeof());
    redName.Replace(redName.Index(AxisName),TString(redName(redName.Index(AxisName)+AxisName.Sizeof()-1,redName.Sizeof())).First("_")+AxisName.Sizeof(),"");


    Int_t iP=DataBins->GetParti(va,axisBin);//index for this bin on this axis
    // cout<<redName<<" "<<axisBin<<" "<<iP<<" "<<va<<endl;
    //Open the file with the results
    TFile* fileR=new TFile(DirName+"/"+DataBins->GetBinName(ib)+"/"+resultsFileN);
    if(!fileR->IsOpen()) continue;

    for(TString varName : {"PhiH","PhiR","Theta"}) {
      TString canvN = DataBins->GetBinName(ib)+"_"+varName;
      cout << "GET " << canvN << endl;
      TCanvas *canv = (TCanvas*)fileR->Get(canvN);
      for(TObject *obj : *canv->GetPad(1)->GetListOfPrimitives()) { // make parameter box invisible, except for its text
        //cout << obj->GetName() << endl;
        if(TString(obj->GetName()).Contains("paramBox")) {
          TPaveText *pave = dynamic_cast<TPaveText*>(obj);
          pave->SetFillStyle(0);
          pave->SetLineWidth(0);
        };
      };
      canv->SaveAs(DirName+"/residuals/"+varName+"_"+DataBins->GetBinName(ib)+".png");
    };

    fileR->Close();
    delete fileR;
  }

  TFile* fileG=new TFile(DirName+"/ParGraphs"+Var+".root","recreate");
  fileG->Close();
  delete fileG;

  file->Close();
  delete file;

  return;


}
