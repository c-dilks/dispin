#include "EventTree.h"

ClassImp(EventTree)

using namespace std;


EventTree::EventTree(TString filelist, Int_t whichPair_) {
  printf("EventTree instantiated\n");

  debug = true;

  useDiphBG = false;

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
  chain->SetBranchAddress("eleFidPCAL",eleFidPCAL);
  chain->SetBranchAddress("eleFidDC",eleFidDC);


  chain->SetBranchAddress("pairType",&pairType);
  chain->SetBranchAddress("hadOrder",&hadOrder);
  chain->SetBranchAddress("hadIdx",hadIdx);
  chain->SetBranchAddress("hadE",hadE);
  chain->SetBranchAddress("hadP",hadP);
  chain->SetBranchAddress("hadPt",hadPt);
  chain->SetBranchAddress("hadEta",hadEta);
  chain->SetBranchAddress("hadPhi",hadPhi);
  chain->SetBranchAddress("hadVertex",hadVertex);
  chain->SetBranchAddress("hadStatus",hadStatus);
  chain->SetBranchAddress("hadChi2pid",hadChi2pid);
  /*
  if(chain->GetBranch("hadFidPCAL")) chain->SetBranchAddress("hadFidPCAL",hadFidPCAL);
  else { for(int h=0; h<2; h++) hadFidPCAL[h]=false; };
  if(chain->GetBranch("hadFidDC")) chain->SetBranchAddress("hadFidDC",hadFidDC);
  else { for(int h=0; h<2; h++) hadFidDC[h]=false; };
  */

  // these branches were temporarily used for early crosscheck studies
  if(chain->GetBranch("hadPtq")) chain->SetBranchAddress("hadPtq",hadPtq);
  else { for(int h=0; h<2; h++) hadPtq[h]=UNDEF; };
  if(chain->GetBranch("hadXF")) chain->SetBranchAddress("hadXF",hadXF);
  else { for(int h=0; h<2; h++) hadXF[h]=UNDEF; };
  /////////////

  chain->SetBranchAddress("particleCnt",particleCnt);
  //chain->SetBranchAddress("particleCntAll",&particleCntAll);

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

  chain->SetBranchAddress("runnum",&runnum);
  chain->SetBranchAddress("evnum",&evnum);
  chain->SetBranchAddress("torus",&torus);
  chain->SetBranchAddress("triggerBits",&triggerBits);
  chain->SetBranchAddress("helicity",&helicity);

  if(chain->GetBranch("helicityMC")) chain->SetBranchAddress("helicityMC",helicityMC);
  else { for(int hh=0; hh<NhelicityMC; hh++) helicityMC[hh]=UNDEF; };
  if(chain->GetBranch("matchDiff")) {
    MCrecMode = true;
    chain->SetBranchAddress("matchDiff",&matchDiff);
    chain->SetBranchAddress("gen_eleE",&gen_eleE);
    chain->SetBranchAddress("gen_elePt",&gen_elePt);
    chain->SetBranchAddress("gen_eleEta",&gen_eleEta);
    chain->SetBranchAddress("gen_elePhi",&gen_elePhi);
    chain->SetBranchAddress("gen_hadE",gen_hadE);
    chain->SetBranchAddress("gen_hadPt",gen_hadPt);
    chain->SetBranchAddress("gen_hadEta",gen_hadEta);
    chain->SetBranchAddress("gen_hadPhi",gen_hadPhi);
  } else { 
    MCrecMode = false;
    matchDiff=UNDEF; 
    gen_eleE = UNDEF;
    gen_elePt = UNDEF;
    gen_eleEta = UNDEF;
    gen_elePhi = UNDEF;
    for(int h=0; h<2; h++) {
      gen_hadE[h] = UNDEF;
      gen_hadPt[h] = UNDEF;
      gen_hadEta[h] = UNDEF;
      gen_hadPhi[h] = UNDEF;
    };
  };

  chain->SetBranchAddress("diphCnt",&diphCnt);
  chain->SetBranchAddress("diphPhotE",diphPhotE);
  chain->SetBranchAddress("diphPhotPt",diphPhotPt);
  chain->SetBranchAddress("diphPhotEta",diphPhotEta);
  chain->SetBranchAddress("diphPhotPhi",diphPhotPhi);
  chain->SetBranchAddress("diphE",diphE);
  chain->SetBranchAddress("diphZ",diphZ);
  chain->SetBranchAddress("diphPt",diphPt);
  chain->SetBranchAddress("diphM",diphM);
  chain->SetBranchAddress("diphAlpha",diphAlpha);
  chain->SetBranchAddress("diphEta",diphEta);
  chain->SetBranchAddress("diphPhi",diphPhi);

  // random number generator (for random theta symmetrization)
  RNG = new TRandom(928); // (argument is seed)

  // instantiate useful objects
  objDihadron = new Dihadron();
  candDih= new Dihadron();
  objDIS = new DIS();
  trEle = new Trajectory(kE);
  for(int h=0; h<2; h++) {
    trHad[h] = new Trajectory(dihHadIdx(whichHad[qA],whichHad[qB],h));
  };
  whichHelicityMC = 0;
};


void EventTree::GetEvent(Int_t i) {
  if(i%10000==0) printf("[+] %.2f%%\n",100*(float)i/((float)ENT));

  chain->GetEntry(i);

  /* 
   * // CODE TO BE USED FOR MC GEN READING
   *
  // calculate DIS and Dihadron kinematics (GetDihadronObj calls GetDISObj,
  // so that both `objDihadron` and `objDIS` will contain all the kinematics we want)
  this->GetDihadronObj();

  // set kinematic variables
  // -- DIS kinematics
  W = objDIS->W;
  Q2 = objDIS->Q2;
  Nu = objDIS->Nu;
  x = objDIS->x;
  y = objDIS->y;
  // -- dihadron kinematics
  Mh = objDihadron->Mh;
  Mmiss = objDihadron->Mmiss;
  for(int h=0; h<2; h++) Z[h] = objDihadron->z[h];
  Zpair = objDihadron->zpair;
  zeta = objDihadron->zeta;
  xF = objDihadron->xF;
  alpha = objDihadron->alpha;
  theta = objDihadron->theta;
  Ph = objDihadron->PhMag;
  PhPerp = objDihadron->PhPerpMag;
  PhEta = objDihadron->PhEta;
  PhPhi = objDihadron->PhPhi;
  R = objDihadron->RMag;
  RPerp = objDihadron->RPerpMag;
  RT = objDihadron->RTMag;
  PhiH = objDihadron->PhiH;
  // -- phiR angles
  PhiRq = objDihadron->PhiRq; // via R_perp
  PhiRp = objDihadron->PhiRp; // via R_T
  PhiRp_r = objDihadron->PhiRp_r; // via R_T (frame-dependent)
  PhiRp_g = objDihadron->PhiRp_g; // via eq. 9 in 1408.5721
  */

  // set preferred PhiR angle
  PhiR = PhiRp; // preferred definition by Bacchetta (see Dihadron.cxx)
  PhiHR = Tools::AdjAngle( PhiH - PhiR );

  // adjust range to 0-2pi (for cross-checking with Timothy)
  /*
  PhiR = Tools::AdjAngleTwoPi(PhiR);
  PhiH = Tools::AdjAngleTwoPi(PhiH);
  PhiHR = Tools::AdjAngleTwoPi(PhiHR);
  */

  // theta symmetrization tests
  //theta = fabs( fabs(theta-PI/2.0) - PI/2.0 ); // HERMES symmetrization
  //if(Z[qB] > Z[qA]) theta = PI - theta; // Z symmetrization
  //if( RNG->Rndm() > 0.5 ) theta = PI - theta; // coin-flip symmetrization
  //theta = PI-theta; // full theta flip

  // convert eta to polar angle theta (not to be confused with partial wave theta)
  for(int h=0; h<2; h++) {
    hadTheta[h] = Tools::EtaToTheta(hadEta[h]);
    gen_hadTheta[h] = MCrecMode ? Tools::EtaToTheta(gen_hadEta[h]) : UNDEF;
  };

  // DIS cuts
  cutQ2 = Q2 > 1.0;
  cutW = W > 2.0;
  cutY = y < 0.8;
  cutDIS = cutQ2 && cutW && cutY;



  // diphoton and pi0/BG cuts
  // -- if dihadron does not include pi0s, just set to true; first we set
  //    the booleans' default values to true
  for(int h=0; h<2; h++) {
    cutDiphKinematics[h] = true;
    cutDiph[h] = true;
    for(int p=0; p<2; p++) angEle[h][p] = UNDEF;
  };
  // -- then if there are diphotons in this dihadron we evaluate their cuts
  if(diphCnt>0) {
    eleMom.SetPtEtaPhiE(elePt,eleEta,elePhi,eleE);
    for(int h=0; h<diphCnt; h++) {

      // compute angle of photons wrt electron
      for(int p=0; p<2; p++) {
        photMom[p].SetPtEtaPhiE(
          diphPhotPt[h][p], diphPhotEta[h][p],
          diphPhotPhi[h][p], diphPhotE[h][p] );
        angEle[h][p] = Tools::AngleSubtend( photMom[p].Vect(), eleMom.Vect() );
        angEle[h][p] *= TMath::RadToDeg();
      };

      // diphoton kinematics cut
      cutDiphKinematics[h] = angEle[h][0]>8 && angEle[h][1]>8 &&
                             diphPhotE[h][0]>0.6 && diphPhotE[h][1]>0.6;
                             //Tools::PhiFiducialCut(diphPhotPhi[h][0]) && 
                             //Tools::PhiFiducialCut(diphPhotPhi[h][1]);

      // mass cut (depends on whether pi0 signal or BG is desired
      if(useDiphBG) {
        // BG cut
        cutDiph[h] = cutDiphKinematics[h] && diphM[h] < 0.1 || diphM[h] > 0.16;
      } else {
        // pi0 cut
        cutDiph[h] = cutDiphKinematics[h] && diphM[h] >= 0.1 && diphM[h] <= 0.16;
      };
    };
  };



  // dihadron kinematics cuts
  cutDihadronKinematics = 
    Zpair < 0.95 &&
    Mmiss > 1.5 &&
    hadXF[qA] > 0 && hadXF[qB] > 0 &&
    hadP[qA] > 1.25 && hadP[qB] > 1.25;

  // cutDihadron is the full dihadron cut
  cutDihadron = 
    Tools::PairSame(hadIdx[qA],hadIdx[qB],whichHad[qA],whichHad[qB]) &&
    cutDihadronKinematics && 
    cutDiph[qA] && cutDiph[qB];

  
  //// cut for doing cross-checks - deprecated
  //// -- tim's cuts
  //cutCrossCheck = 
    //Tools::PairSame(hadIdx[qA],hadIdx[qB],kPip,kPim) &&
    ///*
    //particleCnt[kPip]==1 && particleCnt[kPim]==1 &&
    //Tools::EMtoP(hadE[qA],PartMass(kPip)) > 1.0 &&
    //Tools::EMtoP(hadE[qB],PartMass(kPim)) > 1.0 &&
    //Tools::EMtoP(eleE,PartMass(kE)) > 2.0 &&
    //*/
    //Q2>1 && W>2;


  // vertex cuts
  cutVertex = eleVertex[eZ]     > -8  &&  eleVertex[eZ]     < 3  &&
              hadVertex[qA][eZ] > -8  &&  hadVertex[qA][eZ] < 3  &&
              hadVertex[qB][eZ] > -8  &&  hadVertex[qB][eZ] < 3;
  if(diphCnt>0) { // diphotons don't yet have a vertex! TODO
    if(hadIdx[qA]==kDiph && hadIdx[qB]==kDiph) 
      cutVertex = eleVertex[eZ] > -8  &&  eleVertex[eZ] < 3;
    else if(hadIdx[qA]==kDiph) 
      cutVertex = eleVertex[eZ]     > -8  &&  eleVertex[eZ]     < 3  &&
                  hadVertex[qB][eZ] > -8  &&  hadVertex[qB][eZ] < 3;
    else if(hadIdx[qB]==kDiph) 
      cutVertex = eleVertex[eZ]     > -8  &&  eleVertex[eZ]     < 3  &&
                  hadVertex[qA][eZ] > -8  &&  hadVertex[qA][eZ] < 3;
  };


  // fiducial cuts
  whichLevel = FiducialCuts::cutLoose;
  cutFiducial = eleFidPCAL[whichLevel] && eleFidDC[whichLevel];
  //cutFiducial = true; // override
  
  // require hadrons observed in forward detectors
  cutDihadronStatus = 
    ( TMath::Abs(hadStatus[qA])<4000 || TMath::Abs(hadStatus[qA])>=5000 ) &&
    ( TMath::Abs(hadStatus[qB])<4000 || TMath::Abs(hadStatus[qB])>=5000 );


  // MCgen and MCrec matching cut
  cutMCmatch = MCrecMode && matchDiff>=0 && matchDiff<0.02;

};


/////////////////////////////////////////////////////////
// MAIN ANALYSIS CUT
Bool_t EventTree::Valid() {
  return cutDIS && cutDihadron &&
         /*cutVertex && cutFiducial &&*/ cutDihadronStatus;
};
/////////////////////////////////////////////////////////


Int_t EventTree::SpinState() {
  if(runnum>=4700 && runnum<=6000) {
    // Fall 2018 convention
    switch(helicity) {
      case 1: return sM;
      case -1: return sP;
      case 0: return UNDEF;
      default: fprintf(stderr,"WARNING: bad SpinState request: %d\n",helicity);
    };
  }
  else if(runnum>=4000 && runnum<=4100) {
    // Spring 2018 (for DNP18) convention
    switch(helicity) {
      case 0: return sP;
      case 1: return sM;
      default: fprintf(stderr,"WARNING: bad SpinState request: %d\n",helicity);
    };
  }

  else if(runnum==11) { // MC helicity

    // event matching cut
    if(MCrecMode && !cutMCmatch) return UNDEF;

    // MC convention (from injected asymmetries)
    helicity = helicityMC[whichHelicityMC];
    switch(helicity) {
      case 2: return sM;
      case 3: return sP;
      case 0: return UNDEF;
      default: fprintf(stderr,"WARNING: bad SpinState request: %d\n",helicity);
    };
  }
  else fprintf(stderr,"WARNING: runnum %d not in EventTree::SpinState\n",runnum);
  return UNDEF;
};


void EventTree::PrintEventVerbose() {
  printf("[---] Event Info\n");
  printf("  evnum=%d",evnum);
  printf("  runnum=%d",runnum);
  printf("  pairType=0x%x",pairType);
  printf("\n");
  printf("  helicity=%d",helicity);
  printf("  torus=%.1f",torus);
  printf("\n");
  printf("  triggerBits=%lld",triggerBits);
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
  printf(" %d",hadOrder);
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
    trHad[h]->SetVec(hadMom[h]);
    trHad[h]->Status = hadStatus[h];
    trHad[h]->chi2pid = hadChi2pid[h];
    trHad[h]->SetVertex(hadVertex[h][eX],hadVertex[h][eY],hadVertex[h][eZ]);
  };
  this->GetDISObj();
  objDihadron->SetEvent(trHad[qA],trHad[qB],objDIS);
  return objDihadron;
};

// use electron kinematics to reconstruct DIS object
DIS * EventTree::GetDISObj() {
  objDIS->ResetVars();

  eleMom.SetPtEtaPhiE(elePt,eleEta,elePhi,eleE);
  trEle->SetVec(eleMom);
  trEle->Status = eleStatus;
  trEle->chi2pid = eleChi2pid;
  trEle->SetVertex(eleVertex[eX],eleVertex[eY],eleVertex[eZ]);

  objDIS->SetElectron(trEle);
  objDIS->Analyse();
  return objDIS;
};



// get y-dependent kinematic factor
Float_t EventTree::GetKinematicFactor(Char_t kf) {
  // source: arXiv:1408.5721
  if(kf=='A')      return 1 - y + y*y/2.0;
  else if(kf=='B') return 1 - y;
  else if(kf=='C') return y * (1-y/2.0);
  else if(kf=='V') return (2-y) * TMath::Sqrt(1-y);
  else if(kf=='W') return y * TMath::Sqrt(1-y);
  else {
    fprintf(stderr,"ERROR: unknown kinematic factor %c; returning 0\n",kf);
    return 0;
  };
};


/*
// build map `evnumMap : evnum -> vector of corresponding tree entries`
// -- return true if successful
// -- deprecated, but still here for if you want to test MC matching ideas
Bool_t EventTree::BuildMatchTable() {
  printf("building EventTree event matching table...\n");

  for(int i=0; i<ENT; i++) {

    chain->GetEntry(i);

    if(i==0) {
      evnumTmp = evnum;
      iList.clear();
    };

    if(evnum!=evnumTmp || i+1==ENT) {
      inserted = 
        evnumMap.insert(std::pair<Int_t,std::vector<Int_t>>(evnumTmp,iList)).second;
      if(!inserted) {
        fprintf(stderr,"ERROR: BuildMatchTable failed\n");
        return false;
      };
      iList.clear();
      evnumTmp = evnum;
    };

    if(Tools::PairSame(hadIdx[qA],hadIdx[qB],whichHad[qA],whichHad[qB])) {
      iList.push_back(i);
    };
  };
  return true;
};


// DIHADRON EVENT MATCHING ALGORITHM
// ---------------------------------
// find event in `this` which matches the event in `queryEv`
// - this->BuildMatchTable MUST be (successfully) called before using this
// - loops through vector of dihadrons, and looks for matches based on how close the
//   hadrons' kinematics are 
// - returns true if a match is found
// - deprecated, but still here for if you want to test MC matching ideas
Bool_t EventTree::FindEvent(Int_t evnum_, Dihadron * queryDih) {

  // reset vars
  MDmin = 1e6;
  MD = 1e6;
  iiFound = -1;

  // find the event
  auto evnumMapIT = evnumMap.find(evnum_); 
  if(evnumMapIT!=evnumMap.end()) {

    // loop through vector of dihadron tree indices within the found event
    for(auto ii : evnumMapIT->second) {
      this->GetEvent(ii);

      // get candidate match kinematics
      candDih = this->GetDihadronObj();
      for(int h=0; h<2; h++) {
        queryTheta[h] = (queryDih->vecHad[h]).Theta();
        candTheta[h] = (candDih->vecHad[h]).Theta();
        queryPhi[h] = (queryDih->vecHad[h]).Phi();
        candPhi[h] = (candDih->vecHad[h]).Phi();
      };

      // calculate euclidean distance between query and candidate dihadrons' kinematics
      MD = TMath::Sqrt(
        TMath::Power( Tools::AdjAngle(queryTheta[qA] - candTheta[qA]), 2) +
        TMath::Power( Tools::AdjAngle(queryTheta[qB] - candTheta[qB]), 2) +
        TMath::Power( Tools::AdjAngle(queryPhi[qA] - candPhi[qA]), 2) +
        TMath::Power( Tools::AdjAngle(queryPhi[qB] - candPhi[qB]), 2) );

      // check if this is the smallest MD
      if(MD < MDmin) {
        MDmin = MD;
        iiFound = ii;
      };
    };
  };

  // if a match is found, set branch variables to the matching dihadron;
  // if not, set MD to a high value and set hadron kinematics to UNDEF
  if(iiFound>=0) {
    this->GetEvent(iiFound);
    MD = MDmin;
    return true;
  }
  else {
    MD = 1000;
    for(int h=0; h<2; h++) {
      hadE[h] = UNDEF;
      hadPt[h] = UNDEF;
      hadEta[h] = UNDEF;
      hadPhi[h] = UNDEF;
    };
    return false;
  };
  
};
*/


EventTree::~EventTree() {
};


