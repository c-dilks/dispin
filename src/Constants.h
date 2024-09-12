#ifndef CONSTANTS_H_GUARD
#define CONSTANTS_H_GUARD

#include "TString.h"
#include "TMath.h"

// pi
// ---------------------------------------------------
static const Double_t PI = TMath::Pi();
static const Double_t PIe = TMath::Pi() + 0.3;

// undefined constant
static const Double_t UNDEF = -10000;

// special run numbers
static const Int_t RUNNUM_MC = 11; // TODO: when we have real run numbers, this will change!
static const Int_t RUNNUM_STRING_SPINNER = 15;

// default beam energy
static const Float_t DEFAULT_BEAM_ENERGY = 10.6041; // default (RGA Fall18)

// useful enumerators
enum xyz_enum { eX, eY, eZ };


// particles constants
// ---------------------------------------------------

// local "particle index" used to address each particle
// - some names are chosen to avoid conflicts with ROOT...
enum particle_enum {
  kE,
  kP,
  kN,
  kPip,
  kPim,
  kPio,
  kKp,
  kKm,
  kAP,
  kPhoton,
  kDiph,
  kDiphBasic,
  kPioBG,
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
    case kPio: return "pi0";
    case kKp: return "KPlus";
    case kKm: return "KMinus";
    case kAP: return "antiproton";
    case kPhoton: return "photon";
    case kDiph: return "diphoton";
    case kDiphBasic: return "diphotonBasic";
    case kPioBG: return "pi0bg";
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
    case kPio: return "#pi^{0}";
    case kKp: return "K^{+}";
    case kKm: return "K^{-}";
    case kAP: return "p^{-}";
    case kPhoton: return "#gamma";
    case kDiph: return "#gamma#gamma";
    case kDiphBasic: return "#gamma#gamma_{basic}";
    case kPioBG: return "#pi^{0}_{BG}";
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
    case kPio: return 111;
    case kKp: return 321;
    case kKm: return -321;
    case kAP: return -2212;
    case kPhoton: return 22;
    case kDiph: return UNDEF; // (no PID)
    case kDiphBasic: return UNDEF; // (no PID)
    case kPioBG: return UNDEF; // (no PID)
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
    case kPio: return 0.134977; // (true mass, see Diphoton class)
    case kKp: return 0.493677;
    case kKm: return 0.493677;
    case kAP: return 0.938272;
    case kPhoton: return 0.0;
    case kDiph: return UNDEF; // (use Diphoton class instead)
    case kDiphBasic: return UNDEF; // (use Diphoton class instead)
    case kPioBG: return UNDEF; // (use Diphoton class instead)
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
    case kPio: return 0;
    case kKp: return 1;
    case kKm: return -1;
    case kAP: return -1;
    case kPhoton: return 0;
    case kDiph: return 0;
    case kDiphBasic: return 0;
    case kPioBG: return 0;
    default: 
      fprintf(stderr,"ERROR: bad PartCharge request\n");
      return UNDEF;
  };
};

// particle plotting colors
static Int_t PartColor(Int_t p) {
  switch(p) {
    case kE: return kGray+2;
    case kP: return kBlack;
    case kN: return kAzure+10;
    case kPip: return kBlue;
    case kPim: return kRed;
    case kPio: return kMagenta;
    case kKp: return kGreen+1;
    case kKm: return kGreen-1;
    case kAP: return kAzure;
    case kPhoton: return kOrange;
    case kDiph: return kMagenta;
    case kDiphBasic: return kMagenta;
    case kPioBG: return kViolet;
    default: 
      fprintf(stderr,"ERROR: bad PartColor request\n");
      return kBlack;
  };
};
static TString PartColorName(Int_t p) {
  switch(p) {
    case kE: return "grey";
    case kP: return "black";
    case kN: return "lightBlue";
    case kPip: return "blue";
    case kPim: return "red";
    case kPio: return "magenta";
    case kKp: return "lightGreen";
    case kKm: return "darkGreen";
    case kAP: return "darkBlue";
    case kPhoton: return "orange";
    case kDiph: return "magenta";
    case kDiphBasic: return "magenta";
    case kPioBG: return "violet";
    default: 
      fprintf(stderr,"ERROR: bad PartColor request\n");
      return "black";
  };
};



// observables -- NOTE: may be deprecated, as these enumerators were used
//                only for the old pairing algorithm
// ---------------------------------------------------
// - these are the particles that we observe for analysis, for example, for 
//   making dihadron pairs (excludes DIS electron, which is already
//   a default observable)

// observable index
enum observable_enum {
  sPip,
  sPim,
  sPio,
  sPioBG,
  sKp,
  sKm,
  sProt,
  sAP,
  nObservables
};

// convert observable index -> particle index
static Int_t OI(Int_t s) {
  switch(s) {
    case sPip: return kPip;
    case sPim: return kPim;
    case sPio: return kPio;
    case sPioBG: return kPioBG;
    case sKp: return kKp;
    case sKm: return kKm;
    case sProt: return kP;
    case sAP: return kAP;
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
    case kPio: return sPio;
    case kPioBG: return sPioBG;
    case kKp: return sKp;
    case kKm: return sKm;
    case kP: return sProt;
    case kAP: return sAP;
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
  str.ReplaceAll(PartName(kPio),"diphBG");
};
static void TransformTitleBG(TString & str) {
  str.ReplaceAll(PartTitle(kPio),"#gamma#gamma_{BG}");
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

// return true if Idx pair (ia,ib) is in the proper order
static Bool_t CorrectOrder(Int_t ia, Int_t ib) {
  return ia==dihHadIdx(ia,ib,qA) && ib==dihHadIdx(ia,ib,qB);
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
  return CorrectOrder(p_a,p_b);
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
static Int_t SpinInt(Int_t s) {
  switch(s) {
    case sP: return 1;
    case sM: return -1;
    default:
      fprintf(stderr,"ERROR: bad SpinInt request\n");
      return 0;
  };
};
static TString SpinTitle(Int_t s) {
  return "spin " + SpinSign(s);
};


// run dependent settings and numbers
//---------------------------------------------------

// torus setting
enum torus_enum {
  kInbending = -1,
  kOutbending = 1
};
// get torus, given runnum; if MC the runnum is always RUNNUM_MC, so use fileName instead
static Int_t RundepTorus(Int_t run, TString fileName="") {
  if     (run>= 5032 && run<= 5419) return kInbending;  // rga_inbending_fa18
  else if(run>= 5422 && run<= 5666) return kOutbending; // rga_outbending_fa18
  else if(run>= 6616 && run<= 6783) return kInbending;  // rga_inbending_sp19
  else if(run>= 6156 && run<= 6603) return kInbending;  // rgb_inbending_sp19
  else if(run>=11093 && run<=11283) return kOutbending; // rgb_outbending_fa19
  else if(run>=11284 && run<=11300) return kInbending;  // rgb_BAND_inbending_fa19
  else if(run>=11323 && run<=11571) return kInbending;  // rgb_inbending_wi20
  else if(run==16771) return kInbending;
  else if(run==RUNNUM_MC) {
    if(fileName.Contains("inbending")) return kInbending;
    else if(fileName.Contains("outbending")) return kOutbending;
    else {
      fprintf(stderr,"ERROR: RundepTorus file name or path does not contain 'inbending' or 'outbending' (necessary if runnum==%d)\n", RUNNUM_MC);
      return kInbending;
    }
  }
  else if(run==RUNNUM_STRING_SPINNER) return kInbending;
  else {
    fprintf(stderr,"ERROR: RundepTorus unknown for run %d\n",run);
    return kInbending;
  };
};


// beam energy
static Float_t RundepBeamEn(Int_t run) {
  if     (run>= 5032 && run<= 5666) return 10.6041; // rga fall 18
  else if(run>= 6616 && run<= 6783) return 10.1998; // rga spring 19
  else if(run>= 6120 && run<= 6399) return 10.5986; // rgb spring 19
  else if(run>= 6409 && run<= 6604) return 10.1998; // rgb spring 19
  else if(run>=11093 && run<=11283) return 10.4096; // rgb fall 19
  else if(run>=11284 && run<=11300) return 4.17179; // rgb fall BAND_FT 19
  else if(run>=11323 && run<=11571) return 10.3894; // rgb winter 20 (RCDB may still be incorrect)
  else if(run==16771) return 10.5473;
  else if(run==RUNNUM_MC)             return DEFAULT_BEAM_ENERGY; // MC for RGA inbending
  else if(run==RUNNUM_STRING_SPINNER) return DEFAULT_BEAM_ENERGY; // StringSpinner
  else {
    fprintf(stderr,"ERROR: RundepBeamEn unknown for run %d\n",run);
    return 0.0;
  };
};


// helicity flip
static Bool_t RundepHelicityFlip(Int_t run) {
  if     (run>= 5032 && run<= 5666) return true;  // rga fall 18
  else if(run>= 6616 && run<= 6783) return true;  // rga spring 19
  else if(run>= 6120 && run<= 6604) return true;  // rgb spring 19
  else if(run>=11093 && run<=11283) return false; // rgb fall 19
  else if(run>=11284 && run<=11300) return true;  // rgb fall BAND_FT 19
  else if(run>=11323 && run<=11571) return false; // rgb winter 20
  else if(run==16771) return false;
  else {
    fprintf(stderr,"ERROR: RundepHelicityFlip unknown for run %d\n",run);
    return false;
  };
};


// polarization
// - set `v` to `false` to return polarization error
static Float_t RundepPolarization(Int_t run, Bool_t v=true) {
  /* RGA */
  if     (run>= 5032 && run<= 5332) return v? 0.8592 : 0.01290; // rga_fa18, before Wien angle change
  else if(run>= 5333 && run<= 5666) return v? 0.8922 : 0.02509; // rga_fa18, after Wien angle change
  else if(run>= 6616 && run<= 6783) return v? 0.8453 : 0.01474; // rga_sp19 https://logbooks.jlab.org/entry/3677077
  /* RGB */
  else if(run>= 6142 && run<= 6149) return v? 0.81132 : 0.01505; // rgb_sp19
  else if(run>= 6150 && run<= 6188) return v? 0.82137 : 0.01491; // https://clasweb.jlab.org/wiki/images/c/ca/Moller_Runs_15Jan.pdf
  else if(run>= 6189 && run<= 6260) return v? 0.83598 : 0.01475;
  else if(run>= 6261 && run<= 6339) return v? 0.80770 : 0.01449;
  else if(run>= 6340 && run<= 6342) return v? 0.85536 : 0.01484;
  else if(run>= 6344 && run<= 6399) return v? 0.87038 : 0.01474;
  else if(run>= 6420 && run<= 6476) return v? 0.88214 : 0.01502;
  else if(run>= 6479 && run<= 6532) return v? 0.86580 : 0.01460;
  else if(run>= 6533 && run<= 6603) return v? 0.87887 : 0.01454;
  else if(run>=11013 && run<=11309) return v? 0.84983 : 0.02929; // rgb_fa19
  else if(run>=11323 && run<=11334) return v? 0.87135 : 0.01464; // rgb_wi20
  else if(run>=11335 && run<=11387) return v? 0.85048 : 0.01530;
  else if(run>=11389 && run<=11571) return v? 0.84262 : 0.01494; // NOTE: table last updated 1/15/2020, but run ended on 1/30
  /* RGC test */
  else if(run==16771) return v? 0.80 : 0.02; // just a guess
  /* MC */
  else if(run==RUNNUM_MC)             return v? 0.86 : 0.0; // MC
  else if(run==RUNNUM_STRING_SPINNER) return v? 1.00 : 0.0; // StringSpinner
  else {
    fprintf(stderr,"ERROR: RundepPolarization unknown for run %d\n",run);
    return 0.0;
  };
};


// brufit
// ---------------------------------------------------
enum minimEnum { mkMCMC, mkMCMCseq, mkMCMCcov, mkMinuit };

// convert minimizer string to an enumerator
static Int_t MinimizerStrToEnum(TString str) {
  if(str.CompareTo("mcmc",TString::kIgnoreCase)==0)    return mkMCMC;    // sequential, no acceptance rate locking
  if(str.CompareTo("mcmcseq",TString::kIgnoreCase)==0) return mkMCMCseq; // sequential, with acceptance rate locking
  if(str.CompareTo("mcmccov",TString::kIgnoreCase)==0) return mkMCMCcov; // sequential + covariant
  if(str.CompareTo("minuit",TString::kIgnoreCase)==0)  return mkMinuit;  // minuit MIGRAD
  fprintf(stderr,"ERROR: unknown minimizer type\n");
  return -1;
};

// return brufit results file name, given minimizer enum
static TString BrufitResultsFileName(Int_t minimizer) {
  switch(minimizer) {
    case mkMCMC:    return "ResultsHSRooMcmcSeq.root";        break;
    case mkMCMCseq: return "ResultsHSRooMcmcSeqHelper.root";  break;
    case mkMCMCcov: return "ResultsHSRooMcmcSeqThenCov.root"; break;
    case mkMinuit:  return "ResultsHSMinuit2.root";           break;
    default:
                    fprintf(stderr,"ERROR: unknown minimizer in Constants::BrufitResultsFileName\n");
                    return "UNKNOWN";
  };
};

// return true if minimizer is MCMC
static Bool_t IsMCMC(Int_t minimizer) {
  switch(minimizer) {
    case mkMCMC:    return true;  break;
    case mkMCMCseq: return true;  break;
    case mkMCMCcov: return true;  break;
    case mkMinuit:  return false; break;
    default:
                    fprintf(stderr,"ERROR: unknown minimizer in Constants::IsMCMC\n");
                    return "UNKNOWN";
  };
};

#endif
