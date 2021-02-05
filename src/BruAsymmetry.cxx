#include "BruAsymmetry.h"

ClassImp(BruAsymmetry)

BruAsymmetry::BruAsymmetry() {
  
  printf("construct.\n");
  HS::FIT::FitManager Fitter;
  Fitter.SetUp().SetOutDir("outPi2Amps/");

  Fitter.SetUp().LoadVariable("CosTh[0,-1,1]");
  Fitter.SetUp().LoadVariable("Phi[-3.14159,3.14159]");

};


BruAsymmetry::~BruAsymmetry() {
};
