#include "Asymmetry.h"

ClassImp(Asymmetry)


Asymmetry::Asymmetry(Binning * binScheme, Int_t binNum) {

  // OPTIONS ////////////
  debug = true;
  enableRellum = true; // (seems ineffective in MLM fit, since absorbed into norm)
  extendMLM = false; // if true, MLM fit will be extended; WARNING: might not be
                     // implemented correctly! Use with CAUTION!
  ///////////////////////


  success = false; // will be true if instance is fully constructed

  // set binning scheme pointer
  BS = binScheme;

  // set up azimuthal modulation
  oaTw = BS->oaTw;
  oaL = BS->oaL;
  oaM = BS->oaM;
  gridDim = BS->gridDim;
  useWeighting = BS->useWeighting;
  if(gridDim==1) { modMax = 1.1; modMin = -modMax; }
  else if(gridDim==2) { modMax = PIe; modMin = -modMax; }
  else {
    fprintf(stderr,"ERROR: bad gridDim\n");
    return;
  };

  // get one-amp fit's modulation name and title
  moduOA = new Modulation(oaTw,oaL,oaM,0,false,Modulation::kLU);
  oaModulationName = moduOA->ModulationName();
  oaModulationTitle = moduOA->ModulationTitle();
  if(debug) {
    printf("Instantiating Asymmetry...\n");
    printf("  ModulationTitle = %s\n",oaModulationTitle.Data());
    printf("  modMin = %f\n",modMin);
    printf("  modMax = %f\n",modMax);
  };

  // set up number of dimensions
  if(BS->dimensions>=1 && BS->dimensions<=3) whichDim = BS->dimensions;
  else {
    fprintf(stderr,"ERROR: bad number of IV bin dimensions\n");
    return;
  };

  // check IV mode
  successIVmode = true;
  if(debug) printf("checking IV mode...\n");
  if(whichDim>=1 && (BS->ivVar[0]<0 || BS->ivVar[0]>Binning::nIV)) successIVmode=false;
  if(whichDim>=2 && (BS->ivVar[1]<0 || BS->ivVar[1]>Binning::nIV)) successIVmode=false;
  if(whichDim>=3 && (BS->ivVar[2]<0 || BS->ivVar[2]>Binning::nIV)) successIVmode=false;
  if(successIVmode) {
    I[0]=BS->ivVar[0];  B[0]=BS->UnhashBinNum(binNum,0);
    I[1]=BS->ivVar[1];  B[1]=BS->UnhashBinNum(binNum,1);
    I[2]=BS->ivVar[2];  B[2]=BS->UnhashBinNum(binNum,2);
  } else {
    fprintf(stderr,"ERROR: bad IV vars\n");
    return;
  };
  if(debug) {
    printf("   ...done\n");
    PrintSettings();
  };


  // set relevant variables for the given IV mode
  for(Int_t d=0; d<3; d++) {
    if(d<whichDim) {
      ivN[d] = BS->IVname[I[d]];
      ivT[d] = BS->IVtitle[I[d]];
      ivMin[d] = BS->minIV[I[d]];
      ivMax[d] = BS->maxIV[I[d]];
    } else {
      ivN[d] = "(unknown)";
      ivT[d] = "(unknown)";
      ivMin[d] = 0;
      ivMax[d] = 0;
    };
  };


  // fixed polarization (for now...)
  ///////////////////////////////////////////////////
  pol = 1; //0.86;
  ///////////////////////////////////////////////////


  // set up binning title and name
  binT = "::";
  binN = "";
  for(int d=0; d<whichDim; d++) {
    binT += "  " + BS->GetBoundStr(B[d],d);
    binN = Form("%s_%s%d",binN.Data(),ivN[d].Data(),B[d]);
  };
  if(debug) printf("binT = %s\nbinN = %s\n",binT.Data(),binN.Data());
  aName = "A" + binN;


  // instantiate histograms etc.
  // - these are mostly used for the one-amplitude ("oa") fit, whereas RooFit data
  //   structures are used for the multi-amplitude fit
  // - see header file for documentation of each object
  // - the histogram of appropriate dimension will be instantiated for use
  // - the unused dimensions will also be instantiated, but never filled; this is done
  //   to prevent null pointers when streaming to an output file (names prefixed with
  //   "nop"=not operational, and they are single bin)
  
  // ivDist
  ivName = Form("ivDist%s",binN.Data());
  if(whichDim == 1) {
    ivTitle = Form("%s distribution %s;%s",
      ivT[0].Data(), binT.Data(),
      ivT[0].Data()
    );
    ivDist1 = new TH1D(ivName,ivTitle,
      iv1Bins,ivMin[0],ivMax[0]
    );
    // nop
    ivDist2 = new TH2D(TString("nop2_"+ivName),TString("nop2"+ivName),1,0,1,1,0,1);
    ivDist3 = new TH3D(TString("nop3_"+ivName),TString("nop3"+ivName),1,0,1,1,0,1,1,0,1);
  }
  else if(whichDim == 2) {
    ivTitle = Form("%s vs. %s %s;%s;%s",
      ivT[1].Data(), ivT[0].Data(), binT.Data(),
      ivT[0].Data(), ivT[1].Data()
    );
    ivDist2 = new TH2D(ivName,ivTitle,
      iv2Bins,ivMin[0],ivMax[0],
      iv2Bins,ivMin[1],ivMax[1]
    );
    // nop
    ivDist1 = new TH1D(TString("nop1_"+ivName),TString("nop1"+ivName),1,0,1);
    ivDist3 = new TH3D(TString("nop3_"+ivName),TString("nop3"+ivName),1,0,1,1,0,1,1,0,1);
  }
  else if(whichDim == 3) {
    ivTitle = Form("%s vs. %s vs. %s %s;%s;%s;%s",
      ivT[2].Data(), ivT[1].Data(), ivT[0].Data(), binT.Data(),
      ivT[0].Data(), ivT[1].Data(), ivT[2].Data()
    );
    ivDist3 = new TH3D(ivName,ivTitle,
      iv3Bins,ivMin[0],ivMax[0],
      iv3Bins,ivMin[1],ivMax[1],
      iv3Bins,ivMin[2],ivMax[2]
    );
    // nop
    ivDist1 = new TH1D(TString("nop1_"+ivName),TString("nop1"+ivName),1,0,1);
    ivDist2 = new TH2D(TString("nop2_"+ivName),TString("nop2"+ivName),1,0,1,1,0,1);
  };
  //if(debug) printf("built %s\n\t%s\n",ivName.Data(),ivTitle.Data());


  // modDist and modBinDist
  modName = Form("modDist%s",binN.Data());
  modTitle = Form("%s distribution %s",oaModulationTitle.Data(),binT.Data());
  if(gridDim==1) {
    modTitle += ";" + oaModulationTitle;
    modDist = new TH1D(modName,modTitle,iv1Bins,modMin,modMax);
    // nop
    modDist2 = new TH2D(TString("nop_"+modName),TString("nop_"+modName),1,0,1,1,0,1);
  } else {
    modTitle += ";#phi_{R};#phi_{h}";
    modDist2 = new TH2D(modName,modTitle,iv2Bins,modMin,modMax,iv2Bins,modMin,modMax);
    // nop
    modDist = new TH1D(TString("nop_"+modName),TString("nop_"+modName),1,0,1);
  };
  //if(debug) printf("built %s\n\t%s\n",modName.Data(),modTitle.Data());

  for(int m=0; m<nModBins; m++) {
    modBinName = Form("%s_bin_%d",modName.Data(),m);
    modBinTitle = Form("bin %d %s",m,modTitle.Data());
    if(gridDim==1) {
      modBinDist[m] = new TH1D(modBinName,modBinTitle,iv1Bins,modMin,modMax);
    } else {
      // nop
      modBinDist[m] = new TH1D(
        TString("nop_"+modBinName),TString("nop_"+modBinName),1,0,1);
    };
  };
  for(int mmH=0; mmH<nModBins2; mmH++) {
    for(int mmR=0; mmR<nModBins2; mmR++) {
      modBinName = Form("%s_bin_H%d_R%d",modName.Data(),mmH,mmR);
      modBinTitle = Form("bin (H%d,R%d) %s",mmH,mmR,modTitle.Data());
      modBinTitle += ";#phi_{R};#phi_{h}";
      if(gridDim==2) {
        modBinDist2[mmH][mmR] = new TH2D(
          modBinName,modBinTitle,
          iv2Bins,modMin,modMax,iv2Bins,modMin,modMax);
      } else {
        //nop
        modBinDist2[mmH][mmR] = new TH2D(
          TString("nop_"+modBinName),TString("nop_"+modBinName),1,0,1,1,0,1);
      };
    };
  };


  // IVvsModDist (only for 1D binning and 1D modulation)
  IVvsModName = Form("IVvsModDist%s",binN.Data());
  IVvsModTitle = Form("%s vs. %s %s;%s;%s",
    ivT[0].Data(),oaModulationTitle.Data(),binT.Data(),
    oaModulationTitle.Data(),ivT[0].Data()
  );
  if(whichDim==1 && gridDim==1) {
    IVvsModDist = new TH2D(IVvsModName,IVvsModTitle,
      iv1Bins,modMin,modMax,
      iv1Bins,ivMin[0],ivMax[0]
    );
  } else {
    //nop
    IVvsModDist = new TH2D(TString("nop_"+IVvsModName),TString("nop_"+IVvsModName),
      1,0,1,1,0,1);
  };


  // aziDist
  for(int s=0; s<nSpin; s++) {
    aziName[s] = Form("aziDist_%s%s",SpinName(s).Data(),binN.Data());
    aziTitle[s] = Form("%s binned distribution :: %s %s",
      oaModulationTitle.Data(),SpinTitle(s).Data(),binT.Data()
    );
    if(gridDim==1) {
      aziTitle[s] += ";" + oaModulationTitle;
      aziDist[s] = new TH1D(aziName[s],aziTitle[s],nModBins,modMin,modMax);
      //nop
      aziDist2[s] = new TH2D(TString("nop_"+aziName[s]),TString("nop_"+aziName[s]),
        1,0,1,1,0,1);
    } else {
      aziTitle[s] += ";#phi_{R};#phi_{h}"; // PhiR is horizontal, PhiH is vertical
      aziDist2[s] = new TH2D(aziName[s],aziTitle[s],
        nModBins2,modMin,modMax,nModBins2,modMin,modMax);
      //nop
      aziDist[s] = new TH1D(TString("nop_"+aziName[s]),TString("nop_"+aziName[s]),
        1,0,1);
    };
  };

  // yieldDist
  yieldDist = new TH1D(
    TString("yieldDist"+binN),
    TString("yield distribution :: "+binT),
    2,0,2
  );

  // kfDist
  kfLB = 0;
  kfUB = 2;
  kfDist = new TH1D(
    TString("kfDist"+binN),
    TString("K(y) distribution :: "+binT),
    300,kfLB,kfUB);


  // asymGr
  asymName = Form("asym%s",binN.Data());
  if(gridDim==1) {
    asymTitle = Form("%s asymmetry %s;%s;asymmetry",
      oaModulationTitle.Data(), binT.Data(),  oaModulationTitle.Data()
    );
    asymGr = new TGraphErrors();
    asymGr->SetName(asymName);
    asymGr->SetTitle(asymTitle);
    //nop
    asymGr2 = new TGraph2DErrors(); asymGr2->SetTitle(TString("nop_"+asymTitle));
    asymGr2hist = new TH2D(TString("nop_hist"+asymName),TString("nop_hist"+asymName),
      1,0,1,1,0,1);
  } else {
    asymTitle = Form("%s asymmetry %s;#phi_{R};#phi_{h};asymmetry",
      oaModulationTitle.Data(), binT.Data()
    );
    asymGr2 = new TGraph2DErrors();
    asymGr2->SetName(asymName);
    asymGr2->SetTitle(asymTitle);
    asymGr2hist = new TH2D(TString("hist"+asymName),asymTitle,
      nModBins2,modMin,modMax,nModBins2,modMin,modMax);
    //nop
    asymGr = new TGraphErrors(); asymGr->SetTitle(TString("nop_"+asymTitle));
  };

  // initialize kinematic variables
  ResetVars();
  nEvents = 0;


  // initialize RooFit variables and parameters and data set containers
  // ------------------------------------------------------------------
  // - build category to index spins
  rfSpinCateg = new RooCategory("rfSpinCateg","rfSpinCateg");
  for(int s=0; s<nSpin; s++) {
    rfSpinName[s] = "rfSpin" + SpinName(s);
    rfSpinCateg->defineType(rfSpinName[s]);
  };
  // - event vars
  rfPhiH = new RooRealVar("rfPhiH","#phi_{h}",-PIe,PIe);
  rfPhiR = new RooRealVar("rfPhiR","#phi_{R}",-PIe,PIe);
  rfTheta = new RooRealVar("rfTheta","#theta",-PIe,PIe);
  rfWeight = new RooRealVar("rfWeight","P_{h}^{T}/M_{h}",0,10);

  rfVars = new RooArgSet(*rfPhiH,*rfPhiR,*rfTheta);
  rfVars->add(*rfWeight);
  rfVars->add(*rfSpinCateg);

  // - amplitudes (fit parameters)
  rfParamRange = 0.5;
  for(int aa=0; aa<nAmp; aa++) {
    rfAname[aa] = Form("A%d",aa);
    rfA[aa] = new RooRealVar(rfAname[aa],rfAname[aa],-rfParamRange,rfParamRange);
  };
  nAmpUsed = 0;

  for(int dd=0; dd<nDparam; dd++) {
    rfDname[dd] = Form("D%d",dd);
    rfD[dd] = new RooRealVar(rfDname[dd],rfDname[dd],-1,1);
  };
  nDparamUsed = 0;


  // - yield factor (proportional to actual yield, for extended fit only)
  rfYield[sP] = new RooRealVar("rfYieldP","YP",0,1e10);
  rfYield[sM] = new RooRealVar("rfYieldM","YM",0,1e10);

  // - polarization and rellum
  rfPol = new RooRealVar("rfPol","P",0,1);
  rfRellum = new RooRealVar("rfRellum","R",0,3);

  // - data sets for each spin
  rfData = new RooDataSet(
    TString("rfData"+binN),TString("rfData"+binN),
    *rfVars,
    rfWeight->GetName()
  );


  if(debug) {
    printf("whichDim = %d\n",whichDim);
    printf("one-amp fit modulation = %s\n",moduOA->StateTitle().Data());
    printf("--> Asymmetry instantiated.\n");
  };

  success = true;
};



// fill all the plots; to be called in an event loop
// -- assumes that EventTree::GetEvent(Int_t) has been called
// -- returns true if filled successfully, or false
//    if not (which usually happens if it's the wrong bin,
//    or if one of the required kinematic variables has
//    a bad value)
Bool_t Asymmetry::AddEvent(EventTree * ev) {

  // set kinematic vars
  Mh = ev->Mh;
  x = ev->x;
  z = ev->Zpair;
  PhiH = ev->PhiH;
  PhiR = ev->PhiR;
  PhPerp = ev->PhPerp;
  Ph = ev->Ph;
  Q2 = ev->Q2;
  theta = ev->theta;

  // testing single-hadron phiH definition
  /*
  PhiH = ev->GetDihadronObj()->GetSingleHadronPhiH(qA);
  z = ev->Z[qA];
  */


  // set iv variable
  for(int d=0; d<whichDim; d++) {
    switch(I[d]) {
      case Binning::vM: iv[d] = Mh; break;
      case Binning::vX: iv[d] = x; break;
      case Binning::vZ: iv[d] = z; break;
      case Binning::vPt: iv[d] = PhPerp; break;
      case Binning::vPh: iv[d] = Ph; break;
      case Binning::vQ: iv[d] = Q2; break;
      default: 
        fprintf(stderr,
          "ERROR: Asymmetry::AddEvent does not understand I[%d]=%d\n",d,I[d]);
        return false;
    };
  };


  // check if we're in the proper bin; if not, simply return silently
  for(int d=0; d<whichDim; d++) {
    if(B[d] != BS->GetBin(I[d],iv[d])) return false;
  };


  // check to make sure kinematics are defined (if they're not, something else
  // probably set them to UNDEF)
  for(int d=0; d<whichDim; d++) { 
    if(iv[d]<-8000) return KickEvent(TString(ivN[d]+" out of range"),iv[d]);
  };
  if(PhiH<-PIe || PhiH>PIe) return KickEvent("PhiH out of range",PhiH);
  if(PhiR<-PIe || PhiR>PIe) return KickEvent("PhiR out of range",PhiR);
  if(PhPerp<-8000) return KickEvent("PhPerp out of range",PhPerp);
  if(Ph<-8000) return KickEvent("Ph out of range",Ph);
  if(Q2<-8000) return KickEvent("Q2 out of range",Q2);
  if(theta<0 || theta>PI) return KickEvent("theta out of range",theta);

  // set spin state
  spinn = ev->SpinState();
  if(spinn<0 || spinn>=nSpin) return false;

  // get kinematic factor
  kf = EvalKinematicFactor(ev);
  if(kf<kfLB || kf>kfUB) return KickEvent("KF out of range",kf);

  // evaluate modValOA
  if(gridDim==1) modValOA = moduOA->Evaluate(PhiR, PhiH, theta);
  else modValOA = UNDEF; // not needed

  // set weight (returns 1, unless weighting for G1perp)
  weight = EvalWeight();
  // weight *= kf; // weight events with kinematic factor

  // set RooFit vars
  rfPhiH->setVal(PhiH);
  rfPhiR->setVal(PhiR);
  rfWeight->setVal(weight);
  rfTheta->setVal(theta);
  rfSpinCateg->setLabel(rfSpinName[spinn]);
  rfData->add(*rfVars,rfWeight->getVal());


  // fill plots
  // ----------

  if(gridDim==1) {
    aziDist[spinn]->Fill(modValOA,weight);
    modbin = aziDist[sP]->FindBin(modValOA);
    if(modbin>=1 && modbin<=nModBins) {
      modBinDist[modbin-1]->Fill(modValOA,weight);
    } else {
      fprintf(stderr,"ERROR: modValOA bin not found for modValOA=%f\n",modValOA);
      return false;
    };
    modDist->Fill(modValOA,weight);
  } 
  else {
    aziDist2[spinn]->Fill(PhiR,PhiH,weight);
    modbinR = aziDist2[sP]->GetXaxis()->FindBin(PhiR);
    modbinH = aziDist2[sP]->GetYaxis()->FindBin(PhiH);
    if(modbinR>=1 && modbinR<=nModBins2 && modbinH>=1 && modbinH<=nModBins2) {
      modBinDist2[modbinH-1][modbinR-1]->Fill(PhiR,PhiH,weight);
    } else {
      fprintf(stderr,"ERROR: 2d bin not found (phiH=%f phiR=%f)\n",PhiH,PhiR);
      return false;
    };
    modDist2->Fill(PhiR,PhiH,weight);
  };

  switch(whichDim) {
    case 1: ivDist1->Fill(iv[0]); break;
    case 2: ivDist2->Fill(iv[0],iv[1]); break;
    case 3: ivDist3->Fill(iv[0],iv[1],iv[2]); break;
  };

  if(whichDim==1 && gridDim==1) IVvsModDist->Fill(modValOA,iv[0],weight);

  yieldDist->Fill(spinn);
  kfDist->Fill(kf);
  
  // increment event counter
  nEvents++;
  return true;
};
  


// perform a fit using TGraphErrors::Fit()
// - if gridDim==1, it is done using a 1D grid, with bins of modulation(phi)
//   - only one amplitude can be fitted
// - if gridDim==2, it is done using a 2D grid, with bins of (PhiR,PhiH)
//   - either one amplitude or multiple amplitudes can be fitted, but the
//     multi-amplitude fit implementation is limited
void Asymmetry::FitAsymGraph() {
  
  if(debug) {
    printf("calculate asymmetries for:\n");
    PrintSettings(); 
  };

  // compute asymmetry for each modulation bin
  pointCnt = 0;
  if(gridDim==1) {
    for(int m=1; m<=nModBins; m++) {
      yL = aziDist[sP]->GetBinContent(m);
      yR = aziDist[sM]->GetBinContent(m);
      SetAsymGrPoint(m);
    };
  } else {
    for(int mmH=1; mmH<=nModBins2; mmH++) {
      for(int mmR=1; mmR<=nModBins2; mmR++) {
        yL = aziDist2[sP]->GetBinContent(mmR,mmH);
        yR = aziDist2[sM]->GetBinContent(mmR,mmH);
        SetAsymGrPoint(mmH,mmR);
      };
    };
  };


  // build fit function
  fitFuncName = "fit_"+asymName;
  if(gridDim==1) {
    fitFunc = new TF1(fitFuncName,"[0]+[1]*x",modMin,modMax);
    fitFunc->SetParName(0,"B");
    fitFunc->SetParName(1,"A_{LU}");
  } else {
    if(!enablePW) {
      fitFunc2 = new TF2(fitFuncName,fitFunc2formu,modMin,modMax,modMin,modMax);
    }
    else {
      fprintf(stderr,"ERROR: cannot perform FitAsymGraph with enablePW==true\n");
      return;
    };
  };


  // fit asymmetry
  if(gridDim==1) {
    fitFunc->FixParameter(0,0); // fix fit offset to 0
    asymGr->Fit(fitFunc,"Q","",modMin,modMax);
  } else {
    asymGr2->Fit(fitFunc2,"Q","");
  };

};


// set fit mode, which defines what combination of amplitude we will fit in 
// a multi-amplitude fit
// - this function must be called before fitting!
// - also sets the relative luminosity
void Asymmetry::SetFitMode(Int_t fitMode) {

  // build asymmetry modulation paramaterization "asymFormu" 
  // = sum { amplitude_i * modulation_i }
  nAmpUsed = 0;
  enablePW = false;
  switch(fitMode) {
    case 0: // test one-amp modulation
      this->FormuAppend(oaTw,oaL,oaM);
      break;
    case 1: // test linear combination of e(x) and g1perp modulations
      this->FormuAppend(3,1,1);
      this->FormuAppend(2,1,1);
      break;
    case 2: // test single partial wave with one-amp modulation
      enablePW = true;
      this->FormuAppend(oaTw,oaL,oaM);
      break;
    case 3: // test 2 partial waves
      enablePW = true;
      this->FormuAppend(2,1,1);
      this->FormuAppend(3,1,1);
      break;
    case 4: // three L=1 modulations (for DNP2019)
      this->FormuAppend(3,0,0);
      this->FormuAppend(2,1,1);
      this->FormuAppend(3,1,1);
      break;
    case 5: // all four L=1 modulations
      this->FormuAppend(3,0,0);
      this->FormuAppend(2,1,1);
      this->FormuAppend(3,1,1);
      this->FormuAppend(3,1,-1);
      break;
    case 6: // modulations up to L=2 with nonegligble overlap with |1,1>_2
      this->FormuAppend(3,0,0); // grey
      this->FormuAppend(2,1,1); // red
      this->FormuAppend(3,1,1); // blue
      this->FormuAppend(3,1,-1); // purple
      this->FormuAppend(2,2,2); // green
      this->FormuAppend(3,2,2); // cyan
      break;
    case 7: // partial waves for G1perp SP
      enablePW = true;
      this->FormuAppend(3,0,0); // grey
      this->FormuAppend(2,1,1); // red
      this->FormuAppend(3,1,1); // blue
      this->FormuAppend(3,1,-1); // purple
      this->FormuAppend(3,2,0); // green
      this->FormuAppend(2,2,2); // cyan
      this->FormuAppend(3,2,2); // yellow
      break;
    case 8: // partial waves for G1perp PP
      enablePW = true;
      this->FormuAppend(3,1,0);
      this->FormuAppend(2,2,1);
      this->FormuAppend(3,2,1);
      this->FormuAppend(3,2,-1);
      break;
    case 110:
      this->FormuAppend(3,1,1);
      this->DenomAppend(2,2,0,0); // tw2 |2,0> UU,T
      break;
    case 111:
      this->FormuAppend(2,1,1);
      this->DenomAppend(2,2,0,0); // tw2 |2,0> UU,T
      break;
    case 120:
      this->FormuAppend(2,1,1);
      this->FormuAppend(3,1,1);
      this->DenomAppend(2,2,0,0); // tw2 |2,0> UU,T
      break;
    case 130:
      this->FormuAppend(3,0,0);
      this->FormuAppend(2,1,1);
      this->FormuAppend(3,1,1);
      this->DenomAppend(2,2,0,0); // tw2 |2,0> UU,T
      break;
    case 140:
      this->FormuAppend(3,0,0);
      this->FormuAppend(2,1,1);
      this->FormuAppend(3,1,1);
      this->FormuAppend(3,1,-1);
      this->DenomAppend(2,2,0,0); // tw2 |2,0> UU,T
      break;
    case 1000:
      this->FormuAppend(2,1,1,0,Modulation::kLL); // double-spin asym
      this->FormuAppend(3,1,1,0,Modulation::kLL);
      break;
    case 1001:
      enablePW = true;
      this->FormuAppend(2,1,1,0,Modulation::kLL); // double-spin asym
      this->FormuAppend(2,2,1,0,Modulation::kLL); // with tw2 PWs
      this->FormuAppend(2,2,2,0,Modulation::kLL);
      break;
    default:
      fprintf(stderr,"ERROR: bad fitMode; using G1perp default\n");
      this->FormuAppend(2,1,1);
      break;
  };


  // COMPUTE RELATIVE LUMINOSITY
  spinbin = yieldDist->FindBin(sP);
  rNumer = yieldDist->GetBinContent(spinbin);
  spinbin = yieldDist->FindBin(sM);
  rDenom = yieldDist->GetBinContent(spinbin);
  if(rDenom>0) {
    // -- relative luminosity
    rellum = enableRellum ? rNumer/rDenom : 1;
    // -- relative luminosity statistical uncertainty (assume poissonian yields)
    rellumErr = sqrt( rNumer * (rNumer+rDenom) / pow(rDenom,3) );
  } else {
    rellum = 0;
    rellumErr = 0;
    fprintf(stderr,"WARNING: rellum denominator==0, abort asym calculation for this bin\n");
    return;
  };
  printf("rellum = %f / %f =  %.3f  +-  %f\n",rNumer,rDenom,rellum,rellumErr);
  
};



// set new asymGr point and error
// -- called by FitAsymGraph() for each modulation bin
// -- need to have yL, yR, and rellum set before calling
// -- modBin_ and modBin2_ are used to address modBinDist for getting mean modulation
//    for this modulation bin
void Asymmetry::SetAsymGrPoint(Int_t modBin_, Int_t modBin2_) {

  asymNumer = yL - (rellum * yR);
  asymDenom = yL + (rellum * yR);

  if(asymDenom>0) {
    // compute asymmetry value
    asymVal = (1.0/pol) * (asymNumer/asymDenom);

    // compute asymmetry statistical error
    // -- full formula
    asymErr = ( 2 * rellum * sqrt( yL*pow(yR,2) + yR*pow(yL,2) ) ) / 
              ( pol * pow(yL+rellum*yR,2) );
    // -- compare to simple formula (assumes asym*pol<<1 and R~1)
    //printf("difference = %f\n",asymErr - 1.0 / ( pol * sqrt(yL+yR) ));

    // compute azimuthal modulation value and error
    if(gridDim==1) {
      modVal = modBinDist[modBin_-1]->GetMean(); // use modulation bin's mean
      modErr = 0; // for now (TODO)
    } else {
      // using modBinDist2[mmH-1][mmR-1]; x-axis is PhiR; y-axis is PhiH
      modValR = modBinDist2[modBin_-1][modBin2_-1]->GetMean(1); // use bin mean
      modValH = modBinDist2[modBin_-1][modBin2_-1]->GetMean(2);
      //modValR = aziDist2[0]->GetXaxis()->GetBinCenter(modBin2_); // use bin center
      //modValH = aziDist2[0]->GetYaxis()->GetBinCenter(modBin_);
      modErrR = 0; // for now (TODO)
      modErrH = 0; // for now (TODO)
    };
    

    if(gridDim==1) {
      asymGr->SetPoint(pointCnt,modVal,asymVal);
      asymGr->SetPointError(pointCnt,modErr,asymErr);
    } else {
      asymGr2->SetPoint(pointCnt,modValR,modValH,asymVal);
      asymGr2->SetPointError(pointCnt,modErrR,modErrH,asymErr);
      asymGr2hist->SetBinContent(modBin2_,modBin_,asymVal);
    };
    pointCnt++;
  };
};



// use unbined MLM method to fit asymmetries, using RooFit Minuit to
// minimize the -log likelihood
void Asymmetry::FitAsymMLM() {

  // append polarization factor to asymFormu
  asymFormu = "rfPol*("+asymFormu+")";

  // append unpolarized denominator, if D_1 is expanded in partial waves
  // (for systematic uncertainty study from unmeasured/non-orthogonal D_1 pp-wave)
  if(nDparamUsed>0) asymFormu += "/(" + denomFormu + ")";
  /*
  if(nDparamUsed==1) { // DEPRECATED (use for "b-scan")
    rfD[0]->setVal(DparamVal);
    rfD[0]->setConstant(kTRUE);
  */
      
  // rellum factors
  rellumFactor[sP] = "rfRellum/(rfRellum+1)";
  rellumFactor[sM] = "1/(rfRellum+1)";


  // -- build full PDF ( = rellumFactor * ( 1 +/- pol*asymFormu ) for each spin
  for(int s=0; s<nSpin; s++) {
    rfPdfFormu[s] = "(" + rellumFactor[s] + ")*(1" + SpinSign(s) + asymFormu + ")";

    // build list of variables and parameters; we *only* want variables that
    // are actually being used in the PDF
    rfParams[s] = new RooArgSet();
    if(rfPdfFormu[s].Contains("rfPhiH")) rfParams[s]->add(*rfPhiH);
    if(rfPdfFormu[s].Contains("rfPhiR")) rfParams[s]->add(*rfPhiR);
    if(rfPdfFormu[s].Contains("rfTheta")) rfParams[s]->add(*rfTheta);
    for(int aa=0; aa<nAmpUsed; aa++) rfParams[s]->add(*rfA[aa]);
    for(int dd=0; dd<nDparamUsed; dd++) rfParams[s]->add(*rfD[dd]);
    rfParams[s]->add(*rfPol);
    rfParams[s]->add(*rfRellum);

    // build pdf
    rfPdf[s] = new RooGenericPdf(
      TString("rfModel" + SpinName(s)),
      TString("rfModel " + SpinTitle(s)),
      rfPdfFormu[s],
      *rfParams[s]
    );
    rfPdfExtended[s] = new RooExtendPdf(
      TString("rfPdf" + SpinName(s)),
      TString("rfPdf " + SpinTitle(s)),
      *rfPdf[s],
      *rfYield[s]
    );
  };


  // build simultanous PDF 
  rfSimPdf = new RooSimultaneous("rfSimPdf","rfSimPdf",*rfSpinCateg);
  for(int s=0; s<nSpin; s++) {
    if(extendMLM) rfSimPdf->addPdf(*rfPdfExtended[s],rfSpinName[s]);
    else          rfSimPdf->addPdf(*rfPdf[s],rfSpinName[s]);
  };

  // -log likelihood
  rfNLL = new RooNLLVar("rfNLL","rfNLL",*rfSimPdf,*rfData);
  for(int aa=0; aa<nAmp; aa++) rfNLLplot[aa] = new RooPlot();


  // fix polarization and rellum PDF parameters to their values
  rfPol->setVal(pol);
  rfRellum->setVal(rellum);
  rfPol->setConstant(true);
  rfRellum->setConstant(true);


  // fit simultaneous PDF to combined data
  // ----------------------------------------------
  Tools::PrintSeparator(70,"=");
  printf("BEGIN FIT\n");
  Tools::PrintSeparator(70,"=");

  // get number of available threads; if this method fails, set number of threads to 1
  nThreads = (Int_t) std::thread::hardware_concurrency();
  if(nThreads<1) nThreads=1;
  printf("---- fit with %d parallel threads\n",nThreads);

  // perform the fit
  if(extendMLM) {
    rfSimPdf->fitTo(*rfData, RooFit::Extended(kTRUE), RooFit::Save(kTRUE));
  } else {
    rfSimPdf->fitTo(*rfData, RooFit::NumCPU(nThreads), RooFit::Save());
  }

  // get -log likelihood
  for(int aa=0; aa<nAmpUsed; aa++) {
    rfNLLplot[aa] = rfA[aa]->frame(
      RooFit::Range(-rfParamRange,rfParamRange),
      RooFit::Title(TString("-log(L) scan vs. A"+TString::Itoa(aa,10)))
    );
    rfNLL->plotOn(
      rfNLLplot[aa],
      RooFit::ShiftToZero()
    );
  };

  Tools::PrintSeparator(70,"=");

  // print fit results
  /*
  Tools::PrintTitleBox("ROOFIT RESULTS");
  this->PrintSettings();
  rfResult->Print("v");
  Tools::PrintSeparator(70,"=");
  */

};


// append a modulation to build a multi-amplitude fit formula
void Asymmetry::FormuAppend(Int_t TW, Int_t L, Int_t M,
  Int_t lev, Int_t polarization) {
  if(nAmpUsed>=nAmp) {
    fprintf(stderr,"ERROR: nAmpUsed > nAmp (the max allowed value)\n");
    return;
  };

  if(nAmpUsed==0) {
    asymFormu = "";
    fitFunc2formu = "";
  }
  else {
    asymFormu += "+";
    fitFunc2formu += "+";
  };

  modu[nAmpUsed] = new Modulation(TW, L, M, lev, enablePW, polarization);

  asymFormu += "A"+TString::Itoa(nAmpUsed,10)+"*"+modu[nAmpUsed]->FormuRF();
  fitFunc2formu += "["+TString::Itoa(nAmpUsed,10)+"]*"+modu[nAmpUsed]->Formu();

  rfA[nAmpUsed]->SetTitle(modu[nAmpUsed]->AsymmetryTitle());

  nAmpUsed++;

};


// append modulations to the asymmetry denominator
// - this is for studying impact of sigma_UU modulations not orthogonal to 1
void Asymmetry::DenomAppend(Int_t TW, Int_t L, Int_t M, Int_t lev) {
  if(nDparamUsed>=nDparam) {
    fprintf(stderr,"ERROR: nDparamUsed > nDparam (the max allowed value)\n");
    return;
  };

  if(nDparamUsed==0) denomFormu = "1";
  denomFormu += "+";

  moduD[nDparamUsed] = new Modulation(TW, L, M, lev, true, Modulation::kUU);

  denomFormu += "D"+TString::Itoa(nDparamUsed,10)+"*"+moduD[nDparamUsed]->FormuRF();
  rfD[nDparamUsed]->SetTitle(TString("D"+moduD[nDparamUsed]->StateTitle()));

  nDparamUsed++;

};


// g1perp PhPerp/Mh weighting
Float_t Asymmetry::EvalWeight() {
  Float_t wt;
  if(useWeighting) { 
    wt = Mh>0 ? PhPerp/Mh : 0; 
  }
  else wt = 1;
  return wt;
};


// if e(x) modulation, return W(y)/A(y)
// if G1perp modulation, return C(y)/A(y)
// see EventTree::GetKinematicFactor() for definitions
Float_t Asymmetry::EvalKinematicFactor(EventTree * ev) {
  
  kfA = ev->GetKinematicFactor('A');
  kfC = ev->GetKinematicFactor('C');
  kfW = ev->GetKinematicFactor('W');

  if(oaTw==3 && oaM==1) return kfW / kfA;
  else if(oaTw==2 && oaM==1) return kfC / kfA;
  else return 1;

};

 
void Asymmetry::ResetVars() {
  Mh = UNDEF;
  x = UNDEF;
  z = UNDEF;
  PhiH = UNDEF;
  PhiR = UNDEF;
  PhPerp = UNDEF;
  Ph = UNDEF;
  Q2 = UNDEF;
  theta = UNDEF;
  spinn = UNDEF;
  kfA = UNDEF;
  kfC = UNDEF;
  kfW = UNDEF;
  for(int d=0; d<3; d++) iv[d]=UNDEF;
};


void Asymmetry::PrintSettings() {
  for(Int_t d=0; d<whichDim; d++) printf("  %s bin %d (I[%d]=%d B[%d]=%d)\n",
    (BS->IVname[I[d]]).Data(),B[d],
    d,I[d],d,B[d]
  );
};


// stream pertinent data structures to a TFile
void Asymmetry::StreamData(TFile * tf) {

  tf->cd();
  tf->mkdir(aName);
  tf->cd(TString(aName));

  appName = this->AppFileName(tf);

  printf("writing plots for: "); this->PrintSettings();

  switch(whichDim) {
    case 1: 
      objName = appName + ivDist1->GetName(); ivDist1->Write(objName); 
      break;
    case 2: 
      objName = appName + ivDist2->GetName(); ivDist2->Write(objName); 
      break;
    case 3: 
      objName = appName + ivDist3->GetName(); ivDist3->Write(objName); 
      break;
  };

  if(gridDim==1) {
    objName = appName + modDist->GetName(); modDist->Write(objName);
    for(Int_t m=0; m<nModBins; m++) {
      objName = appName + modBinDist[m]->GetName(); modBinDist[m]->Write(objName);
    };
    if(whichDim==1) {
      objName = appName + IVvsModDist->GetName(); IVvsModDist->Write(objName);
    }; 
    for(int s=0; s<nSpin; s++) {
      objName = appName + aziDist[s]->GetName(); aziDist[s]->Write(objName);
    };
  } else {
    objName = appName + modDist2->GetName(); modDist2->Write(objName);
    for(Int_t mmH=0; mmH<nModBins2; mmH++) {
      for(Int_t mmR=0; mmR<nModBins2; mmR++) {
        objName = appName + modBinDist2[mmH][mmR]->GetName();
        modBinDist2[mmH][mmR]->Write(objName);
      };
    };
    for(int s=0; s<nSpin; s++) {
      objName = appName + aziDist2[s]->GetName(); aziDist2[s]->Write(objName);
    };
  };

  objName = appName + yieldDist->GetName(); yieldDist->Write(objName);
  objName = appName + kfDist->GetName(); kfDist->Write(objName);

  objName = appName + rfData->GetName(); rfData->Write(objName);

  tf->cd("/");
  printf("done\n");
};


// append pertinent data structures a TFile to this current instance
void Asymmetry::AppendData(TFile * tf) {

  appName = "/" + aName + "/" + this->AppFileName(tf);
  printf("reading plots for: "); this->PrintSettings();

  switch(whichDim) {
    case 1: 
      objName = appName + ivDist1->GetName();
      appDist1 = (TH1D*) tf->Get(objName);
      ivDist1->Add(appDist1); 
      break;
    case 2: 
      objName = appName + ivDist2->GetName();
      appDist2 = (TH2D*) tf->Get(objName);
      ivDist2->Add(appDist2); 
      break;
    case 3: 
      objName = appName + ivDist3->GetName();
      appDist3 = (TH3D*) tf->Get(objName);
      ivDist3->Add(appDist3); 
      break;
  };

  if(gridDim==1) {
    objName = appName + modDist->GetName();
    appDist1 = (TH1D*) tf->Get(objName);
    modDist->Add(appDist1);
    for(Int_t m=0; m<nModBins; m++) {
      objName = appName + modBinDist[m]->GetName();
      appDist1 = (TH1D*) tf->Get(objName);
      modBinDist[m]->Add(appDist1);
    };
    if(whichDim==1) {
      objName = appName + IVvsModDist->GetName();
      appDist2 = (TH2D*) tf->Get(objName);
      IVvsModDist->Add(appDist2);
    };
    for(int s=0; s<nSpin; s++) {
      objName = appName + aziDist[s]->GetName();
      appDist1 = (TH1D*) tf->Get(objName);
      aziDist[s]->Add(appDist1);
    };
  } else {
    objName = appName + modDist2->GetName();
    appDist2 = (TH2D*) tf->Get(objName);
    modDist2->Add(appDist2);
    for(Int_t mmH=0; mmH<nModBins2; mmH++) {
      for(Int_t mmR=0; mmR<nModBins2; mmR++) {
        objName = appName + modBinDist2[mmH][mmR]->GetName();
        appDist2 = (TH2D*) tf->Get(objName);
        modBinDist2[mmH][mmR]->Add(appDist2);
      };
    };
    for(int s=0; s<nSpin; s++) {
      objName = appName + aziDist2[s]->GetName();
      appDist2 = (TH2D*) tf->Get(objName);
      aziDist2[s]->Add(appDist2);
    };
  };

  objName = appName + yieldDist->GetName(); 
  appDist1 = (TH1D*) tf->Get(objName);
  yieldDist->Add(appDist1);

  objName = appName + kfDist->GetName(); 
  appDist1 = (TH1D*) tf->Get(objName);
  kfDist->Add(appDist1);

  objName = appName + rfData->GetName();
  appRooDataSet = (RooDataSet*) tf->Get(objName);
  rfData->append(*appRooDataSet); 
  
  tf->cd("/");
};


TString Asymmetry::AppFileName(TFile * tf) {
  TString retstr = TString(tf->GetName());
  retstr(TRegexp("^.*/")) = "";
  retstr(TRegexp("^spin.")) = "";
  retstr(TRegexp(".root$")) = "";
  retstr(TRegexp(".hipo$")) = "";
  retstr = "stream_" + retstr + "_";
  return retstr;
};


Bool_t Asymmetry::KickEvent(TString reason,Float_t badValue) {
  fprintf(stderr,"kick event, since %s (value=%f)\n",reason.Data(),badValue);
  return false;
};


Asymmetry::~Asymmetry() {
  if(rfData) delete rfData;
};
