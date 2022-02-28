// determine the ratio of inbending to outbending data
void inbendingOutbendingYieldRatio(TString dataset="rga",Bool_t terse=false) {

  // open
  enum {ti,to}; // inbending, outbending
  TFile *infile[2];
  TString prefix = (dataset=="mc") ? "catTreeMC" : "catTreeData";
  infile[ti] = new TFile(Form("%s.%s.inbending.all.idx.root",prefix.Data(),dataset.Data()),"READ");
  infile[to] = new TFile(Form("%s.%s.outbending.all.idx.root",prefix.Data(),dataset.Data()),"READ");

  // count
  TTree *tr[2];
  Long64_t n[2];
  for(int t=0; t<2; t++) {
    tr[t] = (TTree*)infile[t]->Get("tree");
    n[t] = tr[t]->GetEntries();
  };
  Long64_t total = n[0]+n[1];

  // output
  if(terse) printf("%f\n",(Double_t)n[ti]/n[to]); // terse output inbending/outbending ratio, for scripts
  else {
    // note: `\thou` is defined as `\newcommand{\thou}[1]{\num[group-separator={,}]{#1}}` with package `siunitx`
    printf("\\item %s: inbending : outbending = $\\thou{%lld}~:~\\thou{%lld$} = $%.1f\\%%~:~%.1f\\%%$\n",
        dataset.Data(),
        n[ti],
        n[to],
        100.0*(Double_t)n[ti]/total,
        100.0*(Double_t)n[to]/total
        );
  };
};


