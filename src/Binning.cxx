#include "Binning.h"

ClassImp(Binning)

using namespace std;


Binning::Binning() {
  //printf("Instantiating Binning...\n");

  // set minimum and maximum IV values
  minIV[vM] = 0;   maxIV[vM] = 3;
  minIV[vX] = 0;   maxIV[vX] = 1;
  minIV[vZ] = 0;   maxIV[vZ] = 1;
  minIV[vPt] = 0;  maxIV[vPt] = 3;
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

  IVoutrootBranchName[vM] = "Mh";
  IVoutrootBranchName[vX] = "x";
  IVoutrootBranchName[vZ] = "Zpair";
  IVoutrootBranchName[vPt] = "PhPerp";
  IVoutrootBranchName[vDY] = "unknown"; // TODO
  IVoutrootBranchName[vQ] = "Q2";
  IVoutrootBranchName[vXF] = "xF";

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
  schemeVersion = "PM"; // default scheme version
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
  for(int d=0; d<3; d++) ivVal[d] = -1000;
  Int_t ivBin[3] = {-1,-1,-1};
  for(int d=0; d<dimensions; d++) {
    switch(ivVar[d]) {
      case vM:  ivVal[d] = ev->Mh;     break;
      case vX:  ivVal[d] = ev->x;      break;
      case vZ:  ivVal[d] = ev->Zpair;  break;
      case vPt: ivVal[d] = ev->PhPerp; break;
      case vDY: ivVal[d] = ev->DY;     break;
      case vQ:  ivVal[d] = ev->Q2;     break;
      case vXF: ivVal[d] = ev->xF;     break;
      default: 
                         fprintf(stderr,"ERROR: bad ivVar\n");
                         return -1;
    };
    ivBin[d] = this->GetBin(ivVar[d],ivVal[d]);
    if(ivBin[d]<0) return -1;
  };
  return this->HashBinNum(ivBin[0],ivBin[1],ivBin[2]);
};
// analogous method for generated kinematics
Int_t Binning::FindBinGen(EventTree * ev) {
  for(int d=0; d<3; d++) ivValGen[d] = -1000;
  Int_t ivBin[3] = {-1,-1,-1};
  for(int d=0; d<dimensions; d++) {
    switch(ivVar[d]) {
      case vM:  ivValGen[d] = ev->gen_Mh;     break;
      case vX:  ivValGen[d] = ev->gen_x;      break;
      case vZ:  ivValGen[d] = ev->gen_Zpair;  break;
      case vPt: ivValGen[d] = ev->gen_PhPerp; break;
      case vDY: //ivValGen[d] = ev->gen_DY;     break;
                fprintf(stderr,"ERROR: missing EventTree::gen_DY (TODO)\n");
                return -1;
                break;
      case vQ:  ivValGen[d] = ev->gen_Q2;     break;
      case vXF: ivValGen[d] = ev->gen_xF;     break;
      default: 
                         fprintf(stderr,"ERROR: bad ivVar\n");
                         return -1;
    };
    for(int d=0; d<dimensions; d++) if(ivValGen[d]==UNDEF) return -1; // silent return for generated bin not found
    ivBin[d] = this->GetBin(ivVar[d],ivValGen[d]);
    if(ivBin[d]<0) return -1;
  };
  return this->HashBinNum(ivBin[0],ivBin[1],ivBin[2]);
};

// get bin associated with specified values, using current binning scheme
Int_t Binning::FindBin(Float_t iv0, Float_t iv1, Float_t iv2) {
  ivVal[0] = iv0;
  ivVal[1] = iv1;
  ivVal[2] = iv2;
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


Bool_t Binning::SetScheme(Int_t pairType, Int_t ivType, Int_t nb0, Int_t nb1, Int_t nb2) {

  // set whichHad from pairType
  DecodePairType(pairType,whichHad[qA],whichHad[qB]);

  // set scheme version from pairType
  if( whichHad[qA]==kDiphBasic || whichHad[qB]==kDiphBasic ) schemeVersion="PI0"; // if there's a diphoton, assume pi+pi0 or pi-pi0
  else schemeVersion="PM"; // otherwise assume pi+pi-
  printf("\n\nBinning SCHEME VERSION = %s\n\n",schemeVersion.Data());
  // print scheme version
  if(schemeVersion=="PM")       printf("Binning scheme version set for pi+pi- analysis for RGA vs. RGB\n");
  else if(schemeVersion=="PI0") printf("Binning scheme version set for pi+pi0 and pi-pi0 analyses for RGA vs. RGB\n");
  else if(schemeVersion=="DIS") printf("Binning scheme version set for pi+pi- analysis for DIS2021\n");
  else if(schemeVersion=="PRL") printf("Binning scheme version set for pi+pi- analysis for PRL arXiv:2101.04842\n");
  else {
    fprintf(stderr,"ERROR: unknown schemeVersion %s; setting to default value\n",schemeVersion.Data());
    schemeVersion="PM";
  };

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
  // - the more up-to-date schemes are maintained in DatasetLooper.rb
  Int_t nb[3];
  if( nb0==-1 && nb1==-1 && nb2==-1) {
    if(schemeVersion=="PM" || schemeVersion=="PI0") { // pi+pi-, pi+pi0, pi-pi0, for RGA vs. RGB
      if(ivVar[0]==vX) nb0=6;
      if(ivVar[0]==vM) nb0=6;
      if(ivVar[0]==vZ  && ivVar[1]==vM)  { nb0=3; nb1=2; };
      if(ivVar[0]==vPt && ivVar[1]==vM)  { nb0=3; nb1=2; };
    };
    if(schemeVersion=="DIS") { // DIS2021: RGA fa18+sp19 inbending data
      if(ivVar[0]==vX) nb0=12;
      if(ivVar[0]==vM) nb0=12;
      if(ivVar[0]==vZ  && ivVar[1]==vM)  { nb0=4; nb1=3; };
      if(ivVar[0]==vPt && ivVar[1]==vM)  { nb0=4; nb1=3; };
      if(ivVar[0]==vX  && ivVar[1]==vDY) { nb0=4; nb1=3; };
      if(ivVar[0]==vM  && ivVar[1]==vDY) { nb0=4; nb1=3; };
      if(ivVar[0]==vX  && ivVar[1]==vM && ivVar[2]==vDY) { nb0=3; nb1=2; nb2=2; };
      if(ivVar[0]==vDY) nb0=12;
    };
    if(schemeVersion=="PRL") { // PRL: RGA fa18 inbending data
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
          AddBinBound(vM,0.63); // PRL 2D bining
          // AddBinBound(vM,0.95); // test >M_rho region
          break;
        case 3:
          AddBinBound(vM,0.60); // DIS 2D binning
          AddBinBound(vM,0.95);
          break;
        case 6:
          if(schemeVersion=="PM") { // based on RGA bibending pi+pi-
            AddBinBound(vM,0.473);
            AddBinBound(vM,0.627);
            AddBinBound(vM,0.751);
            AddBinBound(vM,0.856);
            AddBinBound(vM,1.011);
          } else if(schemeVersion=="PI0") { // based on RGA bibending pi+pi0
            AddBinBound(vM,0.404);
            AddBinBound(vM,0.518);
            AddBinBound(vM,0.653);
            AddBinBound(vM,0.777);
            AddBinBound(vM,0.930);
          } else { // old pi+pi-
            AddBinBound(vM,0.488);
            AddBinBound(vM,0.654);
            AddBinBound(vM,0.774);
            AddBinBound(vM,0.895);
            AddBinBound(vM,1.075);
          };
          break;
        case 12:
          AddBinBound(vM,0.381); // PRL 1D binning // DIS 1D binning
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
        default:
          fprintf(stderr,"ERROR: unknown nb for %s\n",GetIVname(d).Data());
      };
    }
    // -- x --------------
    else if(ivVar[d] == vX) {
      switch(nb[d]) {
        case 1: break; // single bin
        case 3:
          AddBinBound(vX,0.158); // based on RGA pi+pi0 bibending
          AddBinBound(vX,0.246);
          break;
        case 4:
          AddBinBound(vX,0.149); // DSIDIS 2D binning
          AddBinBound(vX,0.199);
          AddBinBound(vX,0.270);
          break;
        case 6:
          if(schemeVersion=="PM") { // based on RGA bibending pi+pi-
            AddBinBound(vX,0.109);
            AddBinBound(vX,0.139);
            AddBinBound(vX,0.173);
            AddBinBound(vX,0.216);
            AddBinBound(vX,0.283);
          } else if(schemeVersion=="PI0") { // based on RGA bibending pi+pi0
            AddBinBound(vX,0.124);
            AddBinBound(vX,0.158);
            AddBinBound(vX,0.197);
            AddBinBound(vX,0.246);
            AddBinBound(vX,0.322);
          } else { // old pi+pi-
            AddBinBound(vX,0.133);
            AddBinBound(vX,0.165);
            AddBinBound(vX,0.199);
            AddBinBound(vX,0.242);
            AddBinBound(vX,0.308);
          };
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
        default:
          fprintf(stderr,"ERROR: unknown nb for %s\n",GetIVname(d).Data());
      };
    }
    // -- z --------------
    else if(ivVar[d] == vZ) {
      switch(nb[d]) {
        case 1: break; // single bin
        case 3:
          if(schemeVersion=="PM") { // based on RGA bibending pi+pi-
            AddBinBound(vZ,0.512);
            AddBinBound(vZ,0.619);
          } else if(schemeVersion=="PI0") { // based on RGA bibending pi+pi0
            AddBinBound(vZ,0.507);
            AddBinBound(vZ,0.625);
          } else { // old pi+pi-
            AddBinBound(vZ,0.510);
            AddBinBound(vZ,0.614);
          };
          break;
        case 4:
          AddBinBound(vZ,0.468); // DIS2021 2D binning
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
        case 3:
          if(schemeVersion=="PM") { // based on RGA bibending pi+pi-
            AddBinBound(vPt,0.368);
            AddBinBound(vPt,0.586);
          } else if(schemeVersion=="PI0") { // based on RGA bibending pi+pi0
            AddBinBound(vPt,0.358);
            AddBinBound(vPt,0.578);
          } else { // old pi+pi-
            AddBinBound(vPt,0.369);
            AddBinBound(vPt,0.584);
          };
          break;
        case 4:
          AddBinBound(vPt,0.321); // DIS2021 2D binning
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
          AddBinBound(vDY,0.3); // DSIDIS 2D binning
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
Int_t Binning::GetNbinsHighDim() {
  switch(dimensions) {
    case 1:
      return 1;
      break;
    case 2:
      return GetNbins(1);
      break;
    case 3:
      return GetNbins(1) * GetNbins(2);
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
TString Binning::GetIVoutrootBranchName(Int_t dim) {
  return CheckDim(dim) ? IVoutrootBranchName[ivVar[dim]] : "unknown";
};
Float_t Binning::GetIVmin(Int_t dim) { return CheckDim(dim) ? minIV[ivVar[dim]] : UNDEF; };
Float_t Binning::GetIVmax(Int_t dim) { return CheckDim(dim) ? maxIV[ivVar[dim]] : UNDEF; };
Float_t Binning::GetIVval(Int_t dim) { return CheckDim(dim) ? ivVal[dim] : UNDEF; };

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

// 3-digit hex bin number -> brufit bin numbers, which are:
// - I = bin number for IV0, minus 1, (TGraph point #)
// - BL = Bin List number, used in brufit draw code
void Binning::BinNumToIBL(Int_t bn_, Int_t &I_, Int_t &BL_) {
  Int_t bb[3] = {-1,-1,-1};
  for(int d=0; d<dimensions; d++) bb[d] = this->UnhashBinNum(bn_,d);
  I_ = bb[0];
  switch(dimensions) {
    case 1: BL_ = 0; break;
    case 2: BL_ = bb[1]; break;
    case 3: BL_ = (bb[2])*this->GetNbins(1) + bb[1]; break;
    default:
      fprintf(stderr,"ERROR: bad dimensions in BinNumToIBL\n");
      BL_ = -1;
  };
};

Binning::~Binning() {};
