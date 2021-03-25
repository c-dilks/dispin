// computes quantiles for a variety of distributions in plots.root
// -- use `constraint` string to specify a constraint, e.g., `abs(Mh-0.77)<0.1`;
//    this is useful for constructing multidimensional bins

const Int_t N = 100;
Double_t q[N];
Double_t p[N];
int i;
Int_t Nquant;
TString constraint;

void PrintNums(TString name);
TFile * f;
TTree * t;
TH1D * d;
TLine * l;

void GetQuantiles(
  Int_t Nquant_=18,
  TString constraint_="",
  TString catTreeFile="catTreeData.root"
) {
  Nquant = Nquant_;
  constraint = constraint_;
  if(Nquant>N) { fprintf(stderr,"ERROR: Nquant too big\n"); return; }
  f = new TFile(catTreeFile,"READ");
  t = (TTree*) f->Get("tree");

  PrintNums("X");
  PrintNums("Mh");
  PrintNums("Z");
  PrintNums("PhPerp");
  PrintNums("Q2");
  PrintNums("XF");
  PrintNums("DY");
};


void PrintNums(TString name) {

  Double_t min = t->GetMinimum(name);
  Double_t max = t->GetMaximum(name);
  min -= abs(max-min)*0.05;
  max += abs(max-min)*0.05;

  d = new TH1D(name+"_dist",name+" quantiles",100,min,max);
  t->Project(name+"_dist",name,constraint);
  new TCanvas();
  d->Draw();

  for(i=0; i<Nquant; i++) p[i] = Double_t(i+1)/Nquant;
  d->GetQuantiles(Nquant,q,p);

  for(i=0; i<Nquant-1; i++) {
    l = new TLine(q[i],0,q[i],d->GetMaximum());
    l->SetLineWidth(3);
    l->Draw();
  };

  printf("%s:\n",name.Data());
  for(i=0; i<Nquant-1; i++) printf("%.3f\n",q[i]);
  printf("\n");
};
