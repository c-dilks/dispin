// draw partial wave amplitude fit results for DIS2021

R__LOAD_LIBRARY(DiSpin)

void CanvasPartition(TCanvas *C,const Int_t Nx = 2,const Int_t Ny = 2,
                     Float_t lMargin = 0.15, Float_t rMargin = 0.05,
                     Float_t bMargin = 0.15, Float_t tMargin = 0.05);

void drawDIS2021(TString bruAsymFile="bruspin.DIS.z.mh/asym_mcmc_BL0.root") {

  // OPTIONS /////////////////////////////////////////////////////////////////
  Bool_t includeMeq0 = true; // include m=0 in twist 3 plots

  Float_t asymMax = 0.07; // vertical range
  Float_t asymMin = -asymMax;

  Float_t graphTitleSize = 0.08; // title sizes
  Float_t axisTitleSize = 0.05;

  Float_t lMargin = 0.12; // overall margins
  Float_t rMargin = 0.05;
  Float_t bMargin = 0.15;
  Float_t tMargin = 0.05;

  //////////////////////////////////////////////////////////////////////////

  // read asymmetry results file
  TFile * infile = new TFile(bruAsymFile,"READ");

  // define canvases
  gStyle->SetPadTickX(1); // put tick marks on both sides
  gStyle->SetPadTickY(1);
  gStyle->SetTitleSize(0.08,"T");
  enum enumTw {tw2,tw3};
  TString twStr[2] = {"Tw2","Tw3"};
  Int_t nrows[2];
  Int_t ncols[2];
  TCanvas * canv[2];
  for(int t=0; t<2; t++) {

    // get number of rows and columns for canvas division
    if(t==tw2) { nrows[t]=2; ncols[t]=2; }
    else if(t==tw3) {
      if( !includeMeq0 ) { nrows[t]=2; ncols[t]=4; }
      else               { nrows[t]=3; ncols[t]=5; };
    };
    canv[t] = new TCanvas("canv"+twStr[t],"canv"+twStr[t],600*ncols[t],400*nrows[t]);
    CanvasPartition(canv[t],ncols[t],nrows[t],lMargin,rMargin,bMargin,tMargin);
  };


  // loop through asymmetry graphs
  TListIter nextKey = TListIter(infile->GetListOfKeys());
  Modulation * modu;
  TGraphErrors * gr;
  TString keyname,parname,xtitle,gtitle;
  Int_t twist,L,M,padnum;
  TPad * pad;
  TLine * zero;
  TLatex * tex;
  Double_t xmin,xmax;
  while(TKey * key = (TKey*) nextKey()) {

    keyname = TString(key->GetName());
    if(keyname.Contains(TRegexp("^gr_pwAmp"))) {

      gr = (TGraphErrors*) key->ReadObj();
      parname = ((TObjString*)keyname.Tokenize("_")->At(1))->GetString();

      // get modulation's eigenvalues, etc.
      modu = new Modulation(parname);
      switch(modu->GetTw()) {
        case 2: twist=tw2; break;
        case 3: twist=tw3; break;
        default: fprintf(stderr,"ERROR: bad twist\n"); return;
      };
      L = modu->GetL();
      M = modu->GetM();

      // pad assignment
      // (note: padnum here starts from 1 at the bottom left
      // pad, counting upward going left to right)
      padnum=0;
      if(twist==tw2) {
        if     ( L==1 && M==1 ) padnum=3;
        else if( L==2 && M==1 ) padnum=1;
        else if( L==2 && M==2 ) padnum=2;
      }
      else if(twist==tw3) {
        if( !includeMeq0 ) {
          if     ( L==1 && M==-1 ) padnum=6;
          else if( L==1 && M== 1 ) padnum=7;
          else if( L==2 && M==-2 ) padnum=1;
          else if( L==2 && M==-1 ) padnum=2;
          else if( L==2 && M== 1 ) padnum=3;
          else if( L==2 && M== 2 ) padnum=4;
        }
        else {
          if     ( L==0 && M== 0 ) padnum=13;
          else if( L==1 && M==-1 ) padnum=7;
          else if( L==1 && M== 0 ) padnum=8;
          else if( L==1 && M== 1 ) padnum=9;
          else if( L==2 && M==-2 ) padnum=1;
          else if( L==2 && M==-1 ) padnum=2;
          else if( L==2 && M== 0 ) padnum=3;
          else if( L==2 && M== 1 ) padnum=4;
          else if( L==2 && M== 2 ) padnum=5;
        }
      };

      // draw
      if(padnum>0) {
        canv[twist]->cd(0);
        pad = (TPad*) gROOT->FindObject(Form("pad_%d",padnum));
        pad->Draw();
        pad->cd();
        
        gtitle = gr->GetTitle();
        xtitle = gtitle;
        xtitle(TRegexp("^.*vs. ")) = "";
        gr->GetXaxis()->SetTitle(xtitle);
        gr->GetYaxis()->SetRangeUser(asymMin,asymMax);
        gr->GetXaxis()->SetLabelSize(axisTitleSize);
        gr->GetYaxis()->SetLabelSize(axisTitleSize);
        gr->GetXaxis()->SetTitleSize(axisTitleSize);
        gr->GetYaxis()->SetTitleSize(axisTitleSize);
        gr->SetMarkerSize(1.5);

        gr->Draw("APE");

        xmin = gr->GetXaxis()->GetXmin();
        xmax = gr->GetXaxis()->GetXmax();
        zero = new TLine(xmin,0,xmax,0);
        zero->SetLineColor(kBlack);
        zero->SetLineWidth(2);
        zero->SetLineStyle(kDashed);
        zero->Draw();

        // hack to force axis labels to be visible, for cases where there
        // is an empty TPad to the left of the curent one
        // -- almost works, but it's easier to just use inkscape
        /*
        if( (L==0 && M==0) || (L==1 && M==-1) ) {
          printf("--- %d\n",pad->GetBBox().fX);
          Short_t lb = pad->GetBBox().fX;
          UShort_t bw = pad->GetBBox().fWidth;
          Float_t shift = 50;
          Float_t bump = shift/(bw+shift);
          printf("bump=%f\n",bump);
          pad->SetBBoxX1(lb-shift); // bump left boundary of TPad leftward
          pad->SetLeftMargin(bump); // fudge factor for re-alignment of plot
        };
        */

        tex = new TLatex(xmin+0.1*(xmax-xmin),asymMax-0.1*(asymMax-asymMin),gtitle);
        //tex = new TLatex(0.1,0.8,"label");
        gr->SetTitle("");
        tex->SetTextSize(0.1);
        //tex->SetTextAlign(30);
        //tex->SetNDC(1);
        tex->Draw();

          

      };
    };
  };

  // print PDFs
  for(int t=0; t<2; t++) {
    canv[t]->Print(TString(canv[t]->GetName())+".pdf","pdf");
  };
};



// -- canvas partition method, from $ROOTSYS/tutorials/graphics/canvas2.C
// (copied from root v6.20.04, and adapted for use here, with small revisions)
// original author = Olivier Couet
void CanvasPartition(TCanvas *C,const Int_t Nx,const Int_t Ny,
                     Float_t lMargin, Float_t rMargin,
                     Float_t bMargin, Float_t tMargin)
{
   if (!C) return;

   // Setup Pad layout:
   Float_t vSpacing = 0.0;
   Float_t vStep  = (1.- bMargin - tMargin - (Ny-1) * vSpacing) / Ny;

   Float_t hSpacing = 0.0;
   Float_t hStep  = (1.- lMargin - rMargin - (Nx-1) * hSpacing) / Nx;

   Float_t vposd,vposu,vmard,vmaru,vfactor;
   Float_t hposl,hposr,hmarl,hmarr,hfactor;

   for (Int_t i=0;i<Nx;i++) {

      if (i==0) {
         hposl = 0.0;
         hposr = lMargin + hStep;
         hfactor = hposr-hposl;
         hmarl = lMargin / hfactor;
         hmarr = 0.0;
      } else if (i == Nx-1) {
         hposl = hposr + hSpacing;
         hposr = hposl + hStep + rMargin;
         hfactor = hposr-hposl;
         hmarl = 0.0;
         hmarr = rMargin / (hposr-hposl);
      } else {
         hposl = hposr + hSpacing;
         hposr = hposl + hStep;
         hfactor = hposr-hposl;
         hmarl = 0.0;
         hmarr = 0.0;
      }

      for (Int_t j=0;j<Ny;j++) {

         if (j==0) {
            vposd = 0.0;
            vposu = bMargin + vStep;
            vfactor = vposu-vposd;
            vmard = bMargin / vfactor;
            vmaru = 0.0;
         } else if (j == Ny-1) {
            vposd = vposu + vSpacing;
            vposu = vposd + vStep + tMargin;
            vfactor = vposu-vposd;
            vmard = 0.0;
            vmaru = tMargin / (vposu-vposd);
         } else {
            vposd = vposu + vSpacing;
            vposu = vposd + vStep;
            vfactor = vposu-vposd;
            vmard = 0.0;
            vmaru = 0.0;
         }

         C->cd(0);

         char name[16];
         int padn = Nx*j+i+1;
         sprintf(name,"pad_%i",padn);
         TPad *pad = (TPad*) gROOT->FindObject(name);
         if (pad) delete pad;
         pad = new TPad(name,"",hposl,vposd,hposr,vposu);
         pad->SetLeftMargin(hmarl);
         pad->SetRightMargin(hmarr);
         pad->SetBottomMargin(vmard);
         pad->SetTopMargin(vmaru);

         pad->SetFrameBorderMode(0);
         pad->SetBorderMode(0);
         pad->SetBorderSize(0);

         pad->Draw();
      }
   }
}
