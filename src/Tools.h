#ifndef TOOLS_H_GUARD
#define TOOLS_H_GUARD

#include "TSystem.h"
#include "TString.h"
#include "TRegexp.h"
#include "TMath.h"
#include "TH1.h"
#include "TH2.h"
#include "TProfile.h"
#include "TVirtualPad.h"
#include "TLorentzVector.h"
#include "Constants.h"

class Tools {
  public:

    // print a separator
    static void PrintSeparator(Int_t nchar, TString sepchar="-") {
      TString outprint = "";
      for(int ss=0; ss<nchar; ss++) outprint += sepchar;
      printf("%s\n",outprint.Data());
    };

    // print a title box
    static void PrintTitleBox(TString outprint) {
      printf("\n");
      PrintSeparator(outprint.Length()+4);
      printf("| %s |\n",outprint.Data());
      PrintSeparator(outprint.Length()+4);
    };

    // get nonzero minimum of a histogram
    static Double_t GetNonzeroMinimum(TH1 *hist) {
      Double_t min=1e10;
      Double_t infinitesimal=1e-10;
      Double_t minTmp;
      for(int b=1; b<hist->GetNbinsX(); b++) {
        minTmp = hist->GetBinContent(b);
        if(minTmp<infinitesimal) continue;
        if(minTmp<min) min=minTmp;
      };
      return min;
    };

    
    // zoom out the vertical scale for the case where multiple
    // `TH1`s have been drawn with the "SAME" option, but the y-axis
    // range is improperly zoomed
    // - example: `UnzoomVertical(canvas->GetPad(3))`
    // - optionally specify a new title 
    // - set `min0` to the desired minimum (e.g., 0.0 so zero is not suppressed, or 
    //   a small nonzero value for the case of drawing with a log axis)
    static void UnzoomVertical(TVirtualPad *pad, TString title="", Double_t min0=UNDEF) {
      Double_t max=-1e10;
      Double_t min=1e10;
      Double_t maxTmp,minTmp;
      for(auto obj : *pad->GetListOfPrimitives()) {
        if(obj->InheritsFrom(TH1::Class())) {
          maxTmp = ((TH1*)obj)->GetMaximum();
          if(pad->GetLogy()>0) minTmp = GetNonzeroMinimum(((TH1*)obj)); // min should not be zero, if drawing with log scale
          else minTmp = ((TH1*)obj)->GetMinimum();
          max = maxTmp > max ? maxTmp : max;
          min = minTmp < min ? minTmp : min;
        };
      };
      if(pad->GetLogy()>0) {
        max *= 2;
        min /= 2;
      } else {
        max += 0.05*(max-min);
        //min -= 0.05*(max-min);
      };
      if(min0>UNDEF) min=min0; // force min to min0, if specified
      for(auto obj : *pad->GetListOfPrimitives()) {
        if(obj->InheritsFrom(TH1::Class())) {
          ((TH1*)obj)->GetYaxis()->SetRangeUser(min,max);
          if(title!="") ((TH1*)obj)->SetTitle(title);
        };
      };
    };


    // shift angle to the range [-PI,+PI)
    static Float_t AdjAngle(Float_t ang) {
      while(ang>PI) ang-=2*PI;
      while(ang<=-PI) ang+=2*PI;
      return ang;
    };
    // shift angle to the range [0,2*PI)
    static Float_t AdjAngleTwoPi(Float_t ang) {
      while(ang>2*PI) ang-=2*PI;
      while(ang<=0) ang+=2*PI;
      return ang;
    };
    // shift angle to the range [-PI/2,3PI/2)
    static Float_t AdjAngleThreeQuarters(Float_t ang) {
      while(ang>3.0*PI/2.0) ang-=2*PI;
      while(ang<=-PI/2.0) ang+=2*PI;
      return ang;
    };

    // convert Eta to Theta (with theta in degrees)
    static Float_t EtaToTheta(Float_t eta) {
      return 2 * TMath::ATan( TMath::Exp( -1*eta) ) * 180.0/PI;
    };
    // convert Energy and Mass to |Momentum|
    static Float_t EMtoP(Float_t energy, Float_t mass) {
      return TMath::Sqrt( TMath::Power(energy,2) - TMath::Power(mass,2) );
    };



    // get first filled bin of a histogram
    static Float_t GetFirstFilledX(TH1 * h) {
      for(int b=1; b<=h->GetNbinsX(); b++) {
        if(h->GetBinContent(b)>0) {
          return h->GetBinCenter(b);
        };
      };
      fprintf(stderr,"Tools::GetFirstFilledX called on empty histogram\n");
      return UNDEF;
    };

    // get last filled bin of a histogram
    static Float_t GetLastFilledX(TH1 * h) {
      for(int b=h->GetNbinsX(); b>=1; b--) {
        if(h->GetBinContent(b)>0) {
          return h->GetBinCenter(b);
        };
      };
      fprintf(stderr,"Tools::GetLastFilledX called on empty histogram\n");
      return UNDEF;
    };

    // calculate RMS of a histogram
    static Double_t CalculateRMS(TH1 * h) {
      Double_t bc,bv;
      Double_t numer,denom;
      numer=denom=0;
      for(int bn=1; bn<=h->GetNbinsX(); bn++ ) {
        bc = h->GetBinContent(bn);
        bv = h->GetBinCenter(bn);
        numer += bc*bv*bv;
        denom += bc;
      };
      return TMath::Sqrt(numer/denom);
    };

    // calculate the mean angle of a histogram
    static Double_t MeanAngle(TH1 *h) {
      Double_t sumOfSin = 0.0;
      Double_t sumOfCos = 0.0;
      Double_t sumOfWgt = 0.0;
      // compute sums of sines and cosines, weighted by entries in each bin
      for(int bn=1; bn<=h->GetNbinsX(); bn++ ) {
        Double_t wgt = h->GetBinContent(bn);
        Double_t ang = h->GetBinCenter(bn);
        sumOfSin += wgt*TMath::Sin(ang);
        sumOfCos += wgt*TMath::Cos(ang);
        sumOfWgt += wgt;
      };
      return TMath::ATan2( sumOfSin/sumOfWgt, sumOfCos/sumOfWgt );
    };

    
    // get angle between two vectors
    static Float_t AngleSubtend(TVector3 vA, TVector3 vB) {
      Float_t m = vA.Mag() * vB.Mag();
      if(m>0) return TMath::ACos( vA.Dot(vB) / m );
      return UNDEF;
    };



    // vector projection:
    // returns vA projected onto vB
    static TVector3 Project(TVector3 vA, TVector3 vB) {

      if(fabs(vB.Dot(vB))<0.0001) {
        //fprintf(stderr,"WARNING: Tools::Project to null vector\n");
        return TVector3(0,0,0);
      };

      return vB * ( vA.Dot(vB) / ( vB.Dot(vB) ) );
    };


    // vector rejection: 
    // returns vC projected onto plane transverse to vD
    static TVector3 Reject(TVector3 vC, TVector3 vD) {

      if(fabs(vD.Dot(vD))<0.0001) {
        //fprintf(stderr,"WARNING: Tools::Reject to null vector\n");
        return TVector3(0,0,0);
      };

      return vC - Project(vC,vD);

    };



    // compare integer pair (a1,a2) to (b1,b2), returns true if they're the same,
    // regardless of order
    static Bool_t PairSame(Int_t a1, Int_t a2, Int_t b1, Int_t b2) {
      return (a1==b1 && a2==b2) || (a1==b2 && a2==b1);
    };

    
    static void ApplyProfile(TH2D * histo, Int_t whichAxis) {
      TProfile * prof;
      switch(whichAxis) {
        case 1: prof = histo->ProfileX(); break;
        case 2: prof = histo->ProfileY(); break;
        default: 
          fprintf(stderr,"ERROR: bad whichAxis in Tools::ApplyProfile\n");
          return;
      };
      prof->SetLineColor(kBlack);
      prof->SetLineWidth(3);
      prof->Draw("same");
    };


    // apply a TRegexp to a TString globally (I'm not sure if ROOT has this feature,
    // so I made my own)
    static void GlobalRegexp(TString & str, TRegexp re, TString rep) {
      while(str.Contains(re)) { str(re)=rep; };
    };


    // print out latex code for images
    static void LatexImage(
        TString latexFile,
        TString imgFile,
        TString caption,
        TString label,
        TString format="t",
        Float_t widthScale=0.8
        )
    {
      caption+="."; caption(TRegexp("\\.\\.$"))="."; // add a period (if forgotten)
      gSystem->RedirectOutput(latexFile,"a");
      printf("\\begin{figure}[%s]\n",format.Data());
      printf("\\centering\n");
      printf("\\includegraphics[width=%.1f\\textwidth]{%s}\n",widthScale,imgFile.Data());
      printf("\\caption{%s}\n",caption.Data());
      printf("\\label{%s}\n",label.Data());
      printf("%%~\\\\\n"); // uncomment this and comment out next 2 lines for 'connected' figures
      printf("\\end{figure}\n");
      gSystem->RedirectOutput(0);
    };

    ClassDef(Tools,1);
};

#endif
