R__LOAD_LIBRARY(DiSpin)

//#include "Constants.h"
//#include "Modulation.h"

void Draw3d(TH3D * dd, Int_t whichProj);

// weightSetting: 
// 0 = use acceptance from infile (ortho.root)
// 1 = uniform across all variables
// 2+ = see switch statement

void Orthogonality(Int_t binNum=0, Int_t weightSetting=0,
  TString infileN="ortho.root", Bool_t printLatex=false) {

  
  // OPTIONS
  ///////////////////
  Int_t polarizationSetting = Modulation::kUU;
  Int_t LMAX = 2;
  Bool_t useModulationTitle = true; // if true, print functions instaed of kets
  ///////////////////

  Bool_t enableLegendre = 0;
  if(polarizationSetting==Modulation::kUU) enableLegendre = true;

  // open data hist
  TFile * infile = new TFile(infileN,"READ");
  TString dataDistN = Form("d3_bin%d",binNum);
  TH3D * dataDist = (TH3D*) infile->Get(dataDistN);
  dataDist->SetTitle("data distribution");

  int f,g,h,r,t;

  // get number of bins and bin widths
  Int_t nbinsR = dataDist->GetNbinsX();
  Float_t minR = dataDist->GetXaxis()->GetXmin();
  Float_t maxR = dataDist->GetXaxis()->GetXmax();
  //Float_t rangeR = maxR - minR;
  //Float_t widthR = dataDist->GetXaxis()->GetBinWidth(0);

  Int_t nbinsH = dataDist->GetNbinsY();
  Float_t minH = dataDist->GetYaxis()->GetXmin();
  Float_t maxH = dataDist->GetYaxis()->GetXmax();
  //Float_t rangeH = maxH - minH;
  //Float_t widthH = dataDist->GetYaxis()->GetBinWidth(0);

  Int_t nbinsT = dataDist->GetNbinsZ();
  Float_t minT = dataDist->GetZaxis()->GetXmin();
  Float_t maxT = dataDist->GetZaxis()->GetXmax();
  //Float_t rangeT = maxT - minT;
  //Float_t widthT = dataDist->GetZaxis()->GetBinWidth(0);

  //Double_t binSize = widthR * widthH * widthT;


  // set test weight distributions (if weightSetting==0, just uses those from infile) 
  if(weightSetting>0) {
    for(r=1; r<=nbinsR; r++) { 
      for(h=1; h<=nbinsH; h++) { 
        for(t=1; t<=nbinsT; t++) { 
          switch(weightSetting) {
            case 1: dataDist->SetBinContent(r,h,t,
              1
            ); break;
            case 2: dataDist->SetBinContent(r,h,t,
              r < (Double_t)nbinsR/2. ? 1:0 
            ); break;
            case 3: dataDist->SetBinContent(r,h,t,
              h < (Double_t)nbinsH/2. ? 1:0 
            ); break;
            case 4: dataDist->SetBinContent(r,h,t,
              t < (Double_t)nbinsT/2. ? 1:0 
            ); break;
            default: fprintf(stderr,"ERROR: bad weightSetting\n"); return;
          };
        };
      };
    };
  };


  // generate list of (l,m,twist) values
  // - Modulation instances are set with the polarizations LU or UU (or any other)
  // - since Modulation will create a new TF3 if the twist,l,m values change, we create
  //   an instance of Modulation for each function; this speeds things up
  enum idx_enum{kL,kM,kTwist,Nidx};
  Int_t l,m,twist,lev;
  Modulation * modu;
  TObjArray * moduArr = new TObjArray();
  // F_LU modulations
  if(polarizationSetting == Modulation::kLU) {
    for(l=0; l<=LMAX; l++) {
      for(m=0; m<=l; m++) {
        for(twist=2; twist<=3; twist++) {
          if(!enableLegendre && l<LMAX) continue;
          if((twist==2 && m>0) || twist==3) {
            moduArr->AddLast(new Modulation(twist,l,m,0,enableLegendre,polarizationSetting));
            if(twist==3 && m>0) { // negative m states
              moduArr->AddLast(new Modulation(twist,l,-m,0,enableLegendre,polarizationSetting));
            };
          } 
        };
      };
    };
  }
  // F_UU modulations
  else if(polarizationSetting == Modulation::kUU) {
    Int_t levMax;
    for(l=0; l<=LMAX; l++) {
      for(m=0; m<=l; m++) {
        for(twist=2; twist<=3; twist++) {
          levMax = twist==2 ? 1:0;
          for(lev=0; lev<=levMax; lev++) {
            if(!enableLegendre && l<LMAX) continue;
            if((twist==2 && lev==0 && m>=0) || (twist==2 && lev==1) || twist==3) {
              moduArr->AddLast(new Modulation(twist,l,m,lev,enableLegendre,polarizationSetting));
              if(((twist==2 && lev==1) || twist==3) && m>0) { // negative m states
                moduArr->AddLast(new Modulation(twist,l,-m,lev,enableLegendre,polarizationSetting));
              };
            };
          };
        };
      };
    };
  };
  Int_t NMODi = moduArr->GetEntries();
  const Int_t NMOD = NMODi;


  // |<fg>| matrix
  TH2D * orthMatrix = new TH2D("orthMatrix","<fg> matrix",NMOD,0,NMOD,NMOD,0,NMOD);
  TString funcT[NMOD];
  TString funcTex[NMOD];
  for(f=0; f<NMOD; f++) { 
    modu = (Modulation*) moduArr->At(f);
    //if(useModulationTitle) funcT[f] = modu->ModulationTitle();
    //else funcT[f] = modu->StateTitle();
    //funcT[f] = modu->ModulationTitle();
    //funcT[f] += " -- " + modu->StateTitle();
    funcT[f] = modu->StateTitle();
    funcTex[f] = modu->StateTitle();
    //funcTex[f] = modu->ModulationTitle();
    printf("%s\n",funcT[f].Data());
    orthMatrix->GetXaxis()->SetBinLabel(f+1,funcT[f]);
    orthMatrix->GetYaxis()->SetBinLabel(f+1,funcT[f]);
  };


  // calculate <fg>
  TH3D * intDist[NMOD][NMOD];
  //TH2D * modDist_hr[NMOD];
  //TH1D * modDist_t[NMOD];
  //TString modDistN,modDistT;
  Float_t phiH,phiR,theta;
  Double_t dataWeight,modValF,modValG,product,integral;
  TString intDistN,intDistT;
  Double_t weightedNorm[NMOD];
  Double_t valF,valG;
  Modulation * moduF;
  Modulation * moduG;
  for(f=0; f<NMOD; f++) {
    moduF = (Modulation*) moduArr->At(f);
    for(g=0; g<NMOD; g++) {
      moduG = (Modulation*) moduArr->At(g);

      // generate histos, which hold the product of data*f*g
      intDistN = Form("intDist_f%d_g%d",f,g);
      intDistT = "f: " + funcT[f] + ",  g: " + funcT[g];
      intDist[f][g] = new TH3D(intDistN,intDistT,
        nbinsR,minR,maxR, nbinsH,minH,maxH, nbinsT,minT,maxT);
      /*
      if(f==g) {
        modDistN = Form("modDist_hr_f%d",f);
        modDistT = funcT[f] + 
          " -- #Phi(#phi_{h},#phi_{R}) -- #phi_{h} vs. #phi_{R};#phi_{R};#phi_{h}";
        modDist_hr[f] = new TH2D(modDistN,modDistT,nbinsR,minR,maxR, nbinsH,minH,maxH);
        modDistN = Form("modDist_t_f%d",f);
        modDistT = funcT[f] + " -- P(cos#theta) -- #theta;#theta";
        modDist_t[f] = new TH1D(modDistN,modDistT,nbinsT,minT,maxT);
      };
      */


      // loop over bins
      for(r=1; r<=nbinsR; r++) {
        for(h=1; h<=nbinsH; h++) {
          for(t=1; t<=nbinsT; t++) {

            phiR = intDist[f][g]->GetXaxis()->GetBinCenter(r);
            phiH = intDist[f][g]->GetYaxis()->GetBinCenter(h);
            theta = intDist[f][g]->GetZaxis()->GetBinCenter(t);

            dataWeight = dataDist->GetBinContent(r,h,t);
            modValF = moduF->Evaluate(phiR,phiH,theta);
            modValG = moduG->Evaluate(phiR,phiH,theta);
            
            product = dataWeight * modValF * modValG;

            intDist[f][g]->SetBinContent(r,h,t,product);

            /*
            if(r==1&&h==1) {
              if(f==g) modDist_t[f]->SetBinContent(t,legF);
            };
            if(t==1) {
              if(f==g) modDist_hr[f]->SetBinContent(r,h,aziF);
            };
            */
          };
        };
      };

      // computed weighted normalization for function f as 1/sqrt(<ff>)
      if(f==g) {
        integral = intDist[f][g]->Integral("width");
        weightedNorm[f] = 1 / TMath::Sqrt(integral);
      };
    };
  };



  // fill orthogonality matrix
  for(f=0; f<NMOD; f++) {
    for(g=0; g<NMOD; g++) {

      // normalize data*f*g by 1/sqrt(<ff><gg>)
      intDist[f][g]->Scale(weightedNorm[f]*weightedNorm[g]);

      // sum data*f*g over all bins
      //integral = TMath::Abs( intDist[f][g]->Integral("width") );
      integral = intDist[f][g]->Integral("width");
      orthMatrix->SetBinContent(f+1,g+1,integral);
    };
  };

  // draw
  gStyle->SetOptStat(0);
  gStyle->SetPaintTextFormat(".3f");
  TCanvas * matCanv = new TCanvas("matCanv","matCanv",1000,1000);
  orthMatrix->SetMinimum(-1);
  orthMatrix->SetMaximum(1);
  orthMatrix->GetXaxis()->SetLabelSize(0.03);
  orthMatrix->GetYaxis()->SetLabelSize(0.03);
  if(polarizationSetting==Modulation::kUU) {
    gStyle->SetPalette(kBird);
    orthMatrix->Draw("colztext");
    orthMatrix->GetXaxis()->SetRangeUser(0,1);
  }
  else {
    orthMatrix->Draw("boxtext");
  };




  // EXTRA PLOTS
  TCanvas * dataCanv = new TCanvas("dataCanv","dataCanv",1200,800);
  //dataCanv->Divide(1,2);
  dataCanv->cd(1); Draw3d(dataDist,1);
  //dataCanv->cd(2); Draw3d(dataDist,2);

  /*
  TCanvas * modCanv = new TCanvas("modCanv","modCanv",800,800);
  modCanv->Divide(5,2*((NMOD+5)/5));
  for(f=0; f<NMOD; f++) {
    //intDist[1][1]->Draw();
    modCanv->cd(5*(f/5)+f+1); modDist_hr[f]->Draw("colz");
    modCanv->cd(5*(f/5)+5+f+1); modDist_t[f]->Draw();
    //modCanv->cd(f+1); Draw3d(intDist[f][f],1);
    //modCanv->cd(f+1); intDist[f][f]->Draw();
    //modCanv->cd(NMOD+f+1); Draw3d(intDist[f][f],2);
  };
  */



  // print <fg> matrix (mathematica syntax)
  printf("A={\n");
  for(f=0; f<NMOD; f++) {
    printf("{");
    for(g=0; g<NMOD; g++) 
      printf("%.3f%s",orthMatrix->GetBinContent(f+1,g+1),g+1==NMOD?"":",");
    printf("}%s\n",f+1==NMOD?"":",");
  };
  printf("};\n\n");

  // print matrix for latex
  TString bra,ket;
  TString texname = infileN;
  TString polstr;
  if(polarizationSetting==Modulation::kLU) polstr="LU";
  else if(polarizationSetting==Modulation::kUU) polstr="UU";
  texname.ReplaceAll(".root","");
  texname = Form("%s.%d.tex",texname.Data(),binNum);
  if(printLatex) {
    gSystem->RedirectOutput(texname,"w");
    if(polarizationSetting==Modulation::kLU) printf("\\begin{table}\n");
    else if(polarizationSetting==Modulation::kUU) printf("\\begin{sidewaystable}\n");
    printf("\\begin{center}\n");
    TString fmtStr="|c||"; for(f=0; f<NMOD; f++) fmtStr+="c|";
    if(polarizationSetting==Modulation::kUU) printf("\\resizebox{\\textwidth}{!}{\n");
    printf("\\begin{tabular}{%s}\n",fmtStr.Data());
    printf("\\hline\n");
    printf("~");
    // - print header
    for(f=0; f<NMOD; f++) {
      funcTex[f] = funcTex[f].ReplaceAll("|L,","|\\ell,");
      /*
      funcTex[f] = funcTex[f].ReplaceAll("sin","\\sin");
      funcTex[f] = funcTex[f].ReplaceAll("cos","\\cos");
      funcTex[f] = funcTex[f].ReplaceAll("#phiH","\\phih");
      funcTex[f] = funcTex[f].ReplaceAll("#phiR","\\phir");
      funcTex[f] = funcTex[f].ReplaceAll("#theta","\\theta");
      funcTex[f] = funcTex[f].ReplaceAll("(","\\left(");
      funcTex[f] = funcTex[f].ReplaceAll(")","\\right)");
      funcTex[f] = funcTex[f].ReplaceAll("*","");
      */
      ket = funcTex[f];
      ket = ket.ReplaceAll(">","\\rangle");
      printf("&\\rotatebox{90}{$%s$}",ket.Data());
    };
    printf("\\\\\\hline\\hline\n");
    // - print rows
    for(f=0; f<NMOD; f++) {
      bra = funcTex[f];
      bra = bra.ReplaceAll("|","\\langle");
      bra = bra.ReplaceAll(">","|");
      printf("$%s$",bra.Data());
      // - print columns
      for(g=0; g<NMOD; g++)
        printf("&$%.3f$",orthMatrix->GetBinContent(f+1,g+1));
      printf("\\\\\\hline\n");
    };
    printf("\\end{tabular}\n");
    if(polarizationSetting==Modulation::kUU) printf("}\n");
    printf("\\caption{$d\\sigma_{%s}$ modulation orthogonality matrix for BINCAP.}\n",polstr.Data());
    if(polarizationSetting==Modulation::kLU) printf("\\label{orthoMatrixLU_BINLAB}\n");
    else if(polarizationSetting==Modulation::kUU) printf("\\label{orthoMatrixUU_BINLAB}\n");
    printf("\\end{center}\n");
    if(polarizationSetting==Modulation::kLU) printf("\\end{table}\n");
    else if(polarizationSetting==Modulation::kUU) printf("\\end{sidewaystable}\n");
    gSystem->RedirectOutput(0);
  };
};

void Draw3d(TH3D * dd, Int_t whichProj) {
  TString ddN = dd->GetName();
  TString ddT = dd->GetTitle();

  TH2D * dd_yx;
  TH1D * dd_z;

  switch(whichProj) {
    case 1:
      dd_yx = (TH2D*) dd->Project3D("yx");
      dd_yx->SetTitle(TString(ddT+" -- #phi_{h} vs. #phi_{R} projection;#phi_{R};#phi_{h}"));
      dd_yx->Draw("colz"); 
      break;
    case 2:
      //dd_z = (TH1D*) dd->Project3D("z");
      dd_z = (TH1D*) dd->ProjectionZ();
      dd_z->SetTitle(TString(ddT+" -- #theta projection;#theta"));
      dd_z->Draw();
      break;
    default: return;
  };
};
