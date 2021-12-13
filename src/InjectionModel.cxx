#include "InjectionModel.h"

ClassImp(InjectionModel)

InjectionModel::InjectionModel()
  : numModels(0)
  , verbose(true)
{
  BS = new Binning();
  RNG = new TRandomMixMax(14972); // fixed seed, for reproducibility
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
  this->GetAmplitudeModelList(modu)->AddLast(model);
}

TObjArray *InjectionModel::GetAmplitudeModelList(Modulation *modu) {
  TObjArray *ret;
  try { ret = models.at(modu->AmpName()); }
  catch(const std::out_of_range &ex) {
    fprintf(stderr,"ERROR: modulation %s not found in InjectionModel models\n",modu->AmpName().Data());
    return nullptr;
  }
  return ret;
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

Int_t InjectionModel::InjectHelicity(EventTree *ev, int modelNum) {

  // check modelNum
  if(modelNum<0 || modelNum>numModels) {
    fprintf(stderr,"ERROR: bad modelNum in InjectionModel::InjectHelicity\n");
    return UNDEF;
  }

  // assign `iv[3]` to IV values from EventTree, according to Binning scheme
  Float_t iv[3] = {0,0,0};
  for(int d=0; d<BS->dimensions; d++) {
    TString ivN = BS->GetIVname(d);
    if(ivN=="Mh")          iv[d] = ev->gen_Mh;
    else if(ivN=="X")      iv[d] = ev->gen_x;
    else if(ivN=="Z")      iv[d] = ev->gen_Zpair;
    else if(ivN=="PhPerp") iv[d] = ev->gen_PhPerp;
    else if(ivN=="Q2")     iv[d] = ev->gen_Q2;
    else if(ivN=="XF")     iv[d] = ev->gen_xF;
    else {
      fprintf(stderr,"ERROR: %s not mapped in InjectionModel::InjectHelicity\n",ivN.Data());
      return UNDEF;
    }
  }

  // assign modulation variables from EventTree
  Float_t phiH,phiR,theta;
  phiH  = ev->gen_PhiH;
  phiR  = ev->gen_PhiR;
  theta = ev->gen_theta;
  
  // calculate amplitude and modulation amount for the given modelNum; then calculate the
  // asymmetry we will inject
  TObject *modl;
  Float_t amp,moduVal;
  Float_t asymInj = 0;
  for(auto modu : moduList) {

    // get model
    modl = this->GetAmplitudeModelList(modu)->At(modelNum);
    if(modl==nullptr) {
      fprintf(stderr,"ERROR: modelNum %d not found in InjectionModel models\n",modelNum);
      return UNDEF;
    }
    
    // calculate amplitude
    switch(BS->dimensions) {
      case 1: amp = ((TF1*)modl)->Eval(iv[0]);             break;
      case 2: amp = ((TF2*)modl)->Eval(iv[0],iv[1]);       break;
      case 3: amp = ((TF3*)modl)->Eval(iv[0],iv[1],iv[2]); break;
    };

    // calculate modulation amount
    moduVal = modu->Evaluate(phiR,phiH,theta);

    // calculate asymmetry
    asymInj += amp * moduVal * ev->Polarization() * ev->GetDepolarizationRatio(modu->GetTw()); // TODO [low prio]: depol ratio should be calculated from generated kinematics
  }

  // calculate helicity: 2=spin-, 3=spin+
  Float_t rn = RNG->Uniform(); // generate random number within [0,1]
  return (rn<0.5*(1+asymInj)) ? 3 : 2;
}


void InjectionModel::WriteOut() {
  this->CountAmplitudeModels();
  this->Write("IM");
  for(auto kv : models) kv.second->Write(kv.first,TObject::kSingleKey);
}

InjectionModel::~InjectionModel() {};
