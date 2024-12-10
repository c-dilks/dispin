// copied from https://gitlab.com/albikerbizi/stringspinner.git `dis.cc` (locally deps/StringSpinner/dis.cc)
// and modified for usage in this analysis

# include <vector>

// pythia, stringspinner
#include <Pythia8/Pythia.h>
#include "deps/StringSpinner/StringSpinner.h"

// ROOT
#include <TFile.h>
#include <TTree.h>

// dispin
#include "src/Constants.h"
#include "src/Tools.h"

using namespace Pythia8;

TString GetModeStr(Int_t mode);

/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////

int main(int argc, char** argv) {

  // arguments
  Int_t mode       = 0;
  TString outFileN = Form("sss/%d.root",mode);
  int seed         = -1; // -1: default seed;  0: based on time;  1-900_000_000: fixed seed
  int pairType     = EncodePairType(kPip,kPim);
  if(argc==1) {
    cerr << endl << "USAGE: " << argv[0] << " [ARGUMENTS]..." << endl;
    cerr << endl << "ARGUMENTS:" << endl;
    cerr << " [numEvents]" << endl;
    cerr << " [mode (default=" << mode << ")]" << endl;
    cerr << " [outputFileName (default=" << outFileN << ")]" << endl;
    cerr << " [seed (default=" << seed << ")]" << endl;
    cerr << " [pairType (default=0x" << std::hex << pairType << std::dec << ")]" << endl;
    cerr << endl << "MODES: " << endl;
    for(int m=0; m<4; m++) cerr << "  " << m << ": " << GetModeStr(m) << endl;
    cerr << endl;
    cerr << "See also HARD-CODED SETTINGS in the source file" << endl << endl;
    return 2;
  }
  Long64_t nEvent;
  int ai = 1;
  if(argc>ai) nEvent             = (Long64_t)strtof(argv[ai++],NULL);
  if(argc>ai) mode               = (Int_t)strtof(argv[ai++],NULL);
  if(argc>ai) outFileN           = TString(argv[ai++]);
  if(argc>ai) seed               = (int)strtof(argv[ai++],NULL);
  if(argc>ai) pairType           = (int)strtof(argv[ai++],NULL);
  TString modeStr = GetModeStr(mode);
  if(modeStr.Contains("UNKNOWN")) return 1;
  cout << "ARGS: "        << endl;
  cout << "  nEvent           = " << nEvent   << endl;
  cout << "  mode             = " << mode     << ": " << modeStr << endl;
  cout << "  outFileN         = " << outFileN << endl;
  cout << "  seed             = " << seed     << endl;
  cout << "  pairType         = " << "0x" << std::hex << pairType << std::dec << endl;

  Pythia pythia;
  auto& EV = pythia.event;   // event record
  auto& HP = pythia.process; // hard process record
  auto fhooks = std::make_shared<SimpleStringSpinner>();
  fhooks->plugInto(pythia);

  // load steering file
  pythia.readFile("stringSpinSim.cmnd");

  // Seed
  pythia.readString("Random:setSeed = on");
  pythia.readString("Random:seed = " + std::to_string(seed));

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
  Int_t   genParentIdx[nPar];
  Int_t   genParentPid[nPar];
  Float_t Q2, W, x, y, kT;
  Int_t   parton_pdg;

  auto SetParticleBranch = [&tr] (TString parStr, TString brName, void *brAddr, TString type) {
    TString fullBrName = parStr + TString("_") + brName;
    tr->Branch(fullBrName, brAddr, fullBrName + TString("/") + type);
  };

  tr->Branch("runnum",   &runnum,   "runnum/I");
  tr->Branch("evnum",    &evnum,    "evnum/I");
  tr->Branch("helicity", &helicity, "helicity/I");
  for(int p=0; p<nPar; p++) {
    SetParticleBranch(parName[p], "Row",       &(Row[p]),          "I");
    SetParticleBranch(parName[p], "Pid",       &(Pid[p]),          "I");
    SetParticleBranch(parName[p], "Px",        &(Px[p]),           "F");
    SetParticleBranch(parName[p], "Py",        &(Py[p]),           "F");
    SetParticleBranch(parName[p], "Pz",        &(Pz[p]),           "F");
    SetParticleBranch(parName[p], "E",         &(E[p]),            "F");
    SetParticleBranch(parName[p], "Vx",        &(Vx[p]),           "F");
    SetParticleBranch(parName[p], "Vy",        &(Vy[p]),           "F");
    SetParticleBranch(parName[p], "Vz",        &(Vz[p]),           "F");
    SetParticleBranch(parName[p], "chi2pid",   &(chi2pid[p]),      "F");
    SetParticleBranch(parName[p], "status",    &(status[p]),       "I");
    SetParticleBranch(parName[p], "beta",      &(beta[p]),         "F");
    SetParticleBranch(parName[p], "parentIdx", &(genParentIdx[p]), "I");
    SetParticleBranch(parName[p], "parentPid", &(genParentPid[p]), "I");
  }
  tr->Branch("SS_Q2", &Q2, "SS_Q2/F"); // inclusive kinematics directly from pythia
  tr->Branch("SS_W",  &W,  "SS_W/F");
  tr->Branch("SS_x",  &x,  "SS_x/F");
  tr->Branch("SS_y",  &y,  "SS_y/F");
  tr->Branch("SS_kT", &kT, "SS_kT/F"); // parton kT (momentum transverse to virtual photon)
  tr->Branch("SS_parton_pdg", &parton_pdg, "SS_parton_pdg/I");

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

    // patch for <https://gitlab.com/Pythia8/releases/-/issues/529>
    // FIXME: disabled, since it seems to shift the MX peaks in the _wrong_ direction
    /*
    RotBstMatrix toLab;
    Vec4 pLepLab   = HP[1].p(); // use the electron beam momentum
    Vec4 pLepEvent = EV[1].p();
    toLab.bst(pLepEvent, pLepLab);
    EV.rotbst(toLab);
    for(auto const& [name, row] : std::vector<std::pair<std::string,int>>{{"beam", 1}, {"target", 2}}) {
      auto diff = std::max(
          std::abs(EV[row].pz() - HP[row].pz()),
          std::abs(EV[row].e()  - HP[row].e())
          );
      if(diff > 0.0001)
        throw std::runtime_error("ERROR: mismatch of event-frame and hard-process-frame " + name + " momentum");
    }
    */

    // print some events
    if(iEvent < 3) {
      HP.list(false, false, 8);
      EV.list(false, false, 8);
    }

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
    if(iEvent < 3)
      std::cout << "scattered electron is row " << Row[kEle] << std::endl;
    if(Row[kEle] == -1 || hadRowList[qA].empty() || hadRowList[qB].empty())
      continue;

    // get kT (transverse momentum of the parton)
    int row_init_ele    = -1;
    int row_init_parton = -1;
    kT = UNDEF;
    parton_pdg = (Int_t) UNDEF;
    for(auto const& mother : std::vector<int>{ EV[Row[kEle]].mother1(), EV[Row[kEle]].mother2() }) { // find the initial electron
      if(EV[mother].id() == 11) {
        row_init_ele = mother;
        break;
      }
    }
    for(int row = 0; row < EV.size(); ++row) {
      if(EV[row].status() == -62) {  // -61 (-62): incoming outgoing subprocess particle with primordial kT included
                                     // FIXME: pick one!
        row_init_parton = row;
        break;
      }
    }
    if(row_init_ele >= 0 && row_init_parton >= 0) {
      // virtual photon direction
      auto phot_p = EV[Row[kEle]].p() - EV[row_init_ele].p();
      // auto phot_p = EV[Row[kEle]].p() - Pythia8::Vec4(0, 0, 10.60410, 10.60410);
      kT = Tools::Reject(
          TVector3{ EV[row_init_parton].px(), EV[row_init_parton].py(), EV[row_init_parton].pz() },
          TVector3{ phot_p.px(),              phot_p.py(),              phot_p.pz() }
          ).Mag();
      parton_pdg = (Int_t) EV[row_init_parton].id();
    }

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
          // get parent(s)
          if(i == kEle) { // don't bother for electron
            genParentIdx[i] = 0;
            genParentPid[i] = 0;
          } else {
            genParentIdx[i] = (Int_t) UNDEF;
            genParentPid[i] = (Int_t) UNDEF;
            auto mom1idx    = par.mother1();
            auto mom2idx    = par.mother2();
            if(mom1idx>0 && mom2idx==0) { // single mother
              genParentIdx[i] = mom1idx;
              genParentPid[i] = (Int_t) EV[mom1idx].id();
            }
            else if(mom1idx>0 && mom2idx>0 && mom1idx<mom2idx) { // multiple mothers
              auto st = std::abs(par.status());
              if(st>=81 && st<=86) { // string fragmentation
                // cout << "STRING: " << EV[mom1idx].id() << " " << EV[mom2idx].id() << endl;
                genParentIdx[i] = mom1idx; // set "the" parent to the string's quark as opposed to the diquark // FIXME: might not work
                genParentPid[i] = 92; // means "string fragment" downstream
              }
              else if(st>=101 && st<=106) { // R-hadron formation
                cerr << "WARNING: parent R-hadron formation not handled" << endl;
              }
              else { // 2 distinct mothers, from 2->n hard scattering
                if(Pid[i]!=elePID)
                  cerr << "WARNING: parent 2->n hard scattering not handled" << endl;
              }
            }
            else {
              cerr << "WARNING: parent case not handled" << endl;
            }
          }
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
