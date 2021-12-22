#ifndef InjectionModel_
#define InjectionModel_

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <map>
#include <vector>

// ROOT
#include "TSystem.h"
#include "TObject.h"
#include "TString.h"
#include "TRandom.h"
#include "TRandomGen.h"
#include "TH1.h"
#include "TH2.h"
#include "TF1.h"
#include "TF2.h"
#include "TF3.h"

// dispin
#include "Binning.h"
#include "Modulation.h"
#include "EventTree.h"

class InjectionModel : public TObject
{
  public:
    InjectionModel();
    ~InjectionModel();

    // set IV type, by defining a Binning scheme with 1 bin; the Binning
    // scheme's ivVars will be associated with "x","y","z" of the model functions
    void SetIVtype(Int_t ivType);
    Binning *GetBinning() { return BS; };

    // modulations ---
    void AddModulation(Modulation *modu); // add a single modulation to `moduList` and `models`
    void FillModuList(int preset=0); // fill moduList, for a given `preset`
    std::vector<Modulation*> GetModuList() { return moduList; };

    // amplitude models ---
    // get the number of amplitude models, per Modulation
    Int_t GetNumModels() { return numModels; }
    // add an amplitude model to a specific `Modulation`; it is the caller's responsibility
    // to make sure all modulations in `moduList` get the same number of amplitude models
    void AddAmplitudeModel(Modulation *modu, TObject *model);
    // get list of models for a given Modulation
    TObjArray *GetAmplitudeModelList(Modulation *modu);
    // count the number of models, checking if all modulations have the same number 
    void CountAmplitudeModels();

    // injection ---
    // assign helicity, biased toward selected model
    Int_t InjectHelicity(EventTree *ev, int modelNum);

    // write to TFile; in addition to writing `this`, also write the models, for TBrowser convenience
    void WriteOut();

  private:
    int numModels;
    bool verbose;
    std::vector<Modulation*> moduList;
    std::map<TString,TObjArray*> models;
    Binning *BS;
    TRandom *RNG;
    bool firstInjection;

  ClassDef(InjectionModel,1);
};

#endif
