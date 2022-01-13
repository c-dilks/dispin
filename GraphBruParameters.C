#include <TList.h>
#include <TFile.h>
#include <TString.h>
#include <RooFitResult.h>
#include <RooArgList.h>
#include <RooRealVar.h>
#include <TGraphErrors.h>
#include "Bins.h"

/*
 * ADAPTED FROM: dglazier:brufit/macros/GraphParameters.C, modified for dispin
 */

void GraphBruParameters(TString DirName,TString Var){

  TList * Graphs=new TList();
  TList * Canvases=new TList();


  TFile* file=new TFile(DirName+"DataBinsConfig.root");
  auto DataBins=(HS::FIT::Bins*)file->Get("HSBins");
  TString BinName=DataBins->GetBinName(0);
  // cout<<BinName<<endl;

  Int_t va= DataBins->GetAxisi(Var);
  TString AxisName=Var;

  // TList * Graphs=new TList();
  Graphs->SetName("AllGraphs");
  Canvases->SetName("AllCanvases");
  for(Int_t ib=0;ib<DataBins->GetN();ib++){
    TString redName=DataBins->GetBinName(ib);
    //Strip the variable we are plotting from the name
    //so we can find the relevent graph
    TString axisBin=redName(redName.Index(AxisName),TString(redName(redName.Index(AxisName)+AxisName.Sizeof()-1,redName.Sizeof())).First("_")+AxisName.Sizeof());
    redName.Replace(redName.Index(AxisName),TString(redName(redName.Index(AxisName)+AxisName.Sizeof()-1,redName.Sizeof())).First("_")+AxisName.Sizeof(),"");
    
 
    Int_t iP=DataBins->GetParti(va,axisBin);//index for this bin on this axis
    // cout<<redName<<" "<<axisBin<<" "<<iP<<" "<<va<<endl;
    //Open the file with the results
    //TFile* fileR=new TFile(DirName+TString("Plots")+DataBins->GetBinName(ib)+".root");
    TFile* fileR=new TFile(DirName+"/"+DataBins->GetBinName(ib)+"/ResultsHSMinuit2.root");
   if(!fileR->IsOpen()) continue;
   RooFitResult* result=dynamic_cast<RooFitResult*>(gDirectory->Get("MinuitResult"));

   TString canvN = DataBins->GetBinName(ib)+"_diphM"; // TODO: don't hard-code var. name
   TCanvas *canv = (TCanvas*)fileR->Get(canvN);
   for(TObject *obj : *canv->GetPad(1)->GetListOfPrimitives()) { // make parameter box invisible, except for its text
     //cout << obj->GetName() << endl;
     if(TString(obj->GetName()).Contains("paramBox")) {
       TPaveText *pave = dynamic_cast<TPaveText*>(obj);
       pave->SetFillStyle(0);
       pave->SetLineWidth(0);
     };
   };
   canv->SaveAs(DirName+"/fitcanv__"+canvN+".png");
   Canvases->Add(canv);

    file->cd();
    RooArgList Pars= result->floatParsFinal();
    //Loop over parameters getting values and error for graph
    TGraphErrors* graph=0;
    for(Int_t ipar=0;ipar<Pars.getSize();ipar++){
      //if graph doesn't exist yet create new one
      if(!(graph= dynamic_cast<TGraphErrors*>(Graphs->FindObject(redName+Pars[ipar].GetName())))){
	graph=new TGraphErrors(0);
	graph->SetNameTitle(redName+Pars[ipar].GetName(),redName+Pars[ipar].GetName());
	Graphs->Add(graph);
      }
      Int_t Npoint=graph->GetN();
      //cout<<redName<<" "<<DataBins->GetAxis(va).GetBinCenter(iP+1)<<endl;
      graph->SetPoint(Npoint,DataBins->GetAxis(va).GetBinCenter(iP+1),((RooRealVar*)(&Pars[ipar]))->getVal());
      graph->SetPointError(Npoint,DataBins->GetAxis(va).GetBinWidth(iP+1)/2,((RooRealVar*)(&Pars[ipar]))->getError());
    }
    
    delete result;
    fileR->Close();
    delete fileR;
    //TGraphErrors* graph=Graphs->Get(redName);
  }

  TFile* fileG=new TFile(DirName+"ParGraphs"+Var+".root","recreate");
  Graphs->Write();
  Canvases->Write();
  fileG->Close();
  delete fileG;

  file->Close();
  delete file;

  return;


}
