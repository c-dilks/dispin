#include <string>
#include <filesystem>
#include <mutex>

#include <hipo4/reader.h>

#include <TFile.h>

#include "src/Tools.h"
#include "src/Constants.h"


int main(int argc, char** argv) {

  // arguments
  TString hipoFileName;
  TString dataStream = "data";
  TString hipoType   = "skim";
  if(argc == 1) {
    std::cerr << "USAGE: " << argv[0] << " [hipoFileName] [dataStream] [hipoType]" << std::endl;
    std::cerr << "  [hipoFileName] the HIPO file to analyze" << std::endl;
    std::cerr << "  [dataStream]   'data', 'mcrec', or 'mcgen'; default = " << dataStream << std::endl;
    std::cerr << "  [hipoType]     'skim' or 'dst'; default = " << hipoType << std::endl;
    return 1;
  }
  hipoFileName          = TString(argv[1]);
  if(argc>2) dataStream = TString(argv[2]);
  if(argc>3) hipoType   = TString(argv[3]);

  // OPTIONS ///////////////////////////////////////////////////////////////////////
  bool const verbose = false;
  std::vector<particle_enum> const hadronAnaList       = { kPip, kPim };
  std::vector<particle_enum> const singleHadronAnaList = { kP };
  //////////////////////////////////////////////////////////////////////////////////

  // define output ROOT file
  TString diskimFileName = TString("diskim/") + TString(std::filesystem::path(hipoFileName.Data()).filename()) + TString(".root");
  std::cout << "INPUT FILE:  " << hipoFileName << std::endl;
  std::cout << "OUTPUT FILE: " << diskimFileName << std::endl;
  auto diskimFile = new TFile(diskimFileName, "RECREATE");

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

  // read the input HIPO file, and decide which banks to read
  hipo::reader hipoReader(hipoFileName.Data(), {0});
  enum bankEnum {
    b_event,
    b_config,
    b_particle,
    b_calorimeter,
    b_track,
    b_traj,
    b_mc_particle,
    b_mc_lund
  };
  std::map<bankEnum,std::string> bankNamesMap;
  bankNamesMap.insert({b_event,       "REC::Event"});
  bankNamesMap.insert({b_config,      "RUN::config"});
  bankNamesMap.insert({b_particle,    "REC::Particle"});
  bankNamesMap.insert({b_calorimeter, "REC::Calorimeter"});
  bankNamesMap.insert({b_track,       "REC::Track"});
  bankNamesMap.insert({b_traj,        "REC::Traj"});
  if(useMCrec) {
    bankNamesMap.insert({b_mc_particle, "MC::Particle"});
    bankNamesMap.insert({b_mc_lund,     "MC::Lund"});
  }
  std::vector<std::string> bankNamesList;
  for(auto const [_, name] : bankNamesMap)
    bankNamesList.push_back(name);
  hipo::banklist hipoBanks = hipoReader.getBanks(bankNamesList);

  // build list of dihadrons' PDGs
  std::vector<std::pair<int,int>> dihadronAnaList;
  for(auto const idxA : hadronAnaList) {
    for(auto const idxB : hadronAnaList) {
      if(CorrectOrder(idxA, idxB)) {
        dihadronAnaList.push_back({idxA, idxB});
      }
    }
  }
  std::cout << "ANALYSING THE FOLLOWING DIHADRONS:" << std::endl;
  for(auto const& [idxA, idxB] : dihadronAnaList)
    std::cout << " - " << PartName(idxA) << ", " << PartName(idxB) << std::endl;
  std::cout << "ANALYSING THE FOLLOWING SINGLE HADRONS:" << std::endl;
  for(auto const& idx : singleHadronAnaList)
    std::cout << " - " << PartName(idx) << std::endl;
  return 0;


  // ==================================================================================
  // event loop
  // ==================================================================================
  std::once_flag print_once;
  unsigned long evCount = 0;
  while(hipoReader.next(hipoBanks)) {
    evCount++;
    if(evCount>500) { std::cout << "stopping prematurely (limiter)" << std::endl; break; } // limiter
    if(evCount % 100000 == 0) std::cout << "read " << evCount << " events" << std::endl;

    // get event-level information
    int helicity = hipoBanks.at(b_event).getByte("helicity", 0);
    int runnum   = hipoBanks.at(b_config).getInt("run", 0);
    int evnum    = hipoBanks.at(b_config).getInt("event", 0);
    std::call_once(print_once, [runnum]() { std::cout << "ANALYZING RUN " << runnum << std::endl; });

  }

  diskimFile->Close();
  return 0;
}
