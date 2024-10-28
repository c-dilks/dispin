#include <string>
#include <filesystem>
#include <cassert>

// hipo
#include <hipo4/reader.h>

// iguana
#include <iguana/algorithms/AlgorithmSequence.h>

// ROOT
#include <TFile.h>
#include <TTree.h>
#include <Math/Vector4D.h>

// dispin
#include "src/Tools.h"
#include "src/Constants.h"


int main(int argc, char** argv) {

  // ==================================================================================
  // arguments
  // ==================================================================================

  TString hipoFileName;
  TString outputDir;
  TString dataStream = "data";
  TString hipoType   = "skim";
  if(argc < 3) {
    std::cerr << "USAGE: " << argv[0] << " [hipoFileName] [outputDir] [dataStream] [hipoType]" << std::endl;
    std::cerr << "  [hipoFileName] the HIPO file to analyze" << std::endl;
    std::cerr << "  [outputDir]    the output dir, for the output `outroot` file" << std::endl;
    std::cerr << "  [dataStream]   'data', 'mcrec', or 'mcgen'; default = " << dataStream << std::endl;
    std::cerr << "  [hipoType]     'skim' or 'dst'; default = " << hipoType << std::endl;
    return 1;
  }
  hipoFileName          = TString(argv[1]);
  outputDir             = TString(argv[2]);
  if(argc>3) dataStream = TString(argv[3]);
  if(argc>4) hipoType   = TString(argv[4]);

  // check `dataStream` argument
  //// first check if there are any 'special' strings in `dataStream`
  int disLeptonID = 11; // DIS lepton PDG code (11 for electron)
  if(dataStream.Contains("positron"))
    disLeptonID = -11;
  //// then remove any 'special' strings
  Tools::GlobalRegexp(dataStream, TRegexp("positron"), "");
  Tools::GlobalRegexp(dataStream, TRegexp("rad"),      "");
  //// then check if these are data, MC generated, or MC reconstructed
  bool useMC    = false; // true if any MC mode is set
  bool useMCgen = false; // true if reading only MC generated particles
  bool useMCrec = false; // true if reading MC reconstructed particles, with matched generated particles
  if(dataStream == "data") {
    useMC = false;
  }
  else if(dataStream == "mcrec") {
    useMC    = true;
    useMCrec = true;
  }
  else if(dataStream == "mcgen") {
    useMC    = true;
    useMCgen = true;
  }
  else {
    throw std::runtime_error("unrecognized dataStream");
  }
  if(useMC)
    std::cout << "READING MONTE CARLO FILE" << std::endl;


  // ==================================================================================
  // read input file and set input banks
  // ==================================================================================

  // read the input HIPO file, and decide which banks to read
  std::vector<std::string> inputBankNames = {
    "REC::Event",
    "RUN::config",
    "REC::Particle",
    "REC::Calorimeter",
    "REC::Scintillator",
    "REC::Track",
    "REC::Traj"
  };
  if(useMCrec) {
    inputBankNames.push_back("MC::Particle");
    inputBankNames.push_back("MC::Lund");
  }
  hipo::reader hipoReader(hipoFileName, {0});
  hipo::banklist hipoBanks = hipoReader.getBanks(inputBankNames);


  // ==================================================================================
  // setup iguana
  // ==================================================================================

  iguana::AlgorithmSequence iguanaSeq;
  iguanaSeq.Add("clas12::SectorFinder");
  iguanaSeq.Add("physics::InclusiveKinematics");
  iguanaSeq.Add("physics::DihadronKinematics");
  iguanaSeq.Add("physics::SingleHadronKinematics");

  iguanaSeq.SetConfigFileForEachAlgorithm("iguana_config.yaml"); // FIXME: assumes PWD has this file

  iguanaSeq.Start(hipoBanks);


  // ==================================================================================
  // bank indices
  // ==================================================================================

  //// input banks
  auto b_event       = hipo::getBanklistIndex(hipoBanks, "REC::Event");
  auto b_config      = hipo::getBanklistIndex(hipoBanks, "RUN::config");
  auto b_particle    = hipo::getBanklistIndex(hipoBanks, "REC::Particle");
  auto b_calorimeter = hipo::getBanklistIndex(hipoBanks, "REC::Calorimeter");
  auto b_track       = hipo::getBanklistIndex(hipoBanks, "REC::Track");
  auto b_traj        = hipo::getBanklistIndex(hipoBanks, "REC::Traj");
  //// input MC banks
  auto b_mc_particle = useMCrec ? hipo::getBanklistIndex(hipoBanks, "MC::Particle") : 0;
  auto b_mc_lund     = useMCrec ? hipo::getBanklistIndex(hipoBanks, "MC::Lund") : 0;
  //// output banks
  auto b_sector_finder     = hipo::getBanklistIndex(hipoBanks, "REC::Particle::Sector");
  auto b_inclusive_kin     = hipo::getBanklistIndex(hipoBanks, "physics::InclusiveKinematics");
  auto b_dihadron_kin      = hipo::getBanklistIndex(hipoBanks, "physics::DihadronKinematics");
  auto b_single_hadron_kin = hipo::getBanklistIndex(hipoBanks, "physics::SingleHadronKinematics");


  // ==================================================================================
  // setup output file
  // ==================================================================================

  TString outputFileName = outputDir + TString("/") + TString(std::filesystem::path(hipoFileName.Data()).filename()) + TString(".root");
  std::cout << "INPUT FILE:  " << hipoFileName << std::endl;
  std::cout << "OUTPUT FILE: " << outputFileName  << std::endl;
  auto outputFile = new TFile(outputFileName , "RECREATE");

  // OUTROOT TREE BRANCHES
  auto tr = new TTree("tree", "tree");
  // - event-level branches
  Int_t runnum{0};   tr->Branch("runnum",   &runnum,   "runnum/I");
  Int_t evnum{0};    tr->Branch("evnum",    &evnum,    "evnum/I");
  Int_t helicity{0}; tr->Branch("helicity", &helicity, "helicity/I");
  // - DIS kinematics branches
  Float_t W{0};     tr->Branch("W",     &W,     "W/F");
  Float_t Q2{0};    tr->Branch("Q2",    &Q2,    "Q2/F");
  Float_t Nu{0};    tr->Branch("Nu",    &Nu,    "Nu/F");
  Float_t x{0};     tr->Branch("x",     &x,     "x/F");
  Float_t y{0};     tr->Branch("y",     &y,     "y/F");
  Float_t beamE{0}; tr->Branch("beamE", &beamE, "beamE/F");
  // - electron kinematics branches
  Int_t   eleRow{0};           tr->Branch("eleRow",     &eleRow,     "eleRow/I");
  Float_t eleE{0};             tr->Branch("eleE",       &eleE,       "eleE/F");
  Float_t eleP{0};             tr->Branch("eleP",       &eleP,       "eleP/F");
  Float_t elePt{0};            tr->Branch("elePt",      &elePt,      "elePt/F");
  Float_t eleEta{0};           tr->Branch("eleEta",     &eleEta,     "eleEta/F");
  Float_t elePhi{0};           tr->Branch("elePhi",     &elePhi,     "elePhi/F");
  Float_t eleVertex[3]{0,0,0}; tr->Branch("eleVertex",  eleVertex,   "eleVertex[3]/F");
  Int_t   eleStatus{0};        tr->Branch("eleStatus",  &eleStatus,  "eleStatus/I");
  Float_t eleChi2pid{0};       tr->Branch("eleChi2pid", &eleChi2pid, "eleChi2pid/F");
  Bool_t  eleFiduCut{false};   tr->Branch("eleFiduCut", &eleFiduCut, "eleFiduCut/O");
  Float_t elePCALen{0};        tr->Branch("elePCALen",  &elePCALen,  "elePCALen/F");
  Float_t eleECINen{0};        tr->Branch("eleECINen",  &eleECINen,  "eleECINen/F");
  Float_t eleECOUTen{0};       tr->Branch("eleECOUTen", &eleECOUTen, "eleECOUTen/F");
  Int_t   eleSector{0};        tr->Branch("eleSector",  &eleSector,  "eleSector/I");
  // - hadron branches
  Int_t   pairType{0};                      tr->Branch("pairType",   &pairType,  "pairType/I");
  Int_t   hadRow[2]{0,0};                   tr->Branch("hadRow",     hadRow,     "hadRow[2]/I");
  Int_t   hadIdx[2]{0,0};                   tr->Branch("hadIdx",     hadIdx,     "hadIdx[2]/I");
  Float_t hadE[2]{0,0};                     tr->Branch("hadE",       hadE,       "hadE[2]/F");
  Float_t hadP[2]{0,0};                     tr->Branch("hadP",       hadP,       "hadP[2]/F");
  Float_t hadPt[2]{0,0};                    tr->Branch("hadPt",      hadPt,      "hadPt[2]/F");
  Float_t hadEta[2]{0,0};                   tr->Branch("hadEta",     hadEta,     "hadEta[2]/F");
  Float_t hadPhi[2]{0,0};                   tr->Branch("hadPhi",     hadPhi,     "hadPhi[2]/F");
  Float_t hadXF[2]{0,0};                    tr->Branch("hadXF",      hadXF,      "hadXF[2]/F");
  Float_t hadVertex[2][3]{{0,0,0},{0,0,0}}; tr->Branch("hadVertex",  hadVertex,  "hadVertex[2][3]/F");
  Int_t   hadStatus[2]{0,0};                tr->Branch("hadStatus",  hadStatus,  "hadStatus[2]/I");
  Float_t hadBeta[2]{0,0};                  tr->Branch("hadBeta",    hadBeta,    "hadBeta[2]/F");
  Float_t hadChi2pid[2]{0,0};               tr->Branch("hadChi2pid", hadChi2pid, "hadChi2pid[2]/F");
  Bool_t  hadFiduCut[2]{false,false};       tr->Branch("hadFiduCut", hadFiduCut, "hadFiduCut[2]/O");
  // - dihadron branches
  Float_t Mh{0};      tr->Branch("Mh",      &Mh,      "Mh/F");
  Float_t Mmiss{0};   tr->Branch("Mmiss",   &Mmiss,   "Mmiss/F");
  Float_t Z[2]{0,0};  tr->Branch("Z",       Z,        "Z[2]/F");
  Float_t Zpair{0};   tr->Branch("Zpair",   &Zpair,   "Zpair/F");
  Float_t xF{0};      tr->Branch("xF",      &xF,      "xF/F");
  Float_t alpha{0};   tr->Branch("alpha",   &alpha,   "alpha/F");
  Float_t theta{0};   tr->Branch("theta",   &theta,   "theta/F");
  Float_t thetaLI{0}; tr->Branch("thetaLI", &thetaLI, "thetaLI/F");
  Float_t zeta{0};    tr->Branch("zeta",    &zeta,    "zeta/F");
  Float_t Ph{0};      tr->Branch("Ph",      &Ph,      "Ph/F");
  Float_t PhPerp{0};  tr->Branch("PhPerp",  &PhPerp,  "PhPerp/F");
  Float_t PhEta{0};   tr->Branch("PhEta",   &PhEta,   "PhEta/F");
  Float_t PhPhi{0};   tr->Branch("PhPhi",   &PhPhi,   "PhPhi/F");
  Float_t R{0};       tr->Branch("R",       &R,       "R/F");
  Float_t RPerp{0};   tr->Branch("RPerp",   &RPerp,   "RPerp/F");
  Float_t RT{0};      tr->Branch("RT",      &RT,      "RT/F");
  Float_t PhiH{0};    tr->Branch("PhiH",    &PhiH,    "PhiH/F");
  // -- phiR angles
  Float_t PhiRq{0};   tr->Branch("PhiRq",   &PhiRq,   "PhiRq/F");
  Float_t PhiRp{0};   tr->Branch("PhiRp",   &PhiRp,   "PhiRp/F");
  Float_t PhiRp_r{0}; tr->Branch("PhiRp_r", &PhiRp_r, "PhiRp_r/F");
  Float_t PhiRp_g{0}; tr->Branch("PhiRp_g", &PhiRp_g, "PhiRp_g/F");
  // -- SDME kinematics
  Float_t sdmePhiU{0}; tr->Branch("sdmePhiU", &sdmePhiU, "sdmePhiU/F");
  Float_t sdmePhiL{0}; tr->Branch("sdmePhiL", &sdmePhiL, "sdmePhiL/F");
  // - MC branches
  // --- generated DIS kinematics branches
  Float_t gen_W{0};  if(useMCrec) tr->Branch("gen_W",  &gen_W,  "gen_W/F");
  Float_t gen_Q2{0}; if(useMCrec) tr->Branch("gen_Q2", &gen_Q2, "gen_Q2/F");
  Float_t gen_Nu{0}; if(useMCrec) tr->Branch("gen_Nu", &gen_Nu, "gen_Nu/F");
  Float_t gen_x{0};  if(useMCrec) tr->Branch("gen_x",  &gen_x,  "gen_x/F");
  Float_t gen_y{0};  if(useMCrec) tr->Branch("gen_y",  &gen_y,  "gen_y/F");
  // --- generated electron kinematics branches
  Float_t gen_eleE{0};             if(useMCrec) tr->Branch("gen_eleE",      &gen_eleE,     "gen_eleE/F");
  Float_t gen_eleP{0};             if(useMCrec) tr->Branch("gen_eleP",      &gen_eleP,     "gen_eleP/F");
  Float_t gen_elePt{0};            if(useMCrec) tr->Branch("gen_elePt",     &gen_elePt,    "gen_elePt/F");
  Float_t gen_eleEta{0};           if(useMCrec) tr->Branch("gen_eleEta",    &gen_eleEta,   "gen_eleEta/F");
  Float_t gen_elePhi{0};           if(useMCrec) tr->Branch("gen_elePhi",    &gen_elePhi,   "gen_elePhi/F");
  Float_t gen_eleVertex[3]{0,0,0}; if(useMCrec) tr->Branch("gen_eleVertex", gen_eleVertex, "gen_eleVertex[3]/F");
  // --- generated hadron branches
  Int_t   gen_pairType{0};                      if(useMCrec) tr->Branch("gen_pairType",  &gen_pairType, "gen_pairType/I");
  Int_t   gen_hadRow[2]{0,0};                   if(useMCrec) tr->Branch("gen_hadRow",    gen_hadRow,    "gen_hadRow[2]/I");
  Int_t   gen_hadIdx[2]{0,0};                   if(useMCrec) tr->Branch("gen_hadIdx",    gen_hadIdx,    "gen_hadIdx[2]/I");
  Float_t gen_hadE[2]{0,0};                     if(useMCrec) tr->Branch("gen_hadE",      gen_hadE,      "gen_hadE[2]/F");
  Float_t gen_hadP[2]{0,0};                     if(useMCrec) tr->Branch("gen_hadP",      gen_hadP,      "gen_hadP[2]/F");
  Float_t gen_hadPt[2]{0,0};                    if(useMCrec) tr->Branch("gen_hadPt",     gen_hadPt,     "gen_hadPt[2]/F");
  Float_t gen_hadEta[2]{0,0};                   if(useMCrec) tr->Branch("gen_hadEta",    gen_hadEta,    "gen_hadEta[2]/F");
  Float_t gen_hadPhi[2]{0,0};                   if(useMCrec) tr->Branch("gen_hadPhi",    gen_hadPhi,    "gen_hadPhi[2]/F");
  Float_t gen_hadXF[2]{0,0};                    if(useMCrec) tr->Branch("gen_hadXF",     gen_hadXF,     "gen_hadXF[2]/F");
  Float_t gen_hadVertex[2][3]{{0,0,0},{0,0,0}}; if(useMCrec) tr->Branch("gen_hadVertex", gen_hadVertex, "gen_hadVertex[2][3]/F");
  // --- generated dihadron branches
  Float_t gen_Mh{0};      if(useMCrec) tr->Branch("gen_Mh",      &gen_Mh,      "gen_Mh/F");
  Float_t gen_Mmiss{0};   if(useMCrec) tr->Branch("gen_Mmiss",   &gen_Mmiss,   "gen_Mmiss/F");
  Float_t gen_Z[2]{0,0};  if(useMCrec) tr->Branch("gen_Z",       gen_Z,        "gen_Z[2]/F");
  Float_t gen_Zpair{0};   if(useMCrec) tr->Branch("gen_Zpair",   &gen_Zpair,   "gen_Zpair/F");
  Float_t gen_xF{0};      if(useMCrec) tr->Branch("gen_xF",      &gen_xF,      "gen_xF/F");
  Float_t gen_alpha{0};   if(useMCrec) tr->Branch("gen_alpha",   &gen_alpha,   "gen_alpha/F");
  Float_t gen_theta{0};   if(useMCrec) tr->Branch("gen_theta",   &gen_theta,   "gen_theta/F");
  Float_t gen_zeta{0};    if(useMCrec) tr->Branch("gen_zeta",    &gen_zeta,    "gen_zeta/F");
  Float_t gen_Ph{0};      if(useMCrec) tr->Branch("gen_Ph",      &gen_Ph,      "gen_Ph/F");
  Float_t gen_PhPerp{0};  if(useMCrec) tr->Branch("gen_PhPerp",  &gen_PhPerp,  "gen_PhPerp/F");
  Float_t gen_PhEta{0};   if(useMCrec) tr->Branch("gen_PhEta",   &gen_PhEta,   "gen_PhEta/F");
  Float_t gen_PhPhi{0};   if(useMCrec) tr->Branch("gen_PhPhi",   &gen_PhPhi,   "gen_PhPhi/F");
  Float_t gen_R{0};       if(useMCrec) tr->Branch("gen_R",       &gen_R,       "gen_R/F");
  Float_t gen_RPerp{0};   if(useMCrec) tr->Branch("gen_RPerp",   &gen_RPerp,   "gen_RPerp/F");
  Float_t gen_RT{0};      if(useMCrec) tr->Branch("gen_RT",      &gen_RT,      "gen_RT/F");
  Float_t gen_PhiH{0};    if(useMCrec) tr->Branch("gen_PhiH",    &gen_PhiH,    "gen_PhiH/F");
  Float_t gen_PhiRq{0};   if(useMCrec) tr->Branch("gen_PhiRq",   &gen_PhiRq,   "gen_PhiRq/F");
  Float_t gen_PhiRp{0};   if(useMCrec) tr->Branch("gen_PhiRp",   &gen_PhiRp,   "gen_PhiRp/F");
  Float_t gen_PhiRp_r{0}; if(useMCrec) tr->Branch("gen_PhiRp_r", &gen_PhiRp_r, "gen_PhiRp_r/F");
  Float_t gen_PhiRp_g{0}; if(useMCrec) tr->Branch("gen_PhiRp_g", &gen_PhiRp_g, "gen_PhiRp_g/F");
  // --- match quality
  Bool_t  gen_eleIsMatch{false};          if(useMCrec) tr->Branch("gen_eleIsMatch",   &gen_eleIsMatch,   "gen_eleIsMatch/O");
  Bool_t  gen_hadIsMatch[2]{false,false}; if(useMCrec) tr->Branch("gen_hadIsMatch",   gen_hadIsMatch,    "gen_hadIsMatch[2]/O");
  Float_t gen_eleMatchDist{0};            if(useMCrec) tr->Branch("gen_eleMatchDist", &gen_eleMatchDist, "gen_eleMatchDist/F");
  Float_t gen_hadMatchDist[2]{0,0};       if(useMCrec) tr->Branch("gen_hadMatchDist", gen_hadMatchDist,  "gen_hadMatchDist[2]/F");
  // --- parent info
  Int_t gen_hadParentIdx[2]{0,0}; if(useMCrec || useMCgen) tr->Branch("gen_hadParentIdx", gen_hadParentIdx, "gen_hadParentIdx[2]/I");
  Int_t gen_hadParentPid[2]{0,0}; if(useMCrec || useMCgen) tr->Branch("gen_hadParentPid", gen_hadParentPid, "gen_hadParentPid[2]/I");


  // ==================================================================================
  // event loop
  // ==================================================================================
  unsigned long evCount = 0;
  while(hipoReader.next(hipoBanks)) {
    evCount++;
    // if(evCount>500) { std::cout << "stopping prematurely (limiter)" << std::endl; break; } // limiter
    if(evCount % 100000 == 0) std::cout << "read " << evCount << " events" << std::endl;

    // iguana
    iguanaSeq.Run(hipoBanks);

    // banks
    auto const& bank_config            = hipoBanks.at(b_config);
    auto const& bank_event             = hipoBanks.at(b_event);
    auto const& bank_particle          = hipoBanks.at(b_particle);
    auto const& bank_calorimeter       = hipoBanks.at(b_calorimeter);
    auto const& bank_sector_finder     = hipoBanks.at(b_sector_finder);
    auto const& bank_inclusive_kin     = hipoBanks.at(b_inclusive_kin);
    auto const& bank_dihadron_kin      = hipoBanks.at(b_dihadron_kin);
    auto const& bank_single_hadron_kin = hipoBanks.at(b_single_hadron_kin);

    // make sure we have some data
    if(bank_config.getRows() == 0 || bank_event.getRows() == 0)
      continue;
    if(bank_inclusive_kin.getRows() == 0)
      continue;
    if(bank_dihadron_kin.getRows() == 0 && bank_single_hadron_kin.getRows() == 0)
      continue;

    // get event-level information
    runnum   = bank_config.getInt("run", 0);
    evnum    = bank_config.getInt("event", 0);
    helicity = bank_event.getByte("helicity", 0);

    // get inclusive info
    W     = bank_inclusive_kin.getDouble("W", 0);
    Q2    = bank_inclusive_kin.getDouble("Q2", 0);
    Nu    = bank_inclusive_kin.getDouble("nu", 0);
    x     = bank_inclusive_kin.getDouble("x", 0);
    y     = bank_inclusive_kin.getDouble("y", 0);
    beamE = std::hypot(bank_inclusive_kin.getDouble("beamPz", 0), PartMass(kE));

    // get scattered electron
    eleRow = bank_inclusive_kin.getShort("pindex", 0);
    ROOT::Math::PxPyPzMVector eleVec(
        bank_particle.getFloat("px", eleRow),
        bank_particle.getFloat("py", eleRow),
        bank_particle.getFloat("pz", eleRow),
        PartMass(kE)
        );
    eleE         = eleVec.E();
    eleP         = eleVec.P();
    elePt        = eleVec.Pt();
    eleEta       = eleVec.Eta();
    elePhi       = eleVec.Phi();
    eleVertex[0] = bank_particle.getFloat("vx", eleRow);
    eleVertex[1] = bank_particle.getFloat("vy", eleRow);
    eleVertex[2] = bank_particle.getFloat("vz", eleRow);
    eleStatus    = bank_particle.getShort("status", eleRow);
    eleChi2pid   = bank_particle.getFloat("chi2pid", eleRow);
    eleFiduCut   = true; // iguana did the fiducial cuts

    // get calorimeter info for electron
    elePCALen  = UNDEF;
    eleECINen  = UNDEF;
    eleECOUTen = UNDEF;
    for(const auto& row : bank_calorimeter.getRowList()) {
      if(bank_calorimeter.getShort("pindex", row) == eleRow) {
        switch(bank_calorimeter.getByte("layer", row)) {
          case 1:
            elePCALen = bank_calorimeter.getFloat("energy", row);
            break;
          case 4:
            eleECINen = bank_calorimeter.getFloat("energy", row);
            break;
          case 7:
            eleECOUTen = bank_calorimeter.getFloat("energy", row);
            break;
        }
      }
    }

    // get electron sector
    assert(( bank_sector_finder.getShort("pindex", eleRow) == eleRow ));
    eleSector = bank_sector_finder.getInt("sector", eleRow);

    // handle single hadrons
    for(const auto& kinRow : bank_single_hadron_kin.getRowList()) {
      hadIdx[0] = PIDtoIdx(bank_single_hadron_kin.getInt("pdg", kinRow));
      hadIdx[1] = kNix;
      pairType = EncodePairType(hadIdx[0], hadIdx[1]);

      hadRow[0] = bank_single_hadron_kin.getShort("pindex", kinRow);
      ROOT::Math::PxPyPzMVector hadVec(
          bank_particle.getFloat("px", hadRow[0]),
          bank_particle.getFloat("py", hadRow[0]),
          bank_particle.getFloat("pz", hadRow[0]),
          PartMass(hadIdx[0])
          );
      hadE[0]         = hadVec.E();
      hadP[0]         = hadVec.P();
      hadPt[0]        = hadVec.Pt();
      hadEta[0]       = hadVec.Eta();
      hadPhi[0]       = hadVec.Phi();
      hadXF[0]        = bank_single_hadron_kin.getDouble("xF", kinRow);
      hadVertex[0][0] = bank_particle.getFloat("vx", hadRow[0]);
      hadVertex[0][1] = bank_particle.getFloat("vy", hadRow[0]);
      hadVertex[0][2] = bank_particle.getFloat("vz", hadRow[0]);
      hadStatus[0]    = bank_particle.getShort("status", hadRow[0]);
      hadBeta[0]      = bank_particle.getFloat("beta", hadRow[0]);
      hadChi2pid[0]   = bank_particle.getFloat("chi2pid", hadRow[0]);
      hadFiduCut[0]   = true; // iguana did the fiducial cuts

      hadRow[1]       = UNDEF;
      hadE[1]         = UNDEF;
      hadP[1]         = UNDEF;
      hadPt[1]        = UNDEF;
      hadEta[1]       = UNDEF;
      hadPhi[1]       = UNDEF;
      hadXF[1]        = UNDEF;
      hadVertex[1][0] = UNDEF;
      hadVertex[1][1] = UNDEF;
      hadVertex[1][2] = UNDEF;
      hadStatus[1]    = UNDEF;
      hadBeta[1]      = UNDEF;
      hadChi2pid[1]   = UNDEF;
      hadFiduCut[1]   = false;

      Mh       = PartMass(hadIdx[0]);
      Mmiss    = bank_single_hadron_kin.getDouble("MX", kinRow);
      Z[0]     = bank_single_hadron_kin.getDouble("z", kinRow);
      Z[1]     = UNDEF;
      Zpair    = Z[0];
      xF       = hadXF[0];
      alpha    = UNDEF;
      theta    = UNDEF;
      thetaLI  = UNDEF;
      zeta     = UNDEF;
      Ph       = hadP[0];
      PhPerp   = bank_single_hadron_kin.getDouble("PhPerp", kinRow);
      PhEta    = hadEta[0];
      PhPhi    = hadPhi[0];
      R        = UNDEF;
      RPerp    = UNDEF;
      RT       = UNDEF;
      PhiH     = bank_single_hadron_kin.getDouble("phiH", kinRow);
      PhiRq    = UNDEF;
      PhiRp    = UNDEF;
      PhiRp_r  = UNDEF;
      PhiRp_g  = UNDEF;
      sdmePhiU = UNDEF;
      sdmePhiU = UNDEF;

      // fill the output tree for this single hadron
      tr->Fill();
    }

    // handle dihadrons
    for(const auto& kinRow : bank_dihadron_kin.getRowList()) {
      hadIdx[0] = PIDtoIdx(bank_dihadron_kin.getInt("pdg_a", kinRow));
      hadIdx[1] = PIDtoIdx(bank_dihadron_kin.getInt("pdg_b", kinRow));
      pairType  = EncodePairType(hadIdx[0], hadIdx[1]);

      hadRow[0] = bank_dihadron_kin.getShort("pindex_a", kinRow);
      hadRow[1] = bank_dihadron_kin.getShort("pindex_b", kinRow);

      ROOT::Math::PxPyPzMVector hadVec[2];

      for(int h = 0; h < 2; h++) {
        hadVec[h] = ROOT::Math::PxPyPzMVector(
          bank_particle.getFloat("px", hadRow[h]),
          bank_particle.getFloat("py", hadRow[h]),
          bank_particle.getFloat("pz", hadRow[h]),
          PartMass(hadIdx[h])
          );
        hadE[h]         = hadVec[h].E();
        hadP[h]         = hadVec[h].P();
        hadPt[h]        = hadVec[h].Pt();
        hadEta[h]       = hadVec[h].Eta();
        hadPhi[h]       = hadVec[h].Phi();
        hadXF[h]        = bank_dihadron_kin.getDouble("xF", kinRow);
        hadVertex[h][0] = bank_particle.getFloat("vx", hadRow[h]);
        hadVertex[h][1] = bank_particle.getFloat("vy", hadRow[h]);
        hadVertex[h][2] = bank_particle.getFloat("vz", hadRow[h]);
        hadStatus[h]    = bank_particle.getShort("status", hadRow[h]);
        hadBeta[h]      = bank_particle.getFloat("beta", hadRow[h]);
        hadChi2pid[h]   = bank_particle.getFloat("chi2pid", hadRow[h]);
        hadFiduCut[h]   = true; // iguana did the fiducial cuts
        Z[h]            = bank_single_hadron_kin.getDouble("z", hadRow[h]);
        assert((bank_single_hadron_kin.getShort("pindex", hadRow[h]) == hadRow[h])); // make sure pindex matches for single-hadron bank
      }

      auto dihadVec = hadVec[0] + hadVec[1];
      auto RVec     = 0.5 * (vecHad[0] - vecHad[1]);

      Mh       = bank_dihadron_kin.getDouble("Mh", kinRow);
      Mmiss    = bank_dihadron_kin.getDouble("MX", kinRow);
      Zpair    = bank_dihadron_kin.getDouble("z", kinRow);
      xF       = bank_dihadron_kin.getDouble("xF", kinRow);
      alpha    = UNDEF;
      theta    = bank_dihadron_kin.getDouble("theta", kinRow);
      thetaLI  = UNDEF;
      zeta     = UNDEF;
      Ph       = dihadVec.P();
      PhPerp   = bank_dihadron_kin.getDouble("PhPerp", kinRow);
      PhEta    = dihadVec.Eta();
      PhPhi    = dihadVec.Phi();
      R        = RVec.P();
      RPerp    = UNDEF;
      RT       = UNDEF;
      PhiH     = bank_dihadron_kin.getDouble("phiH", kinRow);
      PhiRq    = UNDEF;
      PhiRp    = bank_dihadron_kin.getDouble("phiR", kinRow);
      PhiRp_r  = UNDEF;
      PhiRp_g  = UNDEF;
      sdmePhiU = UNDEF;
      sdmePhiU = UNDEF;

      // fill the output tree for this dihadron
      tr->Fill();
    }
  }

  // write output and clean up
  iguanaSeq.Stop();
  tr->Write();
  outputFile->Close();
  std::cout << "PRODUCED: " << outputFileName << std::endl;
  return 0;
}
