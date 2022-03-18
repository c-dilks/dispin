// StripTweights: strips the specified weights branch from the tree, and produces
// a `Tweights.root` file, needed to apply weights in brufit.C; the weights branch
// will remain in the tree, but brufit needs a separate tree with just weights
//
// - IMPORTANT: execute with `brufit StripTweights.C'(....)'` (you can do `brufit -b -q`, if you want)
//
void StripTweights(
    TString inFileName   = "catTreeMC.mc.bibending.all.idx.root", // input, indexed tree
    TString weightBranch = "Weight",                              // name of weight branch
    TString weightClass  = "IO",                                  // name of weight classification name
    TString treeName     = "tree"                                 // name of TTree in input file
    )
{
  // get input TTree
  TFile *inFile = new TFile(inFileName,"READ");
  TTree *inTree = (TTree*)inFile->Get(treeName);

  // set output TTree name
  TString outDir = "catTreeWeights/" + inFileName;
  outDir.ReplaceAll(".idx.root","");
  gROOT->ProcessLine(Form(".! mkdir -p %s",outDir.Data()));
  TString outFileName  = outDir+"/Tweights.root"; // name of output file (following sWeights convention)
  cout << "output file = " << outFileName << endl;

  // setup Brufit Weights object
  Weights *W = new Weights("HSsWeights"); // (default weight name for FitManager::Data().LoadWeights)
  W->SetFile(outFileName);
  W->SetSpecies(weightClass);
  W->SetIDName("Idx");

  // pull the weights
  W->WeightBySelection(inTree,"",weightBranch);
  W->Save();
  inFile->Close();
};
