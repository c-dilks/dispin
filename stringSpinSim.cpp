// copied from https://gitlab.com/albikerbizi/stringspinner.git `dis.cc` (locally deps/StringSpinner/dis.cc)
// and modified for usage in this analysis

# include <vector>

// pythia, stringspinner
#include "Pythia8/Pythia.h"
#include "StringSpinner.h"

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
  int stringSelection[2]         = {2, 2101}; // u === (ud)_0
  TString stringSelectionDesc[2] = {"u", "(ud)_0"};
  if(argc==1) {
    cerr << "USAGE: " << argv[0] <<
      " [numEvents]" <<
      " [mode(def=" << mode << ")]" <<
      " [outputFileName(def=" << outFileN << ")]" <<
      " [stringSelection0 (default=" << stringSelection[0] << " for '" << stringSelectionDesc[0] << "')]" <<
      " [stringSelection1 (default=" << stringSelection[1] << " for '" << stringSelectionDesc[1] << "')]" <<
      " [seed(def=" << seed << ")]" <<
      endl;
    cerr << endl;
    cerr << "MODES: " << endl;
    for(int m=0; m<4; m++) cerr << "  " << m << ": " << GetModeStr(m) << endl;
    cerr << endl;
    cerr << "NOTE: set stringSelection arguments both to 0 to disable filtering of events by strings" << endl;
    cerr << endl;
    cerr << "See also HARD-CODED SETTINGS in the source file" << endl << endl;
    return 2;
  }
  Long64_t nEvent;
  if(argc>1) nEvent             = (Long64_t)strtof(argv[1],NULL);
  if(argc>2) mode               = (Int_t)strtof(argv[2],NULL);
  if(argc>3) outFileN           = TString(argv[3]); else outFileN = Form("out/%d.root",mode);
  if(argc>4) stringSelection[0] = (int)strtof(argv[4],NULL);
  if(argc>5) stringSelection[1] = (int)strtof(argv[5],NULL);
  if(argc>6) seed               = (int)strtof(argv[6],NULL);
  TString modeStr = GetModeStr(mode);
  if(modeStr.Contains("UNKNOWN")) return 1;
  cout << "ARGS: "        << endl;
  cout << "  nEvent           = " << nEvent   << endl;
  cout << "  mode             = " << mode     << ": " << modeStr << endl;
  cout << "  outFileN         = " << outFileN << endl;
  cout << "  stringSelection0 = " << stringSelection[0] << endl;
  cout << "  stringSelection1 = " << stringSelection[1] << endl;
  cout << "  seed             = " << seed     << endl;

  Pythia pythia;
  Event& event = pythia.event;
  auto fhooks = std::make_shared<SimpleStringSpinner>();
  fhooks->plugInto(pythia);

  // HARD-CODED SETTINGS ////////////////////////////////////////////
  //// incoming particles
  ParticleData& pdt = pythia.particleData;
  double eProton   = pdt.m0(2212); // proton beam energy (target mass)
  double pElectron = 10.6; // electron beam momentum
  double eElectron = Tools::PMtoE(pElectron, pdt.m0(11)); // electron beam energy
  //// phase space
  double Q2min     = 1.0; // minimum Q2
  ///////////////////////////////////////////////////////////////////

  // Seed
  pythia.readString("Random:setSeed = on");
  pythia.settings.parm("Random:seed",seed);

  // Set up incoming beams, for frame with unequal beam energies.
  pythia.readString("Beams:frameType = 2");
  pythia.readString("Beams:idA = 11");      // BeamA = electron
  pythia.readString("Beams:idB = 2212");    // BeamB = proton
  pythia.settings.parm("Beams:eA", eElectron);
  pythia.settings.parm("Beams:eB", eProton);

  // Interaction mechanism.
  pythia.readString("WeakBosonExchange:ff2ff(t:gmZ) = on");

  // Phase-space cut: minimal Q2 of process.
  pythia.settings.parm("PhaseSpace:Q2Min", Q2min);

  // Go down to low x-Bjorken.
  pythia.readString("PhaseSpace:pTHatMinDiverge = 0.5");
  pythia.readString("PhaseSpace:mHatMin = 0.");

  // Set dipole recoil on. Necessary for DIS + shower.
  pythia.readString("SpaceShower:dipoleRecoil = off");

  // QED radiation off lepton not handled yet by the new procedure.
  pythia.readString("PDF:lepton = off");
  pythia.readString("TimeShower:QEDshowerByL = off");

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
  pythia.readString("BeamRemnants:primordialKT = off");
  pythia.readString("BeamRemnants:primordialKTremnant = 0.0");

  // Switches for hadron production and decay.
  // pythia.readString("HadronLevel:Decay= off");
  pythia.readString("111:onMode = off"); // pi0
  pythia.readString("311:onMode = off"); // K0
  pythia.readString("211:onMode = off"); // pi+
  // pythia.readString("221:onMode = off"); // eta
  // pythia.readString("331:onMode = off"); // eta'
  // pythia.readString("311:onMode = off"); // K0 decay

  // Invariant mass distribution of resonances as in the string+3P0 model.
  pythia.readString("ParticleData:modeBreitWigner=3");

  // Switch off automatic event listing in favour of manual.
  pythia.readString("Next:numberShowInfo = 0");
  pythia.readString("Next:numberShowProcess = 0");
  pythia.readString("Next:numberShowEvent = 1");

  // Settings of string fragmentation parameters.
  pythia.readString("StringPT:enhancedFraction = 0.");
  pythia.readString("StringPT:enhancedWidth = 0.");

  // StringSpinner settings.
  // Value of the free parameters |GL/GT| and thetaLT = arg(GL/GT).
  pythia.readString("StringSpinner:GLGT = 5");
  pythia.readString("StringSpinner:thetaLT = 0");

  // Choose to assign polarisations.
  Int_t modeSpin;
  Bool_t beamPolarized   = false;
  Bool_t targetPolarized = false;
  Vec4 Sproton, Squark;
  switch(mode) {
    case 0: // UT, spin up
      targetPolarized = true;
      modeSpin = 1;
      Sproton.p(0.0, (double)modeSpin, 0.0, 0.0);
      pythia.readString("StringSpinner:targetPolarisation = 0.0,1.0,0.0");
      break;
    case 1: // UT, spin down
      targetPolarized = true;
      modeSpin = -1;
      Sproton.p(0.0, (double)modeSpin, 0.0, 0.0);
      pythia.readString("StringSpinner:targetPolarisation = 0.0,-1.0,0.0");
      break;
    case 2: // LU, spin +
      beamPolarized = true;
      modeSpin = 1;
      Squark.p(0.0, 0.0, -1.0*(double)modeSpin, 0.0); // minus sign, since quark momentum is reversed after hard scattering
      pythia.readString("StringSpinner:uPolarisation = 0.0,-1.0,0.0"); // minus sign, since quark momentum is reversed after hard scattering
      break;
    case 3: // LU, spin -
      beamPolarized = true;
      modeSpin = -1;
      Squark.p(0.0, 0.0, -1.0*(double)modeSpin, 0.0); // minus sign, since quark momentum is reversed after hard scattering
      pythia.readString("StringSpinner:uPolarisation = 0.0,1.0,0.0"); // minus sign, since quark momentum is reversed after hard scattering
      break;
  }

  // Initialize.
  pythia.init();
  /*
  BruTree *BT = new BruTree(outFileN);
  bool useStringSelection = stringSelection[0]!=0 && stringSelection[1]!=0;

  // Variables for test calculation of the pi+ Collins asymmetry.
  double Acoll[2] = {0.0};
  int Npi(0);
  double DNN(0.0);
  double STav(0.0);
  
  // Begin event loop.
  for (int iEvent = 0; iEvent < nEvent; ++iEvent) {

    if (!pythia.next()) continue;

    // string selection
    if(useStringSelection) {
      if( event[7].id() != stringSelection[0] ||
          event[8].id() != stringSelection[1] ) continue;
    }
    // cout << "string: " << event[7].id() << " === " << event[8].id() << endl;

    DISKinematics dis(event[1].p(), event[5].p(), event[2].p());

    // Get the photon and proton momenta in the GNS frame.
    Vec4 pPhoton = dis.GNS*dis.q;
    Vec4 pProton = dis.GNS*dis.hadin;
		
    // Target polarization vector, degree of transverse polarization and azimuthal angle in GNS.
    Vec4 SprotonGNS = dis.GNS*Sproton;
    double ST = 0.0;
    double phiS = 0.0;
    if(targetPolarized) {
      ST   = SprotonGNS.pT();
      phiS = SprotonGNS.phi();
      if(phiS<0.0) phiS += 2.0 * M_PI;
    }
		
    // List some events.
    if(iEvent<20) event.list();
		
    // Loop inside the event output.
    for (int i = 0; i < event.size(); ++i){
        
      // Hadron momentum in GNS, id and status.
      Vec4 phad = dis.GNS*event[i].p();
      int idHad = event[i].id();
      int statusHad = event[i].status();
      // Hadrons fractional energy, azimuthal angle and transverse momentum squared in GNS.	
      double zh = ( phad * pProton ) / (pProton * pPhoton);
      double phihad = atan2(phad.py(),phad.px());
      if (phihad<0) phihad += 2.*M_PI;
      double pT2 = phad.pT2();

      // Fill test Collins asymmetry for pi+.
      if(targetPolarized) {
        if(idHad==211&&zh>0.2&&sqrt(pT2)>0.1&&statusHad==83) {
          Acoll[0] += 2.0*sin(phihad+phiS-M_PI);
          Acoll[1] += pow(2.0*sin(phihad+phiS-M_PI),2);
          Npi += 1;
          DNN += 2.0*(1.0-dis.y)/(1.0+pow(1.0-dis.y,2));
          STav += ST;
        }
      }
             
    } // End loop on particle within the same event.

    // fill tree
    BT->SetSpin(modeSpin);
    BT->SetPhiS(phiS);
    BT->Fill(event,dis);
  } // End loop on events.

  // Calculate test Collins asymmetry for pi+.
  if(targetPolarized) {
    for(int i=0;i<2;i++) Acoll[i] /= Npi;
    DNN /= Npi;
    STav /= Npi;
    cout << "The average Collins asymmetry for pi+ is: \n";
    cout << Acoll[0]/(DNN*STav) << "+/-" << sqrt((Acoll[1]-pow(Acoll[0],2))/Npi)/(DNN*STav) << endl;
  }

  BT->Write();
  */
  return 0;

} // end main


/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////

// run modes, for configuring which asymmetry and which spin to run with
TString GetModeStr(Int_t mode) {
  TString ret;
  switch(mode) {
    case 0: ret="UT, spin up";   break;
    case 1: ret="UT, spin down"; break;
    case 2: ret="LU, spin +";    break;
    case 3: ret="LU, spin -";    break;
    default:
            cerr << "ERROR: unknown mode " << mode << endl;
            ret = "UNKNOWN MODE";
  }
  return ret;
}
