void systematicRadiative(
    TString outrootN = "outroot.tmp/45nA_job_3051_3.hipo.root",
    TString radrootN = "radroot.tmp/45nA_job_3051_3.hipo.root"
    )
{
  enum outrad {out,rad};
  EventTree *ev[2];
  for(i in ev[out] ){
    ev[out]->GetEvent(i);
    ev[rad]->GetEvent(i);

    // friend check (make sure we are looking at same dihadron)
    // - we do not use TTree friends, rather EventTrees for better control
    if( ev[out]->runnum     != ev[rad]->runnum     ||
        ev[out]->evnum      != ev[rad]->evnum      ||
        ev[out]->hadRow[qA] != ev[rad]->hadRow[qA] ||
        ev[out]->hadRow[qB] != ev[rad]->hadRow[qB]
      )
    {
      fprintf(stderr,"ERROR: event mis-match\n");
      return 1;
    }

    // check validity in outroot tree
    if(ev[out]->Valid()) {

      // find bin in current Binning scheme associated with ev[out]
      BS->FindBin(ev[out]);

      // fill correlation histograms
      corMmiss->Fill(ev[out]->Mmiss,ev[rad]->Mmiss);

      // FIR cut, where FIR = From Invalid Region, with RC-corrected beamE applied
      Bool_t firCut = ! ev[rad]->Valid();
      // Bool_t firCut = ev[rad]->Mmiss < 1.5;

      // fill histograms, needed to calculate firFrac = fir/ful
      if(firCut) { 
        firMmass->Fill(ev[out]->Mmiss);
      }
      fulMmass->Fill(ev[out]->Mmiss);
    };
  };
}
