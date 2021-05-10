#include "Binning.h"

ClassImp(Binning)

using namespace std;


Binning::Binning() {
  //printf("Instantiating Binning...\n");

  // set minimum and maximum IV values
  minIV[vM] = 0;   maxIV[vM] = 3;
  minIV[vX] = 0;   maxIV[vX] = 1;
  minIV[vZ] = 0;   maxIV[vZ] = 1;
  minIV[vPt] = 0;  maxIV[vPt] = 2;
  minIV[vDY] = 0;  maxIV[vDY] = 4;
  minIV[vQ] = 0;   maxIV[vQ] = 12;
  minIV[vXF] = 0;  maxIV[vXF] = 1;
  for(int v=0; v<nIV; v++) nBins[v]=-1;


  // set minimum and maximum bin boundaries
  for(int v=0; v<nIV; v++) {
    AddBinBound(v,minIV[v]);
    AddBinBound(v,maxIV[v]);
  };


  // set IV names and titles
  IVname[vM] = "Mh";
  IVname[vX] = "X";
  IVname[vZ] = "Z";
  IVname[vPt] = "PhPerp";
  IVname[vDY] = "DY";
  IVname[vQ] = "Q2";
  IVname[vXF] = "XF";

  IVtitle[vM] = "M_{h}";
  IVtitle[vX] = "x";
  IVtitle[vZ] = "z";
  IVtitle[vPt] = "P_{h,T}";
  IVtitle[vDY] = "|#Delta Y|";
  IVtitle[vQ] = "Q^{2}";
  IVtitle[vXF] = "x_{F}";


  // set binning scheme defaults
  dimensions = 0;
  for(int d=0; d<3; d++) ivVar[d] = -1;
  oaTw = UNDEF;
  oaL = UNDEF;
  oaM = UNDEF;
  useWeighting = false;
  gridDim = 1;
  for(int h=0; h<2; h++) whichHad[h]=-1;
  for(int d=0; d<3; d++) binArray[d] = new TArrayD();
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
      printf(" bin  %d:\t%.3f\t%.3f\n",b,bound[v].at(b),bound[v].at(b+1));
    };
  };
  printf("\n");
};


// get bin associated with iv ivIdx_
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

// get bin associated with current event from Event tree, within
// current binning scheme
Int_t Binning::FindBin(EventTree * ev) {
  Float_t ivVal[3] = {-1000,-1000,-1000};
  Int_t ivBin[3] = {-1,-1,-1};
  for(int d=0; d<dimensions; d++) {
    switch(ivVar[d]) {
      case vM: ivVal[d] = ev->Mh; break;
      case vX: ivVal[d] = ev->x; break;
      case vZ: ivVal[d] = ev->Zpair; break;
      case vPt: ivVal[d] = ev->PhPerp; break;
      case vDY: ivVal[d] = ev->DY; break;
      case vQ: ivVal[d] = ev->Q2; break;
      case vXF: ivVal[d] = ev->xF; break;
      default: 
                         fprintf(stderr,"ERROR: bad ivVar\n");
                         return -1;
    };
    ivBin[d] = this->GetBin(ivVar[d],ivVal[d]);
  };
  return this->HashBinNum(ivBin[0],ivBin[1],ivBin[2]);
};

// get bin associated with specified values, using current binning scheme
Int_t Binning::FindBin(Float_t iv0, Float_t iv1, Float_t iv2) {
  Float_t ivVal[3] = {iv0,iv1,iv2};
  Int_t ivBin[3] = {-1,-1,-1};
  for(int d=0; d<dimensions; d++) {
    ivBin[d] = this->GetBin(ivVar[d],ivVal[d]);
  };
  return this->HashBinNum(ivBin[0],ivBin[1],ivBin[2]);
}




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


Bool_t Binning::SetScheme(Int_t ivType, Int_t nb0, Int_t nb1, Int_t nb2) {

  // clear current scheme
  binVec.clear();
  for(int d=0; d<3; d++) {
    ivVar[d] = -1;
    binVecMap[d].clear();
  };
  for(int v=0; v<nIV; v++) {
    nBins[v]=-1;
    bound[v].clear();
  };


  // determine number of dimensions
  dimensions=1;
  if(ivType>=10) dimensions=2;
  if(ivType>=100) dimensions=3;
  if(ivType>=1000) {
    fprintf(stderr,"ERROR: ivType has too many digits\n");
    dimensions = 0;
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


  // check IV enumerators
  for(int d=0; d<dimensions; d++) {
    if(!(ivVar[d]>=0 && ivVar[d]<nIV)) {
      printf("ivVar[%d] = %d\n",d,ivVar[d]);
      fprintf(stderr,"ERROR: this IV is unknown\n");
      return false;
    };
  };


  // default binning schemes for each IV, if they weren't specified as args
  Int_t nb[3];
  TString defaultScheme = "DIS"; // <------------------ main switch
  if( nb0==-1 && nb1==-1 && nb2==-1) {
    if(defaultScheme=="DIS") { // DIS2021: RGA fa18+sp19 inbending data
      if(ivVar[0]==vX) nb0=12;
      if(ivVar[0]==vM) nb0=12;
      if(ivVar[0]==vZ  && ivVar[1]==vM)  { nb0=4; nb1=3; };
      if(ivVar[0]==vPt && ivVar[1]==vM)  { nb0=4; nb1=3; };
      if(ivVar[0]==vX  && ivVar[1]==vDY) { nb0=4; nb1=3; };
      if(ivVar[0]==vM  && ivVar[1]==vDY) { nb0=4; nb1=3; };
      if(ivVar[0]==vX  && ivVar[1]==vM && ivVar[2]==vDY) { nb0=3; nb1=2; nb2=2; };
      if(ivVar[0]==vDY) nb0=12;
    };
    if(defaultScheme=="PRL") { // PRL: RGA fa18 inbending data
      if(ivVar[0]==vX) nb0=12;
      if(ivVar[0]==vM) nb0=12;
      if(ivVar[0]==vZ  && ivVar[1]==vM) { nb0=6; nb1=2; };
      if(ivVar[0]==vPt && ivVar[1]==vM) { nb0=6; nb1=2; };
    };
  };
  nb[0]=nb0; nb[1]=nb1; nb[2]=nb2;


  // set minimum bin bound
  for(int v=0; v<nIV; v++) AddBinBound(v,minIV[v]);

  // set internal bin boundaries, for each defined dimension
  for(int d=0; d<dimensions; d++) {
    // -- Mh --------------
    if(ivVar[d] == vM) {
      switch(nb[d]) {
        case 1: break; // single bin
        case 2:
          //AddBinBound(vM,0.63); // PRL 2D binning
          AddBinBound(vM,0.95); // test >M_rho region
          break;
        case 3:
          AddBinBound(vM,0.60); // DIS 2D binning
          AddBinBound(vM,0.95);
          break;
        case 4:
          AddBinBound(vM,0.572);
          AddBinBound(vM,0.774);
          AddBinBound(vM,0.972);
          break;
        case 6:
          AddBinBound(vM,0.488);
          AddBinBound(vM,0.654);
          AddBinBound(vM,0.774);
          AddBinBound(vM,0.895);
          AddBinBound(vM,1.075);
          break;
        case 12:
          AddBinBound(vM,0.381); // PRL 1D binning
          AddBinBound(vM,0.462);
          AddBinBound(vM,0.531);
          AddBinBound(vM,0.606);
          AddBinBound(vM,0.675);
          AddBinBound(vM,0.737);
          AddBinBound(vM,0.781);
          AddBinBound(vM,0.831);
          AddBinBound(vM,0.900);
          AddBinBound(vM,0.981);
          AddBinBound(vM,1.125);
          break;
        case 18:
          AddBinBound(vM,0.371);
          AddBinBound(vM,0.432);
          AddBinBound(vM,0.488);
          AddBinBound(vM,0.544);
          AddBinBound(vM,0.600);
          AddBinBound(vM,0.654);
          AddBinBound(vM,0.702);
          AddBinBound(vM,0.741);
          AddBinBound(vM,0.774);
          AddBinBound(vM,0.809);
          AddBinBound(vM,0.849);
          AddBinBound(vM,0.895);
          AddBinBound(vM,0.945);
          AddBinBound(vM,1.002);
          AddBinBound(vM,1.075);
          AddBinBound(vM,1.168);
          AddBinBound(vM,1.290);
          break;
        default:
          fprintf(stderr,"ERROR: unknown nb for %s\n",GetIVname(d).Data());
      };
    }
    // -- x --------------
    else if(ivVar[d] == vX) {
      switch(nb[d]) {
        case 1: break; // single bin
        case 3:
          AddBinBound(vX,0.165);
          AddBinBound(vX,0.242);
          break;
        case 4:
          AddBinBound(vX,0.149);
          AddBinBound(vX,0.199);
          AddBinBound(vX,0.270);
          break;
        case 6:
          AddBinBound(vX,0.133); // DIS 2D binning
          AddBinBound(vX,0.165);
          AddBinBound(vX,0.199);
          AddBinBound(vX,0.242);
          AddBinBound(vX,0.308);
          break;
        case 12:
          AddBinBound(vX,0.118); // PRL 1D binning
          AddBinBound(vX,0.135);
          AddBinBound(vX,0.150);
          AddBinBound(vX,0.165);
          AddBinBound(vX,0.183);
          AddBinBound(vX,0.200);
          AddBinBound(vX,0.223);
          AddBinBound(vX,0.248);
          AddBinBound(vX,0.275);
          AddBinBound(vX,0.315);
          AddBinBound(vX,0.375);
          break;
        case 18:
          AddBinBound(vX,0.109);
          AddBinBound(vX,0.122);
          AddBinBound(vX,0.133);
          AddBinBound(vX,0.144);
          AddBinBound(vX,0.154);
          AddBinBound(vX,0.165);
          AddBinBound(vX,0.175);
          AddBinBound(vX,0.187);
          AddBinBound(vX,0.199);
          AddBinBound(vX,0.212);
          AddBinBound(vX,0.226);
          AddBinBound(vX,0.242);
          AddBinBound(vX,0.260);
          AddBinBound(vX,0.282);
          AddBinBound(vX,0.308);
          AddBinBound(vX,0.344);
          AddBinBound(vX,0.398);
          break;
        default:
          fprintf(stderr,"ERROR: unknown nb for %s\n",GetIVname(d).Data());
      };
    }
    // -- z --------------
    else if(ivVar[d] == vZ) {
      switch(nb[d]) {
        case 1: break; // single bin
        case 4:
          AddBinBound(vZ,0.468);
          AddBinBound(vZ,0.548);
          AddBinBound(vZ,0.633);
          break;
        case 6:
          AddBinBound(vZ,0.445); // PRL 2D binning
          AddBinBound(vZ,0.500);
          AddBinBound(vZ,0.555);
          AddBinBound(vZ,0.605);
          AddBinBound(vZ,0.675);
          break;
        default:
          fprintf(stderr,"ERROR: unknown nb for %s\n",GetIVname(d).Data());
      };
    }
    // -- PhPerp --------------
    else if(ivVar[d] == vPt) {
      switch(nb[d]) {
        case 1: break; // single bin
        case 4:
          AddBinBound(vPt,0.321);
          AddBinBound(vPt,0.484);
          AddBinBound(vPt,0.662);
          break;
        case 6:
          AddBinBound(vPt,0.245); // PRL 2D binning
          AddBinBound(vPt,0.365);
          AddBinBound(vPt,0.480);
          AddBinBound(vPt,0.585);
          AddBinBound(vPt,0.725);
          break;
        default:
          fprintf(stderr,"ERROR: unknown nb for %s\n",GetIVname(d).Data());
      };
    }
    // -- DY --------------
    else if(ivVar[d] == vDY) {
      switch(nb[d]) {
        case 1: break; // single bin
        case 2:
          AddBinBound(vDY,0.7);
          break;
        case 3:
          AddBinBound(vDY,0.3); // DIS 2D binning
          AddBinBound(vDY,0.7);
          break;
        case 6:
          AddBinBound(vDY,0.128);
          AddBinBound(vDY,0.264);
          AddBinBound(vDY,0.416);
          AddBinBound(vDY,0.604);
          AddBinBound(vDY,0.878);
          break;
        case 12:
          AddBinBound(vDY,0.072);
          AddBinBound(vDY,0.144);
          AddBinBound(vDY,0.217);
          AddBinBound(vDY,0.294);
          AddBinBound(vDY,0.374);
          AddBinBound(vDY,0.460);
          AddBinBound(vDY,0.554);
          AddBinBound(vDY,0.661);
          AddBinBound(vDY,0.789);
          AddBinBound(vDY,0.952);
          AddBinBound(vDY,1.195);
          break;
        default:
          fprintf(stderr,"ERROR: unknown nb for %s\n",GetIVname(d).Data());
      };
    }
    // -- Q2 -------------- // no schemes defined yet
    // -- xF -------------- // no schemes defined yet
    else { printf("default to single bin\n"); };
  };


  // set maximum bin bound
  for(int v=0; v<nIV; v++) AddBinBound(v,maxIV[v]);


  // print which IVs will be analyzed
  printf("--> Binning scheme set for asymmetries vs. %s ",(IVname[ivVar[0]]).Data());
  if(dimensions>=2) printf("in bins of %s ",(IVname[ivVar[1]]).Data());
  if(dimensions>=3) printf("and %s ",(IVname[ivVar[2]]).Data());
  printf("\n\n--> Bin boundaries:\n");
  PrintBinBounds();

  
  // build binArray
  for(int d=0; d<3; d++) {
    binArray[d]->Set(1);
    binArray[d]->Reset(0);
    if(d<dimensions) {
      binArray[d]->Set(GetNbins(d)+1);
      for(int b=0; b<GetNbins(d); b++) {
        if(b==0) binArray[d]->SetAt(bound[ivVar[d]].at(b),b);
        binArray[d]->SetAt(bound[ivVar[d]].at(b+1),b+1);
      };
    };
  };

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
Int_t Binning::GetNbinsTotal() {
  switch(dimensions) {
    case 1:
      return GetNbins(0);
      break;
    case 2:
      return GetNbins(0) * GetNbins(1);
      break;
    case 3:
      return GetNbins(0) * GetNbins(1) * GetNbins(2);
      break;
  };
  return -1;
};
TArrayD * Binning::GetBinArray(Int_t dim) {
  return CheckDim(dim) ? binArray[dim] : nullptr;
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

// 3-tuple of bin numbers -> single 3-digit hexadecimal bin number
Int_t Binning::HashBinNum(Int_t bin0, Int_t bin1, Int_t bin2) {
  Int_t retval = bin0;
  if(bin1>=0) retval += (bin1<<4);
  if(bin2>=0) retval += (bin2<<8);
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
