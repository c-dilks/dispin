#include "Binning.h"

ClassImp(Binning)

using namespace std;


Binning::Binning(Int_t pairType_) {
  //printf("Instantiating Binning...\n");

  // get hadron indices (bin bounds depends on hadron type)
  DecodePairType(pairType_,whichHad[qA],whichHad[qB]);
  numKaons = 0;
  for(int h=0; h<2; h++) { if(whichHad[h]==kKp || whichHad[h]==kKm) numKaons++; };

  // set minimum and maximum IV values
  minIV[vM] = 0;   maxIV[vM] = 3;
  minIV[vX] = 0;   maxIV[vX] = 1;
  minIV[vZ] = 0;   maxIV[vZ] = 1;
  minIV[vPt] = 0;  maxIV[vPt] = 2;
  minIV[vPh] = 0;  maxIV[vPh] = 10;
  minIV[vQ] = 0;   maxIV[vQ] = 12;
  for(int v=0; v<nIV; v++) nBins[v]=-1;


  // set minimum bin boundaries
  for(int v=0; v<nIV; v++) AddBinBound(v,minIV[v]);


  // set main bin boundaries
  if(numKaons==0) {

    // -- M_h (dihadron invariant mass)
    /* // 7 quantile bins
    AddBinBound(vM,0.46);
    AddBinBound(vM,0.60);
    AddBinBound(vM,0.72);
    AddBinBound(vM,0.81);
    AddBinBound(vM,0.93);
    AddBinBound(vM,1.10);
    */
    ///* // 14 quantile bins
    AddBinBound(vM,0.38);
    AddBinBound(vM,0.44);
    AddBinBound(vM,0.51);
    AddBinBound(vM,0.57);
    AddBinBound(vM,0.63);
    AddBinBound(vM,0.69);
    AddBinBound(vM,0.74);
    AddBinBound(vM,0.78);
    AddBinBound(vM,0.82);
    AddBinBound(vM,0.88);
    AddBinBound(vM,0.94);
    AddBinBound(vM,1.02);
    AddBinBound(vM,1.16);
    //*/
    /* // above/below rho
    AddBinBound(vM,0.77);
    */

    // -- x (bjorken-x)
    /* // 7 quantile bins
    AddBinBound(vX,0.12);
    AddBinBound(vX,0.15);
    AddBinBound(vX,0.18);
    AddBinBound(vX,0.21);
    AddBinBound(vX,0.25);
    AddBinBound(vX,0.32);
    */
    ///* // 14 quantile bins
    AddBinBound(vX,0.11);
    AddBinBound(vX,0.13);
    AddBinBound(vX,0.14);
    AddBinBound(vX,0.15);
    AddBinBound(vX,0.17);
    AddBinBound(vX,0.18);
    AddBinBound(vX,0.19);
    AddBinBound(vX,0.21);
    AddBinBound(vX,0.23);
    AddBinBound(vX,0.25);
    AddBinBound(vX,0.28);
    AddBinBound(vX,0.32);
    AddBinBound(vX,0.38);
    //*/

    // -- z (fragmentation fraction)
    /* // 7 quantile bins
    AddBinBound(vZ,0.43);
    AddBinBound(vZ,0.48);
    AddBinBound(vZ,0.52);
    AddBinBound(vZ,0.57);
    AddBinBound(vZ,0.62);
    AddBinBound(vZ,0.68);
    */
    ///* // 14 quantile bins
    AddBinBound(vZ,0.40);
    AddBinBound(vZ,0.44);
    AddBinBound(vZ,0.46);
    AddBinBound(vZ,0.49);
    AddBinBound(vZ,0.51);
    AddBinBound(vZ,0.53);
    AddBinBound(vZ,0.56);
    AddBinBound(vZ,0.58);
    AddBinBound(vZ,0.60);
    AddBinBound(vZ,0.63);
    AddBinBound(vZ,0.66);
    AddBinBound(vZ,0.69);
    AddBinBound(vZ,0.73);
    //*/

    // -- PhPerp (transverse momentum of dihadron)
    AddBinBound(vPt,0.29); // 5 quantiles (from GetQuantiles.C)
    AddBinBound(vPt,0.42);
    AddBinBound(vPt,0.55);
    AddBinBound(vPt,0.70);

    // -- Ph (magnitude of momentum sum of dihadron)
    AddBinBound(vPh,3.15); // 5 quantiles (from GetQuantiles.C)
    AddBinBound(vPh,3.60);
    AddBinBound(vPh,4.10);
    AddBinBound(vPh,4.80);

    // -- Q^2
    AddBinBound(vQ,2.84); // 5 quantiles (from GetQuantiles.C)
    AddBinBound(vQ,3.33);
    AddBinBound(vQ,3.92);
    AddBinBound(vQ,4.85);
    
  } else if(numKaons==1) {

    // -- mass
    AddBinBound(vM,0.85);
    AddBinBound(vM,1.1);

    // -- x
    AddBinBound(vX,0.2);
    AddBinBound(vX,0.3);

    // -- z
    AddBinBound(vZ,0.5);
    AddBinBound(vZ,0.7);

    // -- other IVs' binnings can be added later when kaons are included
  };

  // set maximum bin boundaries
  for(int v=0; v<nIV; v++) AddBinBound(v,maxIV[v]);


  // set IV names and titles
  IVname[vM] = "M";
  IVname[vX] = "X";
  IVname[vZ] = "Z";
  IVname[vPt] = "Pt";
  IVname[vPh] = "Ph";
  IVname[vQ] = "Q";

  IVtitle[vM] = "M_{h}";
  IVtitle[vX] = "x";
  IVtitle[vZ] = "z";
  IVtitle[vPt] = "P_{h}^{perp}";
  IVtitle[vPh] = "P_{h}";
  IVtitle[vQ] = "Q^{2}";


  //PrintBinBounds();

  // set binning scheme defaults
  dimensions = 0;
  for(int d=0; d<3; d++) ivVar[d] = -1;
  oaTw = UNDEF;
  oaL = UNDEF;
  oaM = UNDEF;
  useWeighting = false;
  gridDim = 1;
};



void Binning::AddBinBound(Int_t ivIdx, Float_t newBound) {
  if(ivIdx<0 || ivIdx>=nIV) {
    fprintf(stderr,"ERROR: bad Binning::AddBinBound call");
    return;
  };

  bound[ivIdx].push_back(newBound);
  nBins[ivIdx]++;

  return;
};


void Binning::PrintBinBounds() {
  printf("\n");
  for(int v=0; v<nIV; v++) {
    printf("[ %s ] %s bins:  (nbins=%d)\n",IVname[v].Data(),IVtitle[v].Data(),nBins[v]);
    for(int b=0; b<nBins[v]; b++) {
      printf(" bin %d:\t\t%.2f\t%.2f\n",b,bound[v].at(b),bound[v].at(b+1));
    };
  };
  printf("\n");
};


Int_t Binning::GetBin(Int_t ivIdx_, Float_t iv_) {
  if(ivIdx_<0 || ivIdx_>=nIV) {
    fprintf(stderr,"ERROR: bad Binning::GetBin call\n");
    return -1;
  };

  for(int b=0; b<nBins[ivIdx_]; b++) {

    if( iv_ >= bound[ivIdx_].at(b) &&
        iv_ <  bound[ivIdx_].at(b+1) ) {
      return b;
    };
  };

  fprintf(stderr,"ERROR bin not found for %s=%.2f\n",IVname[ivIdx_].Data(),iv_);
  return -1;
};


TString Binning::GetBoundStr(Int_t bn, Int_t dim) {
  if(dim<0||dim>3) return "";
  Int_t v_ = ivVar[dim];
  Int_t b_ = this->UnhashBinNum(bn,dim);
  TString retStr;
  Float_t lb,ub;
  try {
    lb = bound[v_].at(b_);
    ub = bound[v_].at(b_+1);
  } catch(const std::out_of_range & ex) {
    fprintf(stderr,"ERROR: bad GetBoundStr call\n");
    return "";
  };
  retStr = Form("%s#in[%.2f, %.2f)",IVtitle[v_].Data(),lb,ub);
  return retStr;
};



Bool_t Binning::SetScheme(Int_t ivType) {

  // determine number of dimensions
  dimensions=1;
  if(ivType>=10) dimensions=2;
  if(ivType>=100) dimensions=3;
  if(ivType>=1000) {
    fprintf(stderr,"ERROR: ivType has too many digits\n");
    return false;
  };

  // read ivType and convert it to IV enumerators
  // -- if dimensions==1, all asymmetries will be plotted against one independent
  //    variable (IV); in this case, ivType is that IV, according to enumerators in
  //    Binning (ivEnum)
  // -- if dimensions==2, asymmetries are plotted for two IVs. For this one, ivType is
  //    understood as a 2-digit number: the first digit is IV0, and the second is IV1.
  //    The asymmetries will be plotted vs. IV0, for bins in IV1
  // -- if dimensions==3, we have 3 IVs and ivType is a 3-digit number, representing
  //    IV0, IV1, and IV2. Aymmetries are plotted vs IV0 for bins in (IV1,IV2) pairs
  switch(dimensions) {
    case 1:
      ivVar[0] = ivType - 1;
      break;
    case 2:
      ivVar[0] = ivType / 10 - 1; 
      ivVar[1] = ivType % 10 - 1;
      break;
    case 3:
      ivVar[0] = ivType / 100 - 1;
      ivVar[1] = ( ivType / 10 ) % 10 - 1;
      ivVar[2] = ivType % 10 - 1;
      break;
    default:
      fprintf(stderr,"ERROR: bad number of dimensions\n");
      return false;
  };

  // DNP2019 override: interested in z-dependence above and below rho mass
  // - override binning scheme, if we requested z-dependence in mass bins (ivType 32)
  if(ivType==32) {
    printf("\n\nDNP2019 OVERRIDE: bin in z for M above and below rho mass\n\n");
    nBins[vM] = -1;
    bound[vM].clear();
    AddBinBound(vM,minIV[vM]);
    AddBinBound(vM,0.77);
    AddBinBound(vM,maxIV[vM]);
    PrintBinBounds();
  };

  // check IV enumerators
  for(int d=0; d<dimensions; d++) {
    if(!(ivVar[d]>=0 && ivVar[d]<nIV)) {
      printf("ivVar[%d] = %d\n",d,ivVar[d]);
      fprintf(stderr,"ERROR: this IV is unknown\n");
      return false;
    };
  };

  
  // print which IVs will be analyzed
  printf("--> Binning scheme set for asymmetries vs. %s ",(IVname[ivVar[0]]).Data());
  if(dimensions>=2) printf("in bins of %s ",(IVname[ivVar[1]]).Data());
  if(dimensions>=3) printf("and %s ",(IVname[ivVar[2]]).Data());
  printf("\n\n");


  // build binVec and binVecMap
  if(dimensions == 1) {
    for(int b=0; b<GetNbins(0); b++) {
      binNum = HashBinNum(b);
      binVec.push_back(binNum);
      binVecMap[0].insert(std::pair<Int_t,Int_t>(binNum,b));
      binVecMap[1].insert(std::pair<Int_t,Int_t>(binNum,-1));
      binVecMap[2].insert(std::pair<Int_t,Int_t>(binNum,-1));
    };
  }
  else if(dimensions == 2) {
    for(int b1=0; b1<GetNbins(1); b1++) {
      for(int b0=0; b0<GetNbins(0); b0++) {
        binNum = HashBinNum(b0,b1);
        binVec.push_back(binNum);
        binVecMap[0].insert(std::pair<Int_t,Int_t>(binNum,b0));
        binVecMap[1].insert(std::pair<Int_t,Int_t>(binNum,b1));
        binVecMap[2].insert(std::pair<Int_t,Int_t>(binNum,-1));
      };
    };
  }
  else if(dimensions == 3) {
    for(int b2=0; b2<GetNbins(2); b2++) {
      for(int b1=0; b1<GetNbins(1); b1++) {
        for(int b0=0; b0<GetNbins(0); b0++) {
          binNum = HashBinNum(b0,b1,b2);
          binVec.push_back(binNum);
          binVecMap[0].insert(std::pair<Int_t,Int_t>(binNum,b0));
          binVecMap[1].insert(std::pair<Int_t,Int_t>(binNum,b1));
          binVecMap[2].insert(std::pair<Int_t,Int_t>(binNum,b2));
        };
      };
    };
  };


  return true;
}; // eo Binning::SetScheme


// scheme accessors
Int_t Binning::GetNbins(Int_t dim) {
  return CheckDim(dim) ? nBins[ivVar[dim]] : -1;
};
TString Binning::GetIVname(Int_t dim) {
  return CheckDim(dim) ? IVname[ivVar[dim]] : "unknown";
};
TString Binning::GetIVtitle(Int_t dim) {
  return CheckDim(dim) ? IVtitle[ivVar[dim]] : "unknown";
};

Bool_t Binning::CheckDim(Int_t dim_) { 
  if(dim_>=0 && dim_<dimensions) return true;
  else {
    fprintf(stderr,"ERROR: Binning::CheckDim failed\n");
    return false;
  };
};

// 3-tuple of bin numbers -> single 3-digit bin number
Int_t Binning::HashBinNum(Int_t bin0, Int_t bin1, Int_t bin2) {
  Int_t retval = bin0;
  if(bin1>=0) retval += 10 * bin1;
  if(bin2>=0) retval += 100 * bin2;
  return retval;
};

// single 3-digit bin number -> bin number in dimension `dim`
Int_t Binning::UnhashBinNum(Int_t bn, Int_t dim) {
  if(dim<0||dim>=3) return -1;
  Int_t retnum;
  try { retnum = binVecMap[dim].at(bn); }
  catch(const std::out_of_range & ex) {
    fprintf(stderr,"ERROR: bad UnhashBinNum call\n");
    return -1;
  };
  return retnum;
};

Binning::~Binning() {};
