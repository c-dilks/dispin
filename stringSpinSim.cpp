// copied from https://gitlab.com/albikerbizi/stringspinner.git `dis.cc` (locally deps/StringSpinner/dis.cc)
// and modified for usage in this analysis

# include <vector>

// pythia, stringspinner
#include "Pythia8/Pythia.h"
#include "StringSpinner.h"

// ROOT
#include <TFile.h>
#include <TTree.h>

// dispin
#include "Constants.h"
#include "Tools.h"

using namespace Pythia8;

TString GetModeStr(Int_t mode);

/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////

int main(int argc, char** argv) {

  // arguments
  Int_t mode       = 0;
  TString outFileN = Form("sss/%d.root",mode);
  int seed         = -1;
  int pairType     = EncodePairType(kPip,kPim);
  int stringSelection[2]         = {2, 2101}; // u === (ud)_0
  TString stringSelectionDesc[2] = {"u", "(ud)_0"};
  if(argc==1) {
    cerr << endl << "USAGE: " << argv[0] << " [ARGUMENTS]..." << endl;
    cerr << endl << "ARGUMENTS:" << endl;
    cerr << " [numEvents]" << endl;
    cerr << " [mode (default=" << mode << ")]" << endl;
    cerr << " [outputFileName (default=" << outFileN << ")]" << endl;
    cerr << " [pairType (default=0x" << std::hex << pairType << std::dec << ")]" << endl;
    cerr << " [stringSelection0 (default=" << stringSelection[0] << " for '" << stringSelectionDesc[0] << "')]" << endl;
    cerr << " [stringSelection1 (default=" << stringSelection[1] << " for '" << stringSelectionDesc[1] << "')]" << endl;
    cerr << " [seed (default=" << seed << ")]" << endl;
    cerr << endl << "MODES: " << endl;
    for(int m=0; m<4; m++) cerr << "  " << m << ": " << GetModeStr(m) << endl;
    cerr << endl;
    cerr << "NOTE: set stringSelection arguments both to 0 to disable filtering of events by strings" << endl;
    cerr << endl;
    cerr << "See also HARD-CODED SETTINGS in the source file" << endl << endl;
    return 2;
  }
  Long64_t nEvent;
  int ai = 1;
  if(argc>ai) nEvent             = (Long64_t)strtof(argv[ai++],NULL);
  if(argc>ai) mode               = (Int_t)strtof(argv[ai++],NULL);
  if(argc>ai) outFileN           = TString(argv[ai++]);
  if(argc>ai) pairType           = (int)strtof(argv[ai++],NULL);
  if(argc>ai) stringSelection[0] = (int)strtof(argv[ai++],NULL);
  if(argc>ai) stringSelection[1] = (int)strtof(argv[ai++],NULL);
  if(argc>ai) seed               = (int)strtof(argv[ai++],NULL);
  TString modeStr = GetModeStr(mode);
  if(modeStr.Contains("UNKNOWN")) return 1;
  cout << "ARGS: "        << endl;
  cout << "  nEvent           = " << nEvent   << endl;
  cout << "  mode             = " << mode     << ": " << modeStr << endl;
  cout << "  outFileN         = " << outFileN << endl;
  cout << "  pairType         = " << "0x" << std::hex << pairType << std::dec << endl;
  cout << "  stringSelection0 = " << stringSelection[0] << endl;
  cout << "  stringSelection1 = " << stringSelection[1] << endl;
  cout << "  seed             = " << seed     << endl;

  bool useStringSelection = stringSelection[0]!=0 && stringSelection[1]!=0;

  Pythia pythia;
  Event& EV = pythia.event;
  auto fhooks = std::make_shared<SimpleStringSpinner>();
  fhooks->plugInto(pythia);

  // HARD-CODED SETTINGS ////////////////////////////////////////////
  //// incoming particles
  ParticleData& pdt = pythia.particleData;
  double eNucleon  = pdt.m0(2212); // proton beam energy (target mass)
  double pLepton   = DEFAULT_BEAM_ENERGY; // electron beam momentum
  double eLepton   = Tools::PMtoE(pLepton, pdt.m0(11)); // electron beam energy
  //// phase space
  double Q2min     = 1.0; // minimum Q2
  ///////////////////////////////////////////////////////////////////

  // Seed
  pythia.readString("Random:setSeed = on");
  pythia.settings.parm("Random:seed",seed);

  // Set up incoming beams, for frame with unequal beam energies.
  pythia.readString("Beams:frameType = 2");   // the beams are back-to-back, but with different energies
  pythia.readString("Beams:idA = 11");        // BeamA = electron
  pythia.readString("Beams:idB = 2212");      // BeamB = proton
  pythia.settings.parm("Beams:eA", eLepton);  // BeamA energy
  pythia.settings.parm("Beams:eB", eNucleon); // BeamB energy

  // Interaction mechanism.
  pythia.readString("WeakBosonExchange:ff2ff(t:gmZ) = on"); // $\gamma*/Z^0$ $t$-channel exchange, with full interference between $\gamma*$ and $Z^0$

  // Phase-space cut: minimal Q2 of process.
  pythia.settings.parm("PhaseSpace:Q2Min", Q2min);

  // Go down to low x-Bjorken.
  pythia.readString("PhaseSpace:pTHatMinDiverge = 0.5"); // extra $p_T$ cut to avoid divergences of some processes in the $p_T \to 0$ limit
  pythia.readString("PhaseSpace:mHatMin = 0.");          // minimum invariant mass

  // Set dipole recoil on. Necessary for DIS + shower.
  pythia.readString("SpaceShower:dipoleRecoil = off");

  // QED radiation off lepton not handled yet by the new procedure.
  // these are recommended when `SpaceShower:dipoleRecoil = on`
  pythia.readString("PDF:lepton = off");              // do not use parton densities for lepton beams
  pythia.readString("TimeShower:QEDshowerByL = off"); // disallow leptons to radiate photons

  // Choice of PDF = CTEQ5L LO (pSet=2).
  pythia.readString("PDF:pSet = 2");
  pythia.readString("PDF:pHardSet = 2");

  // Switch off resonance decays, ISR, FSR, MPI and Bose-Einstein.
  pythia.readString("ProcessLevel:resonanceDecays = off");
  pythia.readString("PartonLevel:FSRinResonances = off");
  pythia.readString("PartonLevel:FSR = off");
  pythia.readString("PartonLevel:ISR = off");
  pythia.readString("PartonLevel:MPI = off");
  pythia.readString("HadronLevel:BoseEinstein = off");

  // Primordial kT is switched off.
  pythia.readString("BeamRemnants:primordialKT = off");        // Allow or not selection of primordial kT according to the parameter values.
  pythia.readString("BeamRemnants:primordialKTremnant = 0.0"); // The width sigma_remn, assigned as a primordial kT to beam-remnant partons.

  // Switches for hadron production and decay.
  // pythia.readString("HadronLevel:Decay= off");
  pythia.readString("111:onMode = off"); // pi0
  pythia.readString("311:onMode = off"); // K0
  pythia.readString("211:onMode = off"); // pi+
  // pythia.readString("221:onMode = off"); // eta
  // pythia.readString("331:onMode = off"); // eta'
  // pythia.readString("311:onMode = off"); // K0 decay

  // Invariant mass distribution of resonances as in the string+3P0 model.
  pythia.readString("ParticleData:modeBreitWigner=3"); // particles registered as having a mass width are given a mass in the range m_min < m < m_max, according to a truncated relativistic Breit-Wigner, i.e. quadratic in m.

  // Switch off automatic event listing in favour of manual.
  pythia.readString("Next:numberShowInfo = 0");
  pythia.readString("Next:numberShowProcess = 0");
  pythia.readString("Next:numberShowEvent = 1");

  // Settings of string fragmentation parameters.
  pythia.readString("StringPT:enhancedFraction = 0."); // the fraction of string breaks with enhanced width.
  pythia.readString("StringPT:enhancedWidth = 0.");    // the enhancement of the width in this fraction.

  // Settings from `clasdis`
  // - pythia 6 -> 8 translation from: https://skands.web.cern.ch/slides/11/11-02-skands-uemb.pdf
  pythia.readString("StringFlav:mesonUDvector = 0.7"); // ratio vector/pseudoscalar for light (u, d) mesons (analogous to PARJ(11): fraction of $\rho / \pi$)
  pythia.readString("StringFlav:mesonSvector = 0.75"); // ratio vector/pseudoscalar for strange mesons      (analogous to PARJ(12): fraction of $K^* / K$)
  pythia.readString("StringPT:sigma = 0.5");           // pT width of the fragmentation process (analogous to PARJ(21))

  // StringSpinner settings.
  // Value of the free parameters |GL/GT| and thetaLT = arg(GL/GT).
  pythia.readString("StringSpinner:GLGT = 5");
  pythia.readString("StringSpinner:thetaLT = 0");

  // Choose to assign polarisations.
  int beamSpin, targetSpin;
  bool beamPolarized   = false;
  bool targetPolarized = false;
  Vec4 SNucleon, SQuark;
  switch(mode) {
    case 0: // LU, spin +
      beamPolarized = true;
      beamSpin      = 1;
      break;
    case 1: // LU, spin -
      beamPolarized = true;
      beamSpin      = -1;
      break;
    case 2: // UL, spin +
      targetPolarized = true;
      targetSpin      = 1;
      break;
    case 3: // UL, spin -
      targetPolarized = true;
      targetSpin      = -1;
      break;
  }

  std::string polStr;
  if(beamPolarized) {
    SQuark.p(0.0, 0.0, (double)(-beamSpin), 0.0); // minus sign, since quark momentum is reversed after hard scattering
    switch(beamSpin) {
      case 1:  polStr = "0.0,0.0,-1.0"; break; // minus sign, since quark momentum is reversed after hard scattering
      case -1: polStr = "0.0,0.0,1.0";  break; // minus sign, since quark momentum is reversed after hard scattering
    }
    std::vector<std::string> quarks = {"u", "d", "s", "ubar", "dbar", "sbar"};
    for(auto quark : quarks)
      pythia.readString("StringSpinner:" + quark + "Polarisation = " + polStr);
  }
  if(targetPolarized) {
    SNucleon.p(0.0, 0.0, (double)targetSpin, 0.0);
    switch(targetSpin) {
      case 1:  polStr = "0.0,0.0,1.0";  break;
      case -1: polStr = "0.0,0.0,-1.0"; break;
    }
    pythia.readString("StringSpinner:targetPolarisation = " + polStr);
  }

  // Initialize.
  pythia.init();


  ////////////////////////////////////////////////////////////////////


  // define output tree
  auto outFile = new TFile(outFileN, "RECREATE");
  auto tr      = new TTree("ditr", "ditr"); // name expected by calcKinematics.cpp
                                            //
  enum parEnum {kEle,kHadA,kHadB,nPar};
  TString parName[nPar] = {"ele", "hadA", "hadB" };

  Int_t   runnum, evnum, helicity;
  Int_t   Row[nPar], Pid[nPar];
  Float_t Px[nPar], Py[nPar], Pz[nPar], E[nPar];
  Float_t Vx[nPar], Vy[nPar], Vz[nPar];
  Float_t chi2pid[nPar];
  Int_t   status[nPar];
  Float_t beta[nPar];
  Float_t Q2, W, x, y;

  auto SetParticleBranch = [&tr] (TString parStr, TString brName, void *brAddr, TString type) {
    TString fullBrName = parStr + TString("_") + brName;
    tr->Branch(fullBrName, brAddr, fullBrName + TString("/") + type);
  };

  tr->Branch("runnum",   &runnum,   "runnum/I");
  tr->Branch("evnum",    &evnum,    "evnum/I");
  tr->Branch("helicity", &helicity, "helicity/I");
  for(int p=0; p<nPar; p++) {
    SetParticleBranch(parName[p], "Row",     &(Row[p]),     "I");
    SetParticleBranch(parName[p], "Pid",     &(Pid[p]),     "I");
    SetParticleBranch(parName[p], "Px",      &(Px[p]),      "F");
    SetParticleBranch(parName[p], "Py",      &(Py[p]),      "F");
    SetParticleBranch(parName[p], "Pz",      &(Pz[p]),      "F");
    SetParticleBranch(parName[p], "E",       &(E[p]),       "F");
    SetParticleBranch(parName[p], "Vx",      &(Vx[p]),      "F");
    SetParticleBranch(parName[p], "Vy",      &(Vy[p]),      "F");
    SetParticleBranch(parName[p], "Vz",      &(Vz[p]),      "F");
    SetParticleBranch(parName[p], "chi2pid", &(chi2pid[p]), "F");
    SetParticleBranch(parName[p], "status",  &(status[p]),  "I");
    SetParticleBranch(parName[p], "beta",    &(beta[p]),    "F");
  }
  tr->Branch("SS_Q2", &Q2, "SS_Q2/F"); // inclusive kinematics directly from pythia
  tr->Branch("SS_W",  &W,  "SS_W/F");
  tr->Branch("SS_x",  &x,  "SS_x/F");
  tr->Branch("SS_y",  &y,  "SS_y/F");

  // decode pairType
  Int_t h0, h1;
  DecodePairType(pairType, h0, h1);
  Int_t whichHad[2], whichPIDs[2];
  for(int h=0; h<2; h++) {
    whichHad[h]  = dihHadIdx(h0, h1, h);
    whichPIDs[h] = PartPID(whichHad[h]);
  }
  cout << "Selecting dihadrons with PIDs (" << whichPIDs[qA] << ", " << whichPIDs[qB] << ")" << endl;
  auto elePID = PartPID(kE);

  ////////////////////////////////////////////////////////////////////
  // EVENT LOOP
  ////////////////////////////////////////////////////////////////////
  for (int iEvent = 0; iEvent < nEvent; ++iEvent) {

    if (!pythia.next()) continue;

    // string selection
    if(useStringSelection) {
      if( EV[7].id() != stringSelection[0] ||
          EV[8].id() != stringSelection[1] ) continue;
    }
    // cout << "string: " << EV[7].id() << " === " << EV[8].id() << endl;

    // if(iEvent < 3) EV.list();

    // event-level branches
    runnum   = RUNNUM_STRING_SPINNER;
    evnum    = (Int_t) iEvent;
    helicity = beamPolarized ? (Int_t) beamSpin : 0;

    // inclusive kinematics directly from pythia (cf. those calculated from scattered lepton)
    DISKinematics dis(EV[1].p(), EV[5].p(), EV[2].p());
    Q2 = (Float_t) dis.Q2;
    W  = (Float_t) TMath::Sqrt(dis.W2);
    x  = (Float_t) dis.xB;
    y  = (Float_t) dis.y;

    // find hadrons and electron
    std::vector<Int_t> hadRowList[2];
    Row[kEle] = -1;
    double eleE = 0;
    for(int row = 0; row < EV.size(); ++row) {
      auto par = EV[row];
      if(!par.isFinal()) continue;
      if(par.id() == elePID) {
        if(par.e() > eleE) {
          eleE      = par.e();
          Row[kEle] = row;
        }
      } else {
        for(int h = 0; h < 2; h++) {
          if(par.id() == whichPIDs[h]) {
            hadRowList[h].push_back(row);
          }
        }
      }
    }
    if(Row[kEle] == -1 || hadRowList[qA].empty() || hadRowList[qB].empty())
      continue;

    // hadron pairing and tree filling
    for(auto iA : hadRowList[qA]) {
      for(auto iB : hadRowList[qB]) {
        Row[kHadA] = iA;
        Row[kHadB] = iB;
        for(int i=0; i<nPar; i++) {
          auto par = EV[Row[i]];
          Pid[i]     = (Int_t)   par.id();
          Px[i]      = (Float_t) par.px();
          Py[i]      = (Float_t) par.py();
          Pz[i]      = (Float_t) par.pz();
          E[i]       = (Float_t) par.e();
          Vx[i]      = 0.0;
          Vy[i]      = 0.0;
          Vz[i]      = 0.0;
          chi2pid[i] = 0.0;
          status[i]  = 0;
          beta[i]    = 0.0;
        }
        tr->Fill();
      }
    }

  } // end EVENT LOOP

  cout << "Number of dihadrons generated: " << tr->GetEntries() << endl;
  cout << "Writing output file:" << endl << "   " << outFileN << endl;
  outFile->Write();
  return 0;

}


/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////

// run modes, for configuring which asymmetry and which spin to run with
TString GetModeStr(Int_t mode) {
  TString ret;
  switch(mode) {
    case 0: ret="LU, spin +"; break;
    case 1: ret="LU, spin -"; break;
    case 2: ret="UL, spin +"; break;
    case 3: ret="UL, spin -"; break;
    default:
            cerr << "ERROR: unknown mode " << mode << endl;
            ret = "UNKNOWN MODE";
  }
  return ret;
}
