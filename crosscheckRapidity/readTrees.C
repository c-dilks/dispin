void readTrees() {
  TFile * outfile = new TFile("tree.root","RECREATE");
  enum names {harut,chris};
  TTree * tr[2];
  tr[harut] = new TTree();
  tr[harut]->ReadFile("myb2bout-run5047.fixed","evnum/D:status/D:helicity/D:epx/D:epy/D:epx/D:p1px/D:p1py/D:p1pz/D:p2px/D:p2py/D:p2pz/D:x/D:y/D:Q2/D:Q/D:z2/D:pT2/D:phi2/D:xF2/D:zeta/D:pT1/D:xF1/D:phi1/D:Mx_ePpi/D:Mx_epi/D:eta1_gN/D:eta2_gN/D:eta1_Br/D:eta2_Br/D:eta1_gNstar");
  tr[chris] = new TTree();
  tr[chris]->ReadFile("eventTable.txt","evntnum/D:Lab_proton_z/D:gN_proton_z/D:Breit_proton_z/D:gN_proton_rapidity/D:Breit_proton_rapidity/D:xF_proton/D:Lab_piPlus_z/D:gN_piPlus_z/D:Breit_piPlus_z/D:gN_piPlus_rapidity/D:Breit_piPlus_rapidity/D:xF_piPlus");
  tr[harut]->Write("harut");
  tr[chris]->Write("chris");

  Double_t evnum[2];
  Double_t Y_breit_proton[2];
  Double_t Y_CM_proton[2];
  Double_t Y_breit_pion[2];
  Double_t Y_CM_pion[2];

  tr[chris]->SetBranchAddress("evntnum",&evnum[chris]);
  tr[harut]->SetBranchAddress("evnum",&evnum[harut]);

  tr[chris]->SetBranchAddress("Breit_piPlus_rapidity",&Y_breit_pion[chris]);
  tr[chris]->SetBranchAddress("Breit_proton_rapidity",&Y_breit_proton[chris]);
  tr[harut]->SetBranchAddress("eta1_Br",&Y_breit_proton[harut]);
  tr[harut]->SetBranchAddress("eta2_Br",&Y_breit_pion[harut]);

  tr[chris]->SetBranchAddress("gN_piPlus_rapidity",&Y_CM_pion[chris]);
  tr[chris]->SetBranchAddress("gN_proton_rapidity",&Y_CM_proton[chris]);
  tr[harut]->SetBranchAddress("eta1_gN",&Y_CM_proton[harut]);
  tr[harut]->SetBranchAddress("eta2_gN",&Y_CM_pion[harut]);

  for(Long64_t c=0; c<tr[chris]->GetEntries(); c++) {
    tr[chris]->GetEntry(c);
    for(Long64_t h=0; h<tr[harut]->GetEntries(); h++) {
      tr[harut]->GetEntry(h);
      if(evnum[chris]==evnum[harut]) {
        printf("EVENT %.0f     harut   chris   diff=|harut|-|chris|\n",evnum[chris]);
        printf(" Y_breit_proton: %.3f  %.3f  diff=%.3f\n",
          Y_breit_proton[harut],Y_breit_proton[chris],
          abs(Y_breit_proton[harut])-abs(Y_breit_proton[chris]));
        printf(" Y_CM_proton:    %.3f  %.3f  diff=%.3f\n",
          Y_CM_proton[harut],Y_CM_proton[chris],
          abs(Y_CM_proton[harut])-abs(Y_CM_proton[chris]));
        printf(" Y_breit_pion:   %.3f  %.3f  diff=%.3f\n",
          Y_breit_pion[harut],Y_breit_pion[chris],
          abs(Y_breit_pion[harut])-abs(Y_breit_pion[chris]));
        printf(" Y_CM_pion:      %.3f  %.3f  diff=%.3f\n",
          Y_CM_pion[harut],Y_CM_pion[chris],
          abs(Y_CM_pion[harut])-abs(Y_CM_pion[chris]));
      };
    };
  };

};

