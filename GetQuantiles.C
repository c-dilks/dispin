// computes quantiles for a variety of distributions in plots.root

const Int_t N = 14; // <-- number of quantiles
Double_t q[N];
Double_t p[N];
int i;

void PrintNums(TString name);
TFile * f;
TH1D * d;

void GetQuantiles(TString plotsFile="plots.root") {
  f = new TFile(plotsFile,"READ");

  PrintNums("XDist");
  PrintNums("MhDist");
  PrintNums("ZpairDist");
  PrintNums("PhPerpDist");
  PrintNums("PhDist");
  PrintNums("Q2Dist");
};


void PrintNums(TString name) {
  for(i=0; i<N; i++) p[i] = Double_t(i+1)/N;
  d = (TH1D*) f->Get(name);
  d->GetQuantiles(N,q,p);
  printf("%s:\n",name.Data());
  for(i=0; i<N-1; i++) printf("%.2f\n",q[i]);
  printf("\n");
};
