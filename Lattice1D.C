void Lattice1D() {

  TString injDir = "spinroot_injLattice1D";
  TString listFile = "moduUUlist.txt";

  TTree * tr = new TTree();
  tr->ReadFile(listFile,"i/I:title/C:b/F");
  Int_t i;
  Char_t title[256];
  Float_t b;
  tr->SetBranchAddress("i",&i);
  tr->SetBranchAddress("title",title);
  tr->SetBranchAddress("b",&b);

  TString infile0,infile1,newtitle,pngname;
  infile0 = Form("%s/spinroot_inj_%d/asym_42.root",injDir.Data(),0);

  //gROOT->LoadMacro("CompareAsyms.C");
  TString macro;
  for(int e=0; e<tr->GetEntries(); e++) {
    tr->GetEntry(e);
    //if(e>0) break; // limiter
    if(!(fabs(fabs(b)-0.3)<1e-4)) continue; // choose |B|

    infile1 = Form("%s/spinroot_inj_%d/asym_42.root",injDir.Data(),i);
    newtitle = Form("#delta[%.1f%s] for",b,title);
    pngname = Form("difference_%s%d.png",i<10?"0":"",i);

    macro = Form(".! root -b -q CompareAsyms.C'(\"%s\",\"%s\",false,\"%s\",\"%s\")'",
      infile0.Data(),infile1.Data(),newtitle.Data(),pngname.Data());
    gROOT->ProcessLine(macro);
  };
};
