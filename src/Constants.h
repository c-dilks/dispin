#ifndef CONSTANTS_H_GUARD
#define CONSTANTS_H_GUARD

#include "TString.h"
#include "TMath.h"

// pi
// ---------------------------------------------------
static Double_t PI = TMath::Pi();
static Double_t PIe = TMath::Pi() + 0.3;

// undefined constant
static Double_t UNDEF = -10000;


// useful enumerators
enum xyz_enum { eX, eY, eZ };


// particles constants
// ---------------------------------------------------

// local "particle index" used to address each particle
enum particle_enum {
  kE,
  kP,
  kN,
  kPip,
  kPim,
  kPi0,
  kKp,
  kKm,
  kPhoton,
  kDiph,
  nParticles
};

// particle name
static TString PartName(Int_t p) {
  switch(p) {
    case kE: return "electron";
    case kP: return "proton";
    case kN: return "neutron";
    case kPip: return "piPlus";
    case kPim: return "piMinus";
    case kPi0: return "pi0";
    case kKp: return "KPlus";
    case kKm: return "KMinus";
    case kPhoton: return "photon";
    case kDiph: return "diphoton";
    default: 
      fprintf(stderr,"ERROR: bad PartName request\n");
      return "unknown";
  };
};
static TString PartTitle(Int_t p) {
  switch(p) {
    case kE: return "e^{-}";
    case kP: return "p";
    case kN: return "n";
    case kPip: return "#pi^{+}";
    case kPim: return "#pi^{-}";
    case kPi0: return "#pi^{0}";
    case kKp: return "K^{+}";
    case kKm: return "K^{-}";
    case kPhoton: return "#gamma";
    case kDiph: return "#gamma#gamma";
    default: 
      fprintf(stderr,"ERROR: bad PartTitle request\n");
      return "unknown";
  };
};

// particle PID
static Int_t PartPID(Int_t p) {
  switch(p) {
    case kE: return 11;
    case kP: return 2212;
    case kN: return 2112;
    case kPip: return 211;
    case kPim: return -211;
    case kPi0: return 111;
    case kKp: return 321;
    case kKm: return -321;
    case kPhoton: return 22;
    case kDiph: return UNDEF; // (no PID)
    default: 
      fprintf(stderr,"ERROR: bad PartPID request\n");
      return UNDEF;
  };
};
// convert PID to particle index
static Int_t PIDtoIdx(Int_t pid) {
  for(int i=0; i<nParticles; i++) { if(pid==PartPID(i)) return i; };
  return UNDEF;
};

// mass and charge
static Float_t PartMass(Int_t p) {
  switch(p) {
    case kE: return 0.000511;
    case kP: return 0.938272;
    case kN: return 0.939565;
    case kPip: return 0.139571;
    case kPim: return 0.139571;
    case kPi0: return 0.134977;
    case kKp: return 0.493677;
    case kKm: return 0.493677;
    case kPhoton: return 0.0;
    case kDiph: return UNDEF; // (use Diphoton class instead)
    default: 
      fprintf(stderr,"ERROR: bad PartMass request\n");
      return UNDEF;
  };
};
static Int_t PartCharge(Int_t p) {
  switch(p) {
    case kE: return -1;
    case kP: return 1;
    case kN: return 0;
    case kPip: return 1;
    case kPim: return -1;
    case kPi0: return 0;
    case kKp: return 1;
    case kKm: return -1;
    case kPhoton: return 0;
    case kDiph: return 0;
    default: 
      fprintf(stderr,"ERROR: bad PartCharge request\n");
      return UNDEF;
  };
};

// particle plotting colors
static Int_t PartColor(Int_t p) {
  switch(p) {
    case kE: return kGray+2;
    case kP: return kAzure;
    case kN: return kAzure+10;
    case kPip: return kBlue;
    case kPim: return kRed;
    case kPi0: return kMagenta;
    case kKp: return kGreen+1;
    case kKm: return kGreen-1;
    case kPhoton: return kOrange;
    case kDiph: return kMagenta;
    default: 
      fprintf(stderr,"ERROR: bad PartColor request\n");
      return kBlack;
  };
};
static TString PartColorName(Int_t p) {
  switch(p) {
    case kE: return "grey";
    case kP: return "darkBlue";
    case kN: return "lightBlue";
    case kPip: return "blue";
    case kPim: return "red";
    case kPi0: return "magenta";
    case kKp: return "lightGreen";
    case kKm: return "darkGreen";
    case kPhoton: return "orange";
    case kDiph: return "magenta";
    default: 
      fprintf(stderr,"ERROR: bad PartColor request\n");
      return "black";
  };
};



// observables 
// ---------------------------------------------------
// - these are the particles that we observe for analysis, for example, for 
//   making dihadron pairs (excludes DIS electron, which is already
//   a default observable)

// observable index
enum observable_enum {
  sPip,
  sPim,
  sDiph,
  //sKp,
  //sKm,
  nObservables
};

// convert observable index -> particle index
static Int_t OI(Int_t s) {
  switch(s) {
    case sPip: return kPip;
    case sPim: return kPim;
    case sDiph: return kDiph;
    //case sDiph: return kPi0; // (for if pi0s are found in HIPO files)
    //case sKp: return kKp;
    //case sKm: return kKm;
    default: 
      fprintf(stderr,"ERROR: bad OI request (see Constants.h)\n");
      return UNDEF;
  };
};
// convert particle index -> observable index
static Int_t IO(Int_t s) {
  switch(s) {
    case kPip: return sPip;
    case kPim: return sPim;
    case kDiph: return sDiph; 
    //case kPi0: return sDiph; // (for if pi0s are found in HIPO files)
    //case kKp: return sKp;
    //case kKm: return sKm;
    default: 
      fprintf(stderr,"ERROR: bad IO request (see Constants.h)\n");
      return UNDEF;
  };
};

// observable names
static TString ObsName(Int_t s) { return PartName(OI(s)); };
static TString ObsTitle(Int_t s) { return PartTitle(OI(s)); };


// the pi0 observable is really a diphoton; thus we allow freedom to change
// the name "pi0" to "background" when analysin background events
static void TransformNameBG(TString & str) {
  str.ReplaceAll(PartName(kPi0),"diphBG");
};
static void TransformTitleBG(TString & str) {
  str.ReplaceAll(PartTitle(kPi0),"#gamma#gamma_{BG}");
};




// dihadrons
// ---------------------------------------------------

// hadron pair (A,B) is equivalent to (B,A); to give them a unique name, define
// the following pair-ordering convention of hadron pairs with enumerators qA and qB:
//    - if charges are different: qA has higher charge than qB
//    - if charges are equal:
//      - if particles are different: qA has higher mass than qB
//      - if particles are same: indistinguishable and order doesn't matter
// -- ordering is implemented in dihHadIdx below
// -- pairs of numbers can be compared using Tools::PairSame, which enforces the
//    equivalence (A,B)==(B,A)
enum pair_enum { qA, qB };

// return the ordered hadron particle index within the dihadron pair, where "idx"
// represents either the first or second hadron (idx==qA or qB, respectively); 
// example: dihHadIdx(kPim,kPip,qA) returns kPip
static Int_t dihHadIdx(Int_t p1, Int_t p2, Int_t idx) {
  if(p1==p2) {
    switch(idx) {
      case qA: return p1;
      case qB: return p2;
    };
  } else {

    if(idx==qA) {
      if( PartCharge(p1) == PartCharge(p2) ) {
        return PartMass(p1) >= PartMass(p2) ? p1 : p2;
      } else {
        return PartCharge(p1) > PartCharge(p2) ? p1 : p2;
      };
    }

    else if(idx==qB) {
      if( PartCharge(p1) == PartCharge(p2) ) {
        return PartMass(p1) < PartMass(p2) ? p1 : p2;
      } else {
        return PartCharge(p1) < PartCharge(p2) ? p1 : p2;
      };
    };

  };
  fprintf(stderr,"ERROR: bad dihHadIdx request\n");
  return UNDEF;
};


// shorthand for referring to a pair is called "pairType", defined as a
// 2-digit hex number, where each hex digit is the particle index
// convert whichPair hex number to hadron indices
// -- whichPair is a 2-digit hex number, where each digit represents 
//    the particle index of the hadron
// -- hex is used since each digit ranges 16 values (0-F), allowing space
//    for 16 particles
static void DecodePairType(Int_t w, Int_t & ha, Int_t & hb) {
  ha = w & 0xF;
  hb = w>>4 & 0xF;
};
// convert hadron indices to whichPair
static Int_t EncodePairType(Int_t ha, Int_t hb) {
  return (ha<<4) + hb;
};


// how to loop through unique pairs:
// IterPair converts observable indices (a,b) to particle indices (p_a,p_b);
// additionally, if the pair-ordering matches the unique ordering, return true
   /*
   for(a=0; a<nObservables; a++) {
     for(b=0; b<nObservables; b++) {
       if(IterPair(a,b,p_a,p_b)) {
         // analyse pair with particle indices p_a, p_b
       };
     };
   };
   */
static Bool_t IterPair(Int_t a, Int_t b, Int_t & p_a, Int_t & p_b) {
  p_a = OI(a);
  p_b = OI(b);
  return p_a==dihHadIdx(p_a,p_b,qA) && p_b==dihHadIdx(p_a,p_b,qB);
};


// name of hadron within the pair
static TString PairHadName(Int_t p1, Int_t p2, Int_t h) {
  TString ret = PartName(dihHadIdx(p1,p2,h));
  if(p1==p2) ret = Form("%s%d",ret.Data(),h+1);
  return ret;
};
static TString PairHadTitle(Int_t p1, Int_t p2, Int_t h) {
  TString ret = PartTitle(dihHadIdx(p1,p2,h));
  if(ret=="#gamma#gamma") ret="#pi^{0}"; // change \gamma\gamma to \pi^0
  if(p1==p2) ret = Form("%s_{%d}",ret.Data(),h+1);
  return ret;
};


// name of pair
static TString PairName(Int_t p1, Int_t p2) {
  return TString( PairHadName(p1,p2,qA) + "_" + PairHadName(p1,p2,qB) );
};
static TString PairName(Int_t ptype) {
  Int_t h1,h2;
  DecodePairType(ptype,h1,h2);
  return PairName(h1,h2);
};
// title of pair
static TString PairTitle(Int_t p1, Int_t p2) {
  return TString( PairHadTitle(p1,p2,qA) + PairHadTitle(p1,p2,qB) );
};
static TString PairTitle(Int_t ptype) {
  Int_t h1,h2;
  DecodePairType(ptype,h1,h2);
  return PairTitle(h1,h2);
};
  


// spin 
// ---------------------------------------------------

// spin index
enum spinEnum { sP, sM, nSpin };

// spin names
static TString SpinName(Int_t s) {
  switch(s) {
    case sP: return "P";
    case sM: return "M";
    default:
      fprintf(stderr,"ERROR: bad SpinName request\n");
      return "unknown";
  };
};
static TString SpinSign(Int_t s) {
  switch(s) {
    case sP: return "+";
    case sM: return "-";
    default:
      fprintf(stderr,"ERROR: bad SpinSign request\n");
      return "unknown";
  };
};
static TString SpinTitle(Int_t s) {
  return "spin " + SpinSign(s);
};


#endif
