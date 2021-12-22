// check fit result of injected asymmetry against the injected model,
// given a fit result `asym_*.root` file and an injection model file `injection.root`

void checkInjectionFit(
    TString fitResultFileN="bruspin.inj0/asym_minuit_BL0.root",
    TString injectionModelFileN="injection.root"
    )
{
  TFile *fitResultFile      = new TFile(fitResultFileN,"READ");
  TFile *injectionModelFile = new TFile(injectionModelFileN,"READ");

  // find fit result `canvAsym*`, assuming there is only one
  TListIter nextKey(fitResultFile->GetListOfKeys());
  TString keyN;
  TCanvas *canvAsym = nullptr;
  while(TKey *key = (TKey*) nextKey()) {
    keyN = TString(key->GetName());
    if(keyN.Contains(TRegexp("^canvAsym_"))) {
      printf("Found asymmetry canvas %s in %s\n",keyN.Data(),fitResultFileN.Data());
      canvAsym = (TCanvas*) key->ReadObj();
      break;
    }
  }
  if(canvAsym==nullptr) {
    fprintf(stderr,"ERROR: cannot find canvAsym in %s\n",fitResultFileN.Data());
    return;
  }

  // loop over pads of canvAsym
  TListIter nextPad(canvAsym->GetListOfPrimitives());
  while(TPad *pad = (TPad*) nextPad()) {
    TListIter nextPrim(pad->GetListOfPrimitives());
    while(TObject *prim = (TObject*) nextPrim()) {
      if(prim->InheritsFrom(TGraph::Class())) {
        printf("%s\n",prim->GetName());
      }
    }
  }
}

