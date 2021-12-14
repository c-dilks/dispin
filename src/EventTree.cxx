#include "EventTree.h"

ClassImp(EventTree)

using namespace std;


EventTree::EventTree(TString filelist, Int_t whichPair_) {
  printf("EventTree instantiated\n");

  debug = true;

  DecodePairType(whichPair_,whichHad[qA],whichHad[qB]);
  printf("\n>>> DIHADRON SELECTION: %s\n\n",PairName(whichHad[qA],whichHad[qB]).Data());


  printf("reading tree chain from %s\n",filelist.Data());
  chain = new TChain("tree");
  chain->Add(filelist);

  ENT = chain->GetEntries();
  printf("number of entries: %lld\n",ENT);

  chain->SetBranchAddress("W",&W);
  chain->SetBranchAddress("Q2",&Q2);
  chain->SetBranchAddress("Nu",&Nu);
  chain->SetBranchAddress("x",&x);
  chain->SetBranchAddress("y",&y);

  chain->SetBranchAddress("eleE",&eleE);
  chain->SetBranchAddress("eleP",&eleP);
  chain->SetBranchAddress("elePt",&elePt);
  chain->SetBranchAddress("eleEta",&eleEta);
  chain->SetBranchAddress("elePhi",&elePhi);
  chain->SetBranchAddress("eleVertex",eleVertex);
  chain->SetBranchAddress("eleStatus",&eleStatus);
  chain->SetBranchAddress("eleChi2pid",&eleChi2pid);
  chain->SetBranchAddress("elePCALen",&elePCALen);
  chain->SetBranchAddress("eleECINen",&eleECINen);
  chain->SetBranchAddress("eleECOUTen",&eleECOUTen);
  chain->SetBranchAddress("eleSector",&eleSector);

  chain->SetBranchAddress("eleFiduCut",&eleFiduCut);
  chain->SetBranchAddress("hadFiduCut",hadFiduCut);

  chain->SetBranchAddress("pairType",&pairType);
  chain->SetBranchAddress("hadIdx",hadIdx);
  chain->SetBranchAddress("hadRow",hadRow);
  chain->SetBranchAddress("hadE",hadE);
  chain->SetBranchAddress("hadP",hadP);
  chain->SetBranchAddress("hadPt",hadPt);
  chain->SetBranchAddress("hadEta",hadEta);
  chain->SetBranchAddress("hadPhi",hadPhi);
  chain->SetBranchAddress("hadVertex",hadVertex);
  chain->SetBranchAddress("hadStatus",hadStatus);
  chain->SetBranchAddress("hadChi2pid",hadChi2pid);
  chain->SetBranchAddress("hadBeta",hadBeta);

  // these branches were temporarily used for early crosscheck studies
  if(chain->GetBranch("hadPtq")) chain->SetBranchAddress("hadPtq",hadPtq);
  else { for(int h=0; h<2; h++) hadPtq[h]=UNDEF; };
  if(chain->GetBranch("hadXF")) chain->SetBranchAddress("hadXF",hadXF);
  else { for(int h=0; h<2; h++) hadXF[h]=UNDEF; };
  /////////////

  chain->SetBranchAddress("Mh",&Mh);
  chain->SetBranchAddress("Mmiss",&Mmiss);
  chain->SetBranchAddress("Z",Z);
  chain->SetBranchAddress("Zpair",&Zpair);
  chain->SetBranchAddress("xF",&xF);
  chain->SetBranchAddress("alpha",&alpha);
  chain->SetBranchAddress("theta",&theta);
  if(chain->GetBranch("zeta")) chain->SetBranchAddress("zeta",&zeta);
  else zeta = UNDEF;

  chain->SetBranchAddress("Ph",&Ph);
  chain->SetBranchAddress("PhPerp",&PhPerp);
  chain->SetBranchAddress("PhEta",&PhEta);
  chain->SetBranchAddress("PhPhi",&PhPhi);
  chain->SetBranchAddress("R",&R);
  chain->SetBranchAddress("RPerp",&RPerp);
  chain->SetBranchAddress("RT",&RT);

  chain->SetBranchAddress("PhiH",&PhiH);

  chain->SetBranchAddress("PhiRq",&PhiRq);
  chain->SetBranchAddress("PhiRp",&PhiRp);
  chain->SetBranchAddress("PhiRp_r",&PhiRp_r);
  chain->SetBranchAddress("PhiRp_g",&PhiRp_g);

  // diphoton branches
  objDiphoton = new Diphoton();
  if(chain->GetBranch("diphM")) {
    chain->SetBranchAddress("photE",objDiphoton->photE);
    chain->SetBranchAddress("photPt",objDiphoton->photPt);
    chain->SetBranchAddress("photEta",objDiphoton->photEta);
    chain->SetBranchAddress("photPhi",objDiphoton->photPhi);
    chain->SetBranchAddress("photAng",objDiphoton->photAng);
    chain->SetBranchAddress("photBeta",objDiphoton->photBeta);
    chain->SetBranchAddress("photChi2pid",objDiphoton->photChi2pid);
    chain->SetBranchAddress("diphM",&(objDiphoton->M));
    chain->SetBranchAddress("diphZE",&(objDiphoton->ZE));
    chain->SetBranchAddress("diphVtxDiff",&(objDiphoton->VtxDiff));
    if(chain->GetBranch("diphMCpi0")) {
      chain->SetBranchAddress("diphMCpi0",&(objDiphoton->IsMCpi0));
      chain->SetBranchAddress("diphMCmatchDist",&(objDiphoton->MCmatchDist));
    };
  };


  // event-level branches
  chain->SetBranchAddress("runnum",&runnum);
  chain->SetBranchAddress("evnum",&evnum);
  chain->SetBranchAddress("helicity",&helicity);

  // MC branches
  if(chain->GetBranch("gen_hadMatchDist")) {
    MCrecMode = true;
    chain->SetBranchAddress("gen_W",&gen_W);
    chain->SetBranchAddress("gen_Q2",&gen_Q2);
    chain->SetBranchAddress("gen_Nu",&gen_Nu);
    chain->SetBranchAddress("gen_x",&gen_x);
    chain->SetBranchAddress("gen_y",&gen_y);
    // - generated electron kinematics branches
    chain->SetBranchAddress("gen_eleE",&gen_eleE);
    chain->SetBranchAddress("gen_eleP",&gen_eleP);
    chain->SetBranchAddress("gen_elePt",&gen_elePt);
    chain->SetBranchAddress("gen_eleEta",&gen_eleEta);
    chain->SetBranchAddress("gen_elePhi",&gen_elePhi);
    chain->SetBranchAddress("gen_eleVertex",gen_eleVertex);
    // - generated hadron branches
    chain->SetBranchAddress("gen_pairType",&gen_pairType);
    chain->SetBranchAddress("gen_hadRow",gen_hadRow);
    chain->SetBranchAddress("gen_hadIdx",gen_hadIdx);
    chain->SetBranchAddress("gen_hadE",gen_hadE);
    chain->SetBranchAddress("gen_hadP",gen_hadP);
    chain->SetBranchAddress("gen_hadPt",gen_hadPt);
    chain->SetBranchAddress("gen_hadEta",gen_hadEta);
    chain->SetBranchAddress("gen_hadPhi",gen_hadPhi);
    chain->SetBranchAddress("gen_hadXF",gen_hadXF);
    chain->SetBranchAddress("gen_hadVertex",gen_hadVertex);
    // - generated dihadron branches
    chain->SetBranchAddress("gen_Mh",&gen_Mh);
    chain->SetBranchAddress("gen_Mmiss",&gen_Mmiss);
    chain->SetBranchAddress("gen_Z",gen_Z);
    chain->SetBranchAddress("gen_Zpair",&gen_Zpair);
    chain->SetBranchAddress("gen_xF",&gen_xF);
    chain->SetBranchAddress("gen_alpha",&gen_alpha);
    chain->SetBranchAddress("gen_theta",&gen_theta);
    chain->SetBranchAddress("gen_zeta",&gen_zeta);
    chain->SetBranchAddress("gen_Ph",&gen_Ph);
    chain->SetBranchAddress("gen_PhPerp",&gen_PhPerp);
    chain->SetBranchAddress("gen_PhEta",&gen_PhEta);
    chain->SetBranchAddress("gen_PhPhi",&gen_PhPhi);
    chain->SetBranchAddress("gen_R",&gen_R);
    chain->SetBranchAddress("gen_RPerp",&gen_RPerp);
    chain->SetBranchAddress("gen_RT",&gen_RT);
    chain->SetBranchAddress("gen_PhiH",&gen_PhiH);
    chain->SetBranchAddress("gen_PhiRq",&gen_PhiRq);
    chain->SetBranchAddress("gen_PhiRp",&gen_PhiRp);
    chain->SetBranchAddress("gen_PhiRp_r",&gen_PhiRp_r);
    chain->SetBranchAddress("gen_PhiRp_g",&gen_PhiRp_g);
    // - match quality
    chain->SetBranchAddress("gen_eleIsMatch",&gen_eleIsMatch);
    chain->SetBranchAddress("gen_hadIsMatch",gen_hadIsMatch);
    chain->SetBranchAddress("gen_eleMatchDist",&gen_eleMatchDist);
    chain->SetBranchAddress("gen_hadMatchDist",gen_hadMatchDist);
    // - other
    chain->SetBranchAddress("gen_hadParentIdx",gen_hadParentIdx);
    chain->SetBranchAddress("gen_hadParentPid",gen_hadParentPid);
    // - helicityMC
    if(chain->GetBranch("helicityMC")) {
      chain->SetBranchAddress("NhelicityMC",&NhelicityMC);
      chain->SetBranchAddress("helicityMC",helicityMC);
      helicityMCinjected = true;
    } else {
      NhelicityMC = 0;
      for(int hh=0; hh<NumInjectionsMax; hh++) helicityMC[hh]=0;
      helicityMCinjected = false;
    };
  } else { 
    MCrecMode = false;
    for(int hh=0; hh<NumInjectionsMax; hh++) helicityMC[hh]=0; 
    helicityMCinjected = false;
    gen_W = UNDEF;
    gen_Q2 = UNDEF;
    gen_Nu = UNDEF;
    gen_x = UNDEF;
    gen_y = UNDEF;
    // - generated electron kinematics branches
    gen_eleE = UNDEF;
    gen_eleP = UNDEF;
    gen_elePt = UNDEF;
    gen_eleEta = UNDEF;
    gen_elePhi = UNDEF;
    for(int c=0;c<3;c++) gen_eleVertex[c] = UNDEF;
    // - generated hadron branches
    gen_pairType = -1;
    for(int h=0; h<2; h++) {
      gen_hadRow[h] = -1;
      gen_hadIdx[h] = -1;
      gen_hadE[h] = UNDEF;
      gen_hadP[h] = UNDEF;
      gen_hadPt[h] = UNDEF;
      gen_hadEta[h] = UNDEF;
      gen_hadPhi[h] = UNDEF;
      gen_hadXF[h] = UNDEF;
      gen_Z[h] = UNDEF;
      for(int c=0;c<3;c++) gen_hadVertex[h][c] = UNDEF;
    };
    // - generated dihadron branches
    gen_Mh = UNDEF;
    gen_Mmiss = UNDEF;
    gen_Zpair = UNDEF;
    gen_xF = UNDEF;
    gen_alpha = UNDEF;
    gen_theta = UNDEF;
    gen_zeta = UNDEF;
    gen_Ph = UNDEF;
    gen_PhPerp = UNDEF;
    gen_PhEta = UNDEF;
    gen_PhPhi = UNDEF;
    gen_R = UNDEF;
    gen_RPerp = UNDEF;
    gen_RT = UNDEF;
    gen_PhiH = UNDEF;
    gen_PhiR = UNDEF;
    gen_PhiRq = UNDEF;
    gen_PhiRp = UNDEF;
    gen_PhiRp_r = UNDEF;
    gen_PhiRp_g = UNDEF;
    // - match quality
    gen_eleIsMatch = false;
    gen_eleMatchDist = UNDEF;
    for(int h=0; h<2; h++) {
      gen_hadIsMatch[h] = false;
      gen_hadMatchDist[h] = UNDEF;
      gen_hadParentIdx[h] = -1;
      gen_hadParentPid[h] = -1;
    };
  };

  // instantiate useful objects
  objDihadron = new Dihadron();
  objDIS = new DIS();
  trEle = new Trajectory();
  trEle->Idx = kE;
  for(int h=0; h<2; h++) {
    trHad[h] = new Trajectory();
    trHad[h]->Idx = dihHadIdx(whichHad[qA],whichHad[qB],h);
  };
  whichHelicityMC = 0;

  RNG = new TRandomMixMax(14972); // seed

};


void EventTree::GetEvent(Long64_t i) {
  if(i%10000==0) printf("[+] %.2f%%\n",100*(float)i/((float)ENT));

  chain->GetEntry(i);

  // set preferred PhiR angle
  PhiR = PhiRp; // preferred definition by Bacchetta (see Dihadron.cxx)
  gen_PhiR = gen_PhiRp;

  PhiHR = Tools::AdjAngle( PhiH - PhiR );


  // adjust range to 0-2pi (for cross-checking with Timothy)
  /*
  PhiR = Tools::AdjAngleTwoPi(PhiR);
  PhiH = Tools::AdjAngleTwoPi(PhiH);
  PhiHR = Tools::AdjAngleTwoPi(PhiHR);
  */


  // DSIDIS angles // TODO: move this to calcKinematics.cpp; (will need to reproduce outroot files)
  this->GetDihadronObj(); // sets objDihadron
  for(int h=0; h<2; h++) {
    hadPhiH[h] = objDihadron->GetSingleHadronPhiH(h);
  };
  PhiD = Tools::AdjAngle(hadPhiH[qA]-hadPhiH[qB]);



  // theta symmetrization tests
  //theta = fabs( fabs(theta-PI/2.0) - PI/2.0 ); // HERMES symmetrization
  //if(Z[qB] > Z[qA]) theta = PI - theta; // Z symmetrization
  //theta = PI-theta; // full theta flip

  // convert eta to polar angle theta 
  // (not to be confused with partial wave theta)
  eleTheta = Tools::EtaToTheta(eleEta);
  gen_eleTheta = MCrecMode ? Tools::EtaToTheta(gen_eleEta) : UNDEF;
  for(int h=0; h<2; h++) {
    hadTheta[h] = Tools::EtaToTheta(hadEta[h]);
    gen_hadTheta[h] = MCrecMode ? Tools::EtaToTheta(gen_hadEta[h]) : UNDEF;
  };

  // compute gamma and epsilon
  // - epsilon is the ratio of longitudinal to transverse photon flux
  gamma = 2*PartMass(kP)*x / TMath::Sqrt(Q2);
  epsilon = ( 1 - y - TMath::Power(gamma*y,2)/4 ) /
    ( 1 - y + y*y/2 + TMath::Power(gamma*y,2)/4 );


  // compute rapidity for hadrons and dihadron
  // - get boost vectors
  this->GetDISObj();
  boostBreit = this->objDIS->BreitBoost; // boost to breit frame
  boostCom = this->objDIS->ComBoost; // boost to q-p com frame
  // - get q momentum, and boost (boost is redundant, being along q)
  qMomBreit = this->objDIS->vecQ; qMomBreit.Boost(boostBreit);
  qMomCom =   this->objDIS->vecQ; qMomCom.Boost(boostCom);
  // - calculate hadron rapidities
  for(int h=0; h<2; h++) {
    hadMom[h].SetPtEtaPhiE(hadPt[h],hadEta[h],hadPhi[h],hadE[h]);
    this->CalculateRapidity(
      hadMom[h], boostBreit, qMomBreit.Vect(), hadYH[h],  hadPqBreit[h] );
    this->CalculateRapidity(
      hadMom[h], boostCom,   qMomCom.Vect(),   hadYCM[h], hadPqCom[h]   );
    qmag = this->objDIS->vecQ.Vect().Mag();
    hadPqLab[h] = qmag*qmag>0 ?
      hadMom[h].Vect().Dot(this->objDIS->vecQ.Vect()) / qmag : 0;
  };
  // - calculate dihadron rapidity
  dihMom.SetPtEtaPhiM( Ph/TMath::CosH(PhEta), PhEta, PhPhi, Mh );
  this->CalculateRapidity(
    dihMom, boostBreit, qMomBreit.Vect(), YH,  dihPqBreit );
  this->CalculateRapidity(
    dihMom, boostCom,   qMomCom.Vect(),   YCM, dihPqCom   );
  // - calculate delta rapidity
  DYsgn = hadYH[qA] - hadYH[qB];
  DY = TMath::Abs(DYsgn);
  // TODO: move these rapidity calculations to calcKinematics.cpp; (will need
  // to reproduce outroot files; but also it's useful to have it here, in case
  // we want to check other frames)


  // compute hadron Pperp and qT
  // TODO: move this to calcKinematics.cpp; (will need to reproduce outroot files)
  for(int h=0; h<2; h++) {
    hadPperp[h] = objDihadron->hadPperp[h];
    hadQt[h] = hadPperp[h] / Z[h];
  }


  // take absolute value of Mmiss (rarely, it can be negative; see Dihadron.cxx)
  if(Mmiss<0) Mmiss *= -1;



  /**************************************/
  /* cut definitions                    */
  /**************************************/

  // DIS cuts
  cutQ2 = Q2 > 1.0; /* legacy */
  cutW = W > 2.0; /* legacy */
  cutY = y < 0.8;
  cutDIS = cutQ2 && cutW && cutY;


  // fragmentation region (FR) cuts
  yhb = 0.0; // y_h bound for TFR/CFR separation
  // Breit frame rapidity cuts
  for(int h=0; h<2; h++) {
    cutCFR[h] = hadYH[h] > yhb;
    cutTFR[h] = hadYH[h] < -yhb;
  };
  /*for(int h=0; h<2; h++) { // Prokudin cuts:
    cutCFR[h] = hadQt[h]<2 && Z[h]>0.2;
    cutTFR[h] = hadQt[h]>1 &&
      ( (x<0.45 && Z[h]<0.1) || (x>=0.45 && Z[h]<0.3) );
  };*/
  cutFR = hadXF[qA]>0 && hadXF[qB]>0; // PRL CFR
  //cutFR = true; // bypass
  //cutFR = cutCFR[qA] && cutCFR[qB]; // CFR/CFR
  //cutFR = cutCFR[qA] && cutTFR[qB]; // CFR/TFR
  //cutFR = cutTFR[qA] && cutCFR[qB]; // TFR/CFR
  //cutFR = cutTFR[qA] && cutTFR[qB]; // TFR/TFR
  //cutFR = hadXF[qA]<0 && hadXF[qB]<0; // xF<0 region
  //cutFR = cutCFR[qA]; // pi+ in CFR, pi- no cut
  //cutFR = Zpair>0.4; // try to look at decays from CFR rhos


  // diphoton cuts and classification
  // - cuts are applied in Diphoton::Classify(), which will
  //   assign an Idx if the diphoton passes cuts
  //   - if cuts fail, Idx remains as kDiph
  //   - if cuts pass, Idx is re-assigned based on
  //     classification of pi0 or pi0-BG
  // - later in cutDihadron, we will check the pairType; if
  //   you asked for a pair which includes a pi0, then 
  //   cutDihadron will only be true if the diphoton is
  //   classified as a pi0
  for(int h=0; h<2; h++) {
    if( hadIdx[h]==kDiph    /* if it's a diphoton, hadIdx is likely */
     || hadIdx[h]==kPio     /* only kDiph, but good to check the    */
     || hadIdx[h]==kPioBG   /* others, just in case...              */
     || hadIdx[h]==kDiphBasic
    ) {
      // mark this hadron as being a diphoton of some type,
      // since cuts below need to know this
      isDiphoton[h] = true;
      // classify diphoton
      objDiphoton->Classify();
      // assign hadIdx; note that if whichHad is kDiph, we
      // don't want to do this, since we're interested in
      // all diphotons
      if(whichHad[h]!=kDiph) hadIdx[h] = objDiphoton->GetIdx();
      // set objDiphoton variables which are not tree branches,
      // so that we can access them from objDiphoton too
      objDiphoton->E = hadE[h];
      objDiphoton->Pt = hadPt[h];
      objDiphoton->Eta = hadEta[h];
      objDiphoton->Phi = hadPhi[h];
      // if you want to see the full M_{gamma,gamma}
      // spectrum with EventTree::Valid() cuts and basic
      // cuts enabled, use pairType with kDiphBasic; we
      // check both whichHad[qA] and whichHad[qB], since
      // pairType order may not equal hadron order
      if(whichHad[qA]==kDiphBasic || whichHad[qB]==kDiphBasic) {
        // classify as kDiphBasic iff basic cuts satisfied
        if(objDiphoton->cutBasic) hadIdx[h] = kDiphBasic;
      };
    }
    else isDiphoton[h] = false;
  };


  // dihadron cuts
  /* (note: PairSame ensures we have the correct channel, e.g., pi+pi-) */
  cutDihadron = 
    Tools::PairSame(hadIdx[qA],hadIdx[qB],whichHad[qA],whichHad[qB]) &&
    Zpair < 0.95 && /* legacy; redundant with Mx>1.5 */
    CheckMissingMass()
    ;


  // vertex cuts
  cutVertex = CheckVertex(); /* applies to electron and hadrons */


  // fiducial cuts
  // - note: status is required to be in FD, as a prerequisite 
  cutFiducial = eleFiduCut && hadFiduCut[qA] && hadFiduCut[qB];


  // PID refinement cuts
  // -- electron
  cutElePID = 
    eleTheta>5 && eleTheta<35 &&
    eleP > 2 && /* legacy; redudant with y<0.8 cut */
    elePCALen > 0.07 &&
    CheckSampFrac(); /* sampling fraction cuts (diagonal cut and (mu,std) cut) */
  // -- pions
  for(int h=0; h<2; h++) {
    minP[h] = isDiphoton[h] ? 0.0 : 1.25;
    cutHadPID[h] = 
      hadTheta[h]>5 && hadTheta[h]<35 &&
      hadP[h] > minP[h] && /* minimum P cut only for charged hadrons */
      CheckHadChi2pid(h); /* refined hadron chi2pid cut */
  };
  cutPID = cutElePID && cutHadPID[qA] && cutHadPID[qB];
  

  // check if helicity is defined
  sps = this->SpinState();
  cutHelicity = sps==sP || sps==sM;


};


/////////////////////////////////////////////////////////
// MAIN ANALYSIS CUT
Bool_t EventTree::Valid() {
  return cutDIS && cutDihadron && cutHelicity && 
         cutFiducial && cutPID && cutVertex && cutFR;
  // NOTE: if you want to disable `cutDihadron`, you likely want to ensure `Tools::PairSame` is still checked
  //return Tools::PairSame(hadIdx[qA],hadIdx[qB],whichHad[qA],whichHad[qB]) && cutHelicity; // disable "all" cuts
};
/////////////////////////////////////////////////////////


// just get trajectories (faster than GetEvent, since
// no more kinematics are calculated, and cuts are not checked)
// - note: do not use this in the same loop as GetEvent; 
//   use the right function for the job
void EventTree::GetTrajectories(Long64_t i, Bool_t prog) {
  if(prog && i%10000==0) printf("[+] %.2f%%\n",100*(float)i/((float)ENT));

  chain->GetEntry(i);

  // get electron trajectory
  eleMom.SetPtEtaPhiE(elePt,eleEta,elePhi,eleE);
  trEle->Momentum = eleMom;
  trEle->Status = eleStatus;
  trEle->chi2pid = eleChi2pid;
  trEle->Vertex.SetXYZ(eleVertex[eX],eleVertex[eY],eleVertex[eZ]);

  // get hadron trajectories
  for(int h=0; h<2; h++) {
    hadMom[h].SetPtEtaPhiE(hadPt[h],hadEta[h],hadPhi[h],hadE[h]);
    trHad[h]->Momentum = hadMom[h];
    trHad[h]->Status = hadStatus[h];
    trHad[h]->chi2pid = hadChi2pid[h];
    trHad[h]->Beta = hadBeta[h];
    trHad[h]->Vertex.SetXYZ(hadVertex[h][eX],hadVertex[h][eY],hadVertex[h][eZ]);
    trHad[h]->Idx = hadIdx[h];
    trHad[h]->Row = hadRow[h];
    // MC
    trHad[h]->gen_parentIdx = gen_hadParentIdx[h];
    trHad[h]->gen_parentPid = gen_hadParentPid[h];
    trHad[h]->gen_isMatch = gen_hadIsMatch[h];
    trHad[h]->gen_matchDist = gen_hadMatchDist[h];
    trHad[h]->gen_E = gen_hadE[h];
  };
};


// translate "helicity" to a local index for the spin
Int_t EventTree::SpinState() {
  
  if(runnum!=11) { // data run
    if(RundepHelicityFlip(runnum)) { // helicity flipped
      switch(helicity) {
        case  1: return sM;
        case -1: return sP;
        case  0: return UNDEF;
      };
    } else { // helicity not flipped
      switch(helicity) {
        case  1: return sP;
        case -1: return sM;
        case  0: return UNDEF;
      };
    };
    fprintf(stderr,"WARNING: bad SpinState request: %d\n",helicity);
    return UNDEF;
  }

  else { // MC run - use injected helicity (DEPRECATED by `InjectionModel`)
    if(!helicityMCinjected) {
      // if helicityMC has not yet been injected, inject something here so cutHelicity==true
      //helicityMC[whichHelicityMC] = 3; // +helicity only
      helicityMC[whichHelicityMC] = RNG->Uniform()<0.5 ? 2:3; // 50/50 random
    };
    if(whichHelicityMC<0||whichHelicityMC>NhelicityMC) {
      fprintf(stderr,"ERROR: bad whichHelicityMC\n");
      return UNDEF;
    };
    switch(helicityMC[whichHelicityMC]) {
      case 2: return sM;
      case 3: return sP;
      case 0: return UNDEF;
      default: fprintf(stderr,"WARNING: bad SpinState request: %d\n",helicityMC[whichHelicityMC]);
    };
  };
  return UNDEF;
};


// return polarization, which can depend on the run number
Float_t EventTree::Polarization() {
  return RundepPolarization(runnum);
};

// return relative luminosity, which can depend on HWP position
Float_t EventTree::Rellum() {
  return 1; // disable for now, until we have a good measure of it
};


// return true if the event passes vertex cuts
Bool_t EventTree::CheckVertex() {

  // electron Vz cuts
  if(RundepTorus(runnum)==kInbending) {
    // vzBoolEle = -13 < eleVertex[eZ] && eleVertex[eZ] < 12; // inbending, RGA common cuts
    vzBoolEle = -8.5 < eleVertex[eZ] && eleVertex[eZ] < 4.5; // inbending; tighter than RGA common cuts, to remove foil from RGB data
  } else {
    vzBoolEle = -18 < eleVertex[eZ] && eleVertex[eZ] < 10; // outbending, RGA common cuts
  };

  // | had_Vz - ele_Vz | cut
  for(int h=0; h<2; h++) {
    vzdiff[h] = TMath::Abs(hadVertex[h][eZ]-eleVertex[eZ]);
  };
  vzdiffBool = vzdiff[qA] < 20 && vzdiff[qB] < 20;

  // full boolean:
  return vzBoolEle && vzdiffBool;

  // - DNP2019 cuts
  /*return eleVertex[eZ]     > -8  &&  eleVertex[eZ]     < 3  &&
         hadVertex[qA][eZ] > -8  &&  hadVertex[qA][eZ] < 3  &&
         hadVertex[qB][eZ] > -8  &&  hadVertex[qB][eZ] < 3;*/
};


// check missing mass cut (which can depend on channel)
Bool_t EventTree::CheckMissingMass() {
  if(Tools::PairSame(hadIdx[qA],hadIdx[qB],kPip,kPim))
    return Mmiss>1.5; // pi+,pi-
  else if(Tools::PairSame(hadIdx[qA],hadIdx[qB],kP,kPip))
    return Mmiss>0.6; // p,pi+
  else if(isDiphoton[qA] || isDiphoton[qB])
    return true; // diphoton,anything (valid only for anything==pi+/-)
  return Mmiss>1.5; // default
};



// sampling fraction (SF) cut, for electrons
Bool_t EventTree::CheckSampFrac() {

  // calorimeter diagonal cut, on PCAL and ECIN SF correlation
  if(eleP<4.5) sfcutDiag=true; // only applies above HTCC threshold
  else sfcutDiag = eleECINen/eleP > 0.2 - elePCALen/eleP; 

  // compute SF
  eleSampFrac = (elePCALen + eleECINen + eleECOUTen) / eleP;

  // need defined EC sector to apply SF cut
  if(eleSector>=1 && eleSector<=6) {

    // parameters for SF mu(p) and sigma(p) functions ///////   from F.X. and RGA common analysis note
    Double_t sfMu[3][6];
    Double_t sfSigma[3][6];

    if( (runnum>=5032 && runnum<=5666) || (runnum>=6616 && runnum<=6783) ) { // RGA
      sfMu[0][0]    = 0.2531;   sfMu[0][1]    = 0.2550;   sfMu[0][2]    = 0.2514;   sfMu[0][3]    = 0.2494;   sfMu[0][4]    = 0.2528;   sfMu[0][5]    = 0.2521;
      sfMu[1][0]    = -0.6502;  sfMu[1][1]    = -0.7472;  sfMu[1][2]    = -0.7674;  sfMu[1][3]    = -0.4913;  sfMu[1][4]    = -0.3988;  sfMu[1][5]    = -0.703;
      sfMu[2][0]    = 4.939;    sfMu[2][1]    = 5.350;    sfMu[2][2]    = 5.102;    sfMu[2][3]    = 6.440;    sfMu[2][4]    = 6.149;    sfMu[2][5]    = 4.957;
      sfSigma[0][0] = 0.002726; sfSigma[0][1] = 0.004157; sfSigma[0][2] = 0.005222; sfSigma[0][3] = 0.005398; sfSigma[0][4] = 0.008453; sfSigma[0][5] = 0.006553;
      sfSigma[1][0] = 1.062;    sfSigma[1][1] = 0.859;    sfSigma[1][2] = 0.5564;   sfSigma[1][3] = 0.6576;   sfSigma[1][4] = 0.3242;   sfSigma[1][5] = 0.4423;
      sfSigma[2][0] = -4.089;   sfSigma[2][1] = -3.318;   sfSigma[2][2] = -2.078;   sfSigma[2][3] = -2.565;   sfSigma[2][4] = -0.8223;  sfSigma[2][5] = -1.274;
    }
    else if( runnum>=6120 && runnum<=6604 ) { // RGB spring 2019
      // based on runs 6472, 6474, and 6479
      sfMu[0][0]    = 0.2520;   sfMu[0][1]    = 0.2520;  sfMu[0][2]    = 0.2479;   sfMu[0][3]    = 0.2444;   sfMu[0][4]    = 0.2463;   sfMu[0][5]    = 0.2478;
      sfMu[1][0]    = -0.8615;  sfMu[1][1]    = -0.8524; sfMu[1][2]    = -0.6848;  sfMu[1][3]    = -0.5521;  sfMu[1][4]    = -0.5775;  sfMu[1][5]    = -0.7327;
      sfMu[2][0]    = 5.596;    sfMu[2][1]    = 6.522;   sfMu[2][2]    = 5.752;    sfMu[2][3]    = 5.278;    sfMu[2][4]    = 6.430;    sfMu[2][5]    = 5.795;
      sfSigma[0][0] = -0.02963; sfSigma[0][1] = -0.1058; sfSigma[0][2] = -0.05087; sfSigma[0][3] = -0.04524; sfSigma[0][4] = -0.02951; sfSigma[0][5] = -0.01769;
      sfSigma[1][0] = 20.4;     sfSigma[1][1] = 129.3;   sfSigma[1][2] = 0.6191;   sfSigma[1][3] = 0.6817;   sfSigma[1][4] = 20.84;    sfSigma[1][5] = 8.44;
      sfSigma[2][0] = -41.44;   sfSigma[2][1] = -101.6;  sfSigma[2][2] = -2.673;   sfSigma[2][3] = -2.606;   sfSigma[2][4] = -42.67;   sfSigma[2][5] = -21.73;
    }
    else if( runnum>=11093 && runnum<=11300 ) { // RGB fall 2019
      fprintf(stderr,"ERROR: RGB fall2019 sampling fraction cuts not yet implemented; ask FX\n");
      return false;
    }
    else if( runnum>=11323 && runnum<=11571 ) { // RGB winter 2020
      // based on run 11375
      sfMu[0][0]    = 0.2427;  sfMu[0][1]    = 0.2417;  sfMu[0][2]    = 0.2409;  sfMu[0][3]    = 0.2482;  sfMu[0][4]    = 0.2414;  sfMu[0][5]    = 0.2450;
      sfMu[1][0]    = -0.7625; sfMu[1][1]    = -1.0297; sfMu[1][2]    = -1.1490; sfMu[1][3]    = -0.5120; sfMu[1][4]    = -0.6389; sfMu[1][5]    = -0.8967;
      sfMu[2][0]    = 5.2105;  sfMu[2][1]    = 4.4726;  sfMu[2][2]    = 4.4601;  sfMu[2][3]    = 5.9110;  sfMu[2][4]    = 5.5941;  sfMu[2][5]    = 4.8921;
      sfSigma[0][0] = 0.0116;  sfSigma[0][1] = 0.0159;  sfSigma[0][2] = 0.0119;  sfSigma[0][3] = 0.0103;  sfSigma[0][4] = 0.0141;  sfSigma[0][5] = 0.0083;
      sfSigma[1][0] = 0.1872;  sfSigma[1][1] = 0.1680;  sfSigma[1][2] = 0.1276;  sfSigma[1][3] = 0.1999;  sfSigma[1][4] = 0.0537;  sfSigma[1][5] = 0.2882;
      sfSigma[2][0] = -0.1221; sfSigma[2][1] = -0.0290; sfSigma[2][2] = 0.4179;  sfSigma[2][3] = 0.2938;  sfSigma[2][4] = 1.0519;  sfSigma[2][5] = -0.4405;
      // TODO: check BSA for these other parameterizations; radiation damage caused slow PMT gain drifts, hence the cuts drift slowly //////////////
      // // based on run 11442
      // sfMu[0][0]    = 0.2425;  sfMu[0][1]    = 0.2415;  sfMu[0][2]    = 0.2402;  sfMu[0][3]    = 0.2478;  sfMu[0][4]    = 0.2412;  sfMu[0][5]    = 0.2445;
      // sfMu[1][0]    = -0.7902; sfMu[1][1]    = -1.0531; sfMu[1][2]    = -1.1415; sfMu[1][3]    = -0.5304; sfMu[1][4]    = -0.6461; sfMu[1][5]    = -0.9213;
      // sfMu[2][0]    = 5.2645;  sfMu[2][1]    = 4.4854;  sfMu[2][2]    = 4.4976;  sfMu[2][3]    = 5.9428;  sfMu[2][4]    = 5.6900;  sfMu[2][5]    = 4.8948;
      // sfSigma[0][0] = 0.0116;  sfSigma[0][1] = 0.0155;  sfSigma[0][2] = 0.0115;  sfSigma[0][3] = 0.0104;  sfSigma[0][4] = 0.0145;  sfSigma[0][5] = 0.0085;
      // sfSigma[1][0] = 0.2024;  sfSigma[1][1] = 0.2095;  sfSigma[1][2] = 0.1564;  sfSigma[1][3] = 0.1989;  sfSigma[1][4] = 0.0470;  sfSigma[1][5] = 0.2886;
      // sfSigma[2][0] = -0.2808; sfSigma[2][1] = -0.3066; sfSigma[2][2] = 0.1420;  sfSigma[2][3] = 0.3124;  sfSigma[2][4] = 1.1334;  sfSigma[2][5] = -0.4543;
      // // based on run 11483
      // sfMu[0][0]    = 0.2433;  sfMu[0][1]    = 0.2421;  sfMu[0][2]    = 0.2415;  sfMu[0][3]    = 0.2486;  sfMu[0][4]    = 0.2419;  sfMu[0][5]    = 0.2447;
      // sfMu[1][0]    = -0.8052; sfMu[1][1]    = -1.0495; sfMu[1][2]    = -1.1747; sfMu[1][3]    = -0.5170; sfMu[1][4]    = -0.6840; sfMu[1][5]    = -0.9022;
      // sfMu[2][0]    = 5.2750;  sfMu[2][1]    = 4.4886;  sfMu[2][2]    = 4.4935;  sfMu[2][3]    = 5.9044;  sfMu[2][4]    = 5.6716;  sfMu[2][5]    = 4.9288;
      // sfSigma[0][0] = 0.0120;  sfSigma[0][1] = 0.0164;  sfSigma[0][2] = 0.0120;  sfSigma[0][3] = 0.0108;  sfSigma[0][4] = 0.0147;  sfSigma[0][5] = 0.0077;
      // sfSigma[1][0] = 0.1794;  sfSigma[1][1] = 0.1519;  sfSigma[1][2] = 0.1379;  sfSigma[1][3] = 0.1838;  sfSigma[1][4] = 0.0494;  sfSigma[1][5] = 0.3509;
      // sfSigma[2][0] = -0.0695; sfSigma[2][1] = 0.1553;  sfSigma[2][2] = 0.3300;  sfSigma[2][3] = 0.4330;  sfSigma[2][4] = 1.1032;  sfSigma[2][5] = -0.7996;
      // // based on run 11518
      // sfMu[0][0]    = 0.2417;  sfMu[0][1]    = 0.2409;  sfMu[0][2]    = 0.2393;  sfMu[0][3]    = 0.2465;  sfMu[0][4]    = 0.2407;  sfMu[0][5]    = 0.2436;
      // sfMu[1][0]    = -0.7828; sfMu[1][1]    = -1.0793; sfMu[1][2]    = -1.1627; sfMu[1][3]    = -0.4932; sfMu[1][4]    = -0.6576; sfMu[1][5]    = -0.9408;
      // sfMu[2][0]    = 5.3301;  sfMu[2][1]    = 4.4935;  sfMu[2][2]    = 4.5123;  sfMu[2][3]    = 6.1411;  sfMu[2][4]    = 5.7539;  sfMu[2][5]    = 4.9382;
      // sfSigma[0][0] = 0.0121;  sfSigma[0][1] = 0.0149;  sfSigma[0][2] = 0.0116;  sfSigma[0][3] = 0.0109;  sfSigma[0][4] = 0.0148;  sfSigma[0][5] = 0.0081;
      // sfSigma[1][0] = 0.1727;  sfSigma[1][1] = 0.2762;  sfSigma[1][2] = 0.1618;  sfSigma[1][3] = 0.1730;  sfSigma[1][4] = 0.0460;  sfSigma[1][5] = 0.3260;
      // sfSigma[2][0] = -0.0368; sfSigma[2][1] = -0.8267; sfSigma[2][2] = 0.1066;  sfSigma[2][3] = 0.4624;  sfSigma[2][4] = 1.1389;  sfSigma[2][5] = -0.6666;
      // // based on run 11563
      // sfMu[0][0]    = 0.2416;  sfMu[0][1]    = 0.2406;  sfMu[0][2]    = 0.2393;  sfMu[0][3]    = 0.2464;  sfMu[0][4]    = 0.2404;  sfMu[0][5]    = 0.2434;
      // sfMu[1][0]    = -0.7865; sfMu[1][1]    = -1.0576; sfMu[1][2]    = -1.1872; sfMu[1][3]    = -0.5078; sfMu[1][4]    = -0.6703; sfMu[1][5]    = -0.9305;
      // sfMu[2][0]    = 5.3506;  sfMu[2][1]    = 4.5243;  sfMu[2][2]    = 4.5071;  sfMu[2][3]    = 6.0961;  sfMu[2][4]    = 5.7028;  sfMu[2][5]    = 4.9425;
      // sfSigma[0][0] = 0.0120;  sfSigma[0][1] = 0.0159;  sfSigma[0][2] = 0.0119;  sfSigma[0][3] = 0.0109;  sfSigma[0][4] = 0.0149;  sfSigma[0][5] = 0.0086;
      // sfSigma[1][0] = 0.1752;  sfSigma[1][1] = 0.1975;  sfSigma[1][2] = 0.1458;  sfSigma[1][3] = 0.1787;  sfSigma[1][4] = 0.0414;  sfSigma[1][5] = 0.2873;
      // sfSigma[2][0] = -0.0405; sfSigma[2][1] = -0.2537; sfSigma[2][2] = 0.2474;  sfSigma[2][3] = 0.4116;  sfSigma[2][4] = 1.2172;  sfSigma[2][5] = -0.4651;
    }
    else if(runnum==11) { // MC
      for(int s=0; s<6; s++) {
        sfMu[0][s] = 0.248605;
        sfMu[1][s] = -0.844221;
        sfMu[2][s] = 4.87777;
        sfSigma[0][s] = 0.00741575;
        sfSigma[1][s] = 0.215861;
        sfSigma[2][s] = -0.319801;
      };
    }
    else {
      fprintf(stderr,"ERROR: unknown run for sampling fraction band cut\n");
      return false;
    };

    // calculate mu(p) and sigma(p), where p=`eleP`
    Double_t mu    = sfMu[0][eleSector-1] + (sfMu[1][eleSector-1]/1000) * TMath::Power(eleP-sfMu[2][eleSector-1],2);
    Double_t sigma = sfSigma[0][eleSector-1] + sfSigma[1][eleSector-1] / (10 * (eleP-sfSigma[2][eleSector-1]));

    // SF must be within 3.5 sigma of mean; here SF is from PCAL+ECAL+ECIN/eleP
    sfcutSigma = TMath::Abs(eleSampFrac-mu) < 3.5*sigma;

  } else {
    sfcutSigma = false;
  };

  // return full SF cut result
  return sfcutDiag && sfcutSigma;
};


// PID refinement cut for pions; the upper bound cut at high momentum
// helps reduce kaon contamination
Bool_t EventTree::CheckHadChi2pid(Int_t had) {

  // this cut should only apply to charged hadrons; bypass this
  // cut if it's a diphoton
  if(isDiphoton[had]) return true;

  // corrected stddev of chi2pid
  Float_t sigma;
  switch(hadIdx[had]) {
    case kPip: sigma=0.88; break;
    case kPim: sigma=0.93; break;
    default: sigma=1.0;
  };

  // lower bound cut
  if(hadChi2pid[had]<=-3*sigma) return false;
  
  // upper bound cut
  ///* -- standard cut
  if(hadP[had]<2.44) return hadChi2pid[had] < 3*sigma; // 3-sigma cut at low p
  else {
    // 1/2 distance cut at higher p (1/2 distance in beta between kaons and pions)
    return hadChi2pid[had] < sigma * (0.00869 + 
      14.98587 * TMath::Exp(-hadP[had]/1.18236) + 1.81751 * exp(-hadP[had]/4.86394));
  };
  //*/
  /* -- strict cut
  if(hadP[had]<2.44) return hadChi2pid[had] < 3*sigma; // 3-sigma cut at low p
  else if(2.44<=hadP[had] && hadP[had]<4.6) {
    // 1/2 distance cut at mid p
    return hadChi2pid[had] < sigma * (0.00869 +
      14.98587 * TMath::Exp(-hadP[had]/1.18236) + 1.81751 * exp(-hadP[had]/4.86394));
  } else {
    // 1-sigma in hadChi2pid[had] for high p
    return hadChi2pid[had] < sigma * (-1.14099 + 
      24.14992 * TMath::Exp(-hadP[had]/1.36554) + 2.66876 * exp(-hadP[had]/6.80552));
  };
  */

  fprintf(stderr,"ERROR: unknown upper bound for hadron chi2pid cut\n");
  return false;
};


void EventTree::PrintEventVerbose() {
  printf("[---] Event Info\n");
  printf("  evnum=%d",evnum);
  printf("  runnum=%d",runnum);
  printf("  pairType=0x%x",pairType);
  printf("\n");
  printf("  helicity=%d",helicity);
  printf("\n");
  printf("[---] DIS Kinematics\n");
  printf("  x=%.2f",x);
  printf("  Q2=%.2f",Q2);
  printf("  W=%.2f",W);
  printf("\n");
  printf("  Nu=%.2f",Nu);
  printf("  y=%.2f",y);
  printf("\n");
  printf("[---] Hadron Kinematics: %s\n",PairName(hadIdx[qA],hadIdx[qB]).Data());
  for(int h=0; h<2; h++) {
    printf(" (%s)\n",PartName(dihHadIdx(hadIdx[qA],hadIdx[qB],h)).Data());
    printf("  E=%.2f",hadE[h]);
    printf("  P=%.2f",hadP[h]);
    printf("  Pt=%.2f",hadPt[h]);
    printf("\n");
    printf("  Eta=%.2f",hadEta[h]);
    printf("  Phi=%.2f",hadPhi[h]);
    printf("\n");
  };
  printf("[---] Dihadron Kinematics\n");
  printf("  Mh=%.2f",Mh);
  printf("  Mmiss=%.2f",Mmiss);
  printf("  xF=%.2f",xF);
  printf("  alpha=%.2f\n",alpha);
  printf("  Zpair=%.2f  Z(a)=%.2f  Z(b)=%.2f\n",Zpair,Z[qA],Z[qB]);
  printf("  PhiH=%.2f\n",PhiH);
  printf("[---] PhiR Tests\n");
  printf("  PhiRq=%.2f",PhiRq);
  printf("  PhiRp=%.2f",PhiRp);
  printf("  PhiRp_r=%.2f",PhiRp_r);
  printf("\n");
};

void EventTree::PrintEvent() {
  printf("\n");
  printf("  evnum = %d\n",evnum);
  for(int h=0; h<2; h++) {
    printf("  %s p = %.5f\n",
      PartName(dihHadIdx(hadIdx[qA],hadIdx[qB],h)).Data(),hadP[h]);
  };
  printf("  eleP = %.5f\n",eleP);
  printf("  Q2 = %.5f\n",Q2);
  printf("  W = %.5f\n",W);
  printf("  x = %.5f\n",x);
  printf("  y = %.5f\n",y);
  printf("  Mh = %.5f\n",Mh);
  printf("  xF = %.5f\n",xF);
  printf("  PhPerp = %.5f\n",PhPerp);
  printf("  theta = %.5f\n",theta);
  printf("  PhiH = %.5f\n",PhiH);
  printf("  PhiR = %.5f\n",PhiR);
  printf("\n");
};

void EventTree::PrintEventLine() {
  printf("%d",evnum);
  for(int h=0; h<2; h++) {
    printf(" %.5f",hadP[h]);
    printf(" %.5f",hadPt[h]);
    printf(" %.5f",hadEta[h]);
    printf(" %.5f",hadPhi[h]);
  };
  printf(" %.5f",eleP);
  printf(" %.5f",elePt);
  printf(" %.5f",eleEta);
  printf(" %.5f",elePhi);
  printf(" %.5f",Q2);
  printf(" %.5f",W);
  printf(" %.5f",x);
  printf(" %.5f",y);
  printf(" %.5f",Mh);
  printf(" %.5f",xF);
  printf(" %.5f",PhPerp);
  printf(" %.5f",theta);
  printf(" %.5f",PhiH);
  printf(" %.5f",PhiR);
  printf(" %.5f",eleChi2pid);
  for(int h=0; h<2; h++) printf(" %.5f",hadChi2pid[h]);
  printf("\n");
};





// use hadron kinematics to reconstruct Dihadron object
Dihadron * EventTree::GetDihadronObj() {
  objDihadron->ResetVars();
  for(int h=0; h<2; h++) {
    hadMom[h].SetPtEtaPhiE(hadPt[h],hadEta[h],hadPhi[h],hadE[h]);
    trHad[h]->Momentum = hadMom[h];
    trHad[h]->Status = hadStatus[h];
    trHad[h]->chi2pid = hadChi2pid[h];
    trHad[h]->Vertex.SetXYZ(hadVertex[h][eX],hadVertex[h][eY],hadVertex[h][eZ]);
    trHad[h]->Idx = hadIdx[h];
  };
  this->GetDISObj();
  objDihadron->CalculateKinematics(trHad[qA],trHad[qB],objDIS);
  return objDihadron;
};

// use electron kinematics to reconstruct DIS object
DIS * EventTree::GetDISObj() {
  objDIS->ResetVars();

  eleMom.SetPtEtaPhiE(elePt,eleEta,elePhi,eleE);
  trEle->Momentum = eleMom;
  trEle->Status = eleStatus;
  trEle->chi2pid = eleChi2pid;
  trEle->Vertex.SetXYZ(eleVertex[eX],eleVertex[eY],eleVertex[eZ]);

  objDIS->CalculateKinematics(trEle,runnum);
  return objDIS;
};




// get depolarization factor
// - approximations which depend only on y -- do not use!
/*
Float_t EventTree::GetDepolarizationFactorApprox(Char_t kf) {
  // source: arXiv:1408.5721
  if(kf=='A')      return 1 - y + y*y/2.0;
  else if(kf=='B') return 1 - y;
  else if(kf=='C') return y * (1-y/2.0);
  else if(kf=='V') return (2-y) * TMath::Sqrt(1-y);
  else if(kf=='W') return y * TMath::Sqrt(1-y);
  else {
    fprintf(stderr,"ERROR: unknown depolarization factor %c; returning 0\n",kf);
    return 0;
  };
};
*/
// - exact expressions which depend on epsilon and y
Float_t EventTree::GetDepolarizationFactor(Char_t kf) {
  // source: arXiv:1408.5721

  dfA = y*y / (2 - 2*epsilon); // A(x,y)

  if(kf=='A')      return dfA;
  else if(kf=='B') return dfA * epsilon;
  else if(kf=='C') return dfA * TMath::Sqrt(1-epsilon*epsilon);
  else if(kf=='V') return dfA * TMath::Sqrt(2*epsilon*(1+epsilon));
  else if(kf=='W') return dfA * TMath::Sqrt(2*epsilon*(1-epsilon));
  else {
    fprintf(stderr,"ERROR: unknown depolarization factor %c; returning 0\n",kf);
    return 0;
  };
};
// - get depolarization factor ratio, which depends on twist (for LU polarization only!!!)
Float_t EventTree::GetDepolarizationRatio(Int_t twist) {
  switch(twist) {
    case 2: return GetDepolarizationFactor('C') / GetDepolarizationFactor('A'); break;
    case 3: return GetDepolarizationFactor('W') / GetDepolarizationFactor('A'); break;
    default:
            fprintf(stderr,"ERROR: unknown twist %d for depolarization factor ratio; return 0\n",twist);
            return 0;
  }
}

// general method to calculate rapidity
// - calculate the rapidity of `momentumVec`, where `momentumVec` is boosted by
//   `boostVec`
// - `zAxis` specifies the light cone coordinates orientation (and is not
//   boosted by `boostVec`), i.e., the direction taken as `p_3` or `p_z`
// - outputs `rapidity` and `p_z`
void EventTree::CalculateRapidity(
  TLorentzVector momentumVec_,
  TVector3 boostVec,
  TVector3 zAxis,
  Float_t &rapidity,
  Float_t &p_z
) {
  TLorentzVector momentumVec = momentumVec_;
  momentumVec.Boost(boostVec);
  Float_t znorm = zAxis.Mag();
  p_z = znorm*znorm>0 ? 
    momentumVec.Vect().Dot(zAxis) / znorm : 0;
  rapidity = 0.5 * TMath::Log(
    (momentumVec.E() + p_z) / (momentumVec.E() - p_z)
  );
};


EventTree::~EventTree() {
};


