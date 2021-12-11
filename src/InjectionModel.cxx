#include "InjectionModel.h"

ClassImp(InjectionModel)

InjectionModel::InjectionModel()
  : numModels(0)
  , verbose(true)
{
  BS = new Binning();
};

void InjectionModel::SetIVtype(Int_t ivType) {
  BS->SetScheme(ivType,1,1,1); // single bin, useful for ranges
};

void InjectionModel::FillModuList(int preset) {
  if(preset==0) { // PWA, all 12 amps
    for(int tw=2; tw<=3; tw++) {
      for(int l=0; l<=2; l++) {
        for(int m=-l; m<=l; m++) {
          if(tw==2 && m<=0) continue;
          if(verbose) printf("modulation %d %d %d\n",tw,l,m);
          auto modu = new Modulation(tw,l,m,0,true,Modulation::kLU);
          this->AddModulation(modu);
        }
      }
    }
  }
  else fprintf(stderr,"ERROR: unknown preset in InjectionModel::FillModuList\n");
}

void InjectionModel::AddModulation(Modulation *modu) {
  moduList.push_back(modu);
  models.insert(std::pair<TString,TObjArray*>(modu->AmpName(),new TObjArray()));
}

void InjectionModel::AddAmplitudeModel(Modulation *modu, TObject *model) {
  try { models.at(modu->AmpName())->AddLast(model); }
  catch(const std::out_of_range &ex) {
    fprintf(stderr,"ERROR: modulation %s not in moduList\n",modu->AmpName().Data());
  }
}

void InjectionModel::CountAmplitudeModels() {
  int tmp;
  numModels = -1;
  for(auto kv : models) {
    tmp = kv.second->GetEntries();
    if(numModels<0) numModels = tmp;
    if(tmp!=numModels) {
      fprintf(stderr,"ERROR: modulations have different numbers of models\n");
      break;
    }
  }
}

void InjectionModel::WriteOut() {
  this->CountAmplitudeModels();
  this->Write("IM");
  for(auto kv : models) kv.second->Write(kv.first,TObject::kSingleKey);
}

InjectionModel::~InjectionModel() {};
