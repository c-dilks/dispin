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
  chain->SetBranchAddress("eleSampFrac",&eleSampFrac);
  chain->SetBranchAddress("elePCALen",&elePCALen);

  chain->SetBranchAddress("eleFiduCut",eleFiduCut);
  chain->SetBranchAddress("hadFiduCut",hadFiduCut);

  chain->SetBranchAddress("pairType",&pairType);
  chain->SetBranchAddress("hadIdx",hadIdx);
  chain->SetBranchAddress("hadE",hadE);
  chain->SetBranchAddress("hadP",hadP);
  chain->SetBranchAddress("hadPt",hadPt);
  chain->SetBranchAddress("hadEta",hadEta);
  chain->SetBranchAddress("hadPhi",hadPhi);
  chain->SetBranchAddress("hadVertex",hadVertex);
  chain->SetBranchAddress("hadStatus",hadStatus);
  chain->SetBranchAddress("hadChi2pid",hadChi2pid);

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

  chain->SetBranchAddress("runnum",&runnum);
  chain->SetBranchAddress("evnum",&evnum);
  chain->SetBranchAddress("helicity",&helicity);

  // MC branches
  if(chain->GetBranch("gen_hadMatchDist")) {
    MCrecMode = true;
    chain->SetBranchAddress("helicityMC",helicityMC);
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
  } else { 
    MCrecMode = false;
    for(int hh=0; hh<NhelicityMC; hh++) helicityMC[hh]=0; 
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
    };
  };

  // instantiate useful objects
  objDihadron = new Dihadron();
  candDih= new Dihadron();
  objDIS = new DIS();
  trEle = new Trajectory();
  trEle->Idx = kE;
  for(int h=0; h<2; h++) {
    trHad[h] = new Trajectory();
    trHad[h]->Idx = dihHadIdx(whichHad[qA],whichHad[qB],h);
  };
  whichHelicityMC = 0;
  vertexWarned = false;

};


void EventTree::GetEvent(Int_t i) {
  if(i%10000==0) printf("[+] %.2f%%\n",100*(float)i/((float)ENT));

  chain->GetEntry(i);

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
  //theta = PI-theta; // full theta flip

  // convert eta to polar angle theta 
  // (not to be confused with partial wave theta)
  eleTheta = Tools::EtaToTheta(eleEta);
  gen_eleTheta = MCrecMode ? Tools::EtaToTheta(gen_eleEta) : UNDEF;
  for(int h=0; h<2; h++) {
    hadTheta[h] = Tools::EtaToTheta(hadEta[h]);
    gen_hadTheta[h] = MCrecMode ? Tools::EtaToTheta(gen_hadEta[h]) : UNDEF;
  };

  // DIS cuts
  cutQ2 = Q2 > 1.0;
  cutW = W > 2.0;
  cutY = y < 0.8;
  cutDIS = cutQ2 && cutW && cutY;

  // dihadron cuts
  cutDihadron = 
    Tools::PairSame(hadIdx[qA],hadIdx[qB],whichHad[qA],whichHad[qB]) &&
    Zpair < 0.95 &&
    Mmiss > 1.5 &&
    hadXF[qA] > 0 && hadXF[qB] > 0 &&
    hadP[qA] > 1.25 && hadP[qB] > 1.25;

  // vertex cuts
  cutVertex = CheckVertex();

  // fiducial cuts
  whichLevel = FiducialCuts::cutLoose;
  cutFiducial = eleFiduCut[whichLevel] && 
                hadFiduCut[qA][whichLevel] &&
                hadFiduCut[qB][whichLevel];

  // PID refinement cuts
  cutElePID = TMath::Abs(eleChi2pid) < 5 &&
              elePCALen > 0.07 &&
              eleSampFrac > 0.17 &&
              eleTheta>5 && eleTheta<35;
  for(int h=0; h<2; h++) {
    cutHadPID[h] = CheckHadChi2pid(hadChi2pid[h],hadP[h]) &&
                   hadTheta[h]>5 && hadTheta[h]<35;
  };
  cutPID = cutElePID && cutHadPID[qA] && cutHadPID[qB];
  

  // check if helicity is defined
  cutHelicity = this->SpinState()==sP || this->SpinState()==sM;

};


/////////////////////////////////////////////////////////
// MAIN ANALYSIS CUT
Bool_t EventTree::Valid() {
  return cutDIS && cutDihadron && cutHelicity && 
         cutFiducial && cutPID && cutVertex;
};
/////////////////////////////////////////////////////////



// translate "helicity" to a local index for the spin
Int_t EventTree::SpinState() {
  if(runnum>=5032 && runnum<=5666) {
    // Fall 2018 convention
    switch(helicity) {
      case 1: return sM;
      case -1: return sP;
      case 0: return UNDEF;
      default: fprintf(stderr,"WARNING: bad SpinState request: %d\n",helicity);
    };
  }
  else if(runnum==11) { // MC helicity
    switch(helicityMC[whichHelicityMC]) {
      case 2: return sM;
      case 3: return sP;
      case 0: return UNDEF;
      default: fprintf(stderr,"WARNING: bad SpinState request: %d\n",helicity);
    };
  }
  else fprintf(stderr,"WARNING: runnum %d not in EventTree::SpinState\n",runnum);
  return UNDEF;
};


// return polarization, which can depend on the run number
Float_t EventTree::Polarization() {
  if(runnum>=5032 && runnum<5333)       return 0.8592; // +-0.0129
  else if(runnum>=5333 && runnum<=5666) return 0.8922; // +-0.02509
  else if(runnum==11) return 0.86; // MC
  fprintf(stderr,"WARNING: runnum %d not in EventTree::Polarization\n",runnum);
  return UNDEF;
};

// return relative luminosity, which can depend on HWP position
Float_t EventTree::Rellum() {
  return 1; // disable for now, until we have a good measure of it
};


// return true if the event passes vertex cuts
Bool_t EventTree::CheckVertex() {

  // electron Vz cuts
  if(runnum>=5032 && runnum<=5419) {
    vzBoolEle = -13 < eleVertex[eZ] && eleVertex[eZ] < 12; // inbending
  } else if(runnum>=5422 && runnum<=5666) {
    vzBoolEle = -18 < eleVertex[eZ] && eleVertex[eZ] < 10; // outbending
  } else if(runnum==11) {
    vzBoolEle = -13 < eleVertex[eZ] && eleVertex[eZ] < 12; // inbending MC
  } else {
    if(!vertexWarned) {
      fprintf(stderr,"WARNING: run neither inbending or outbending\n");
      fprintf(stderr,"         electron vertex cut disabled\n");
      vertexWarned = true;
    };
    vzBoolEle = true;
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


// PID refinement cut for pions; the upper bound cut at high momentum
// helps reduce kaon contamination
Bool_t EventTree::CheckHadChi2pid(Float_t chi2pid, Float_t mom) {
  Float_t sigma=1; // n.b. Stefan uses 0.88 for pi+, and 0.93 for pi-

  // lower bound
  if(chi2pid<=-3) return false;
  
  // upper bound (from Stefan) :

  // - standard cut
  ///*
  if(mom<2.44) return chi2pid < 3*sigma; // 3-sigma cut at low p
  else {
    // 1/2 distance cut at higher p
    // (1/2 distance in beta between kaons and pions)
    return chi2pid < sigma * (0.00869 + 
      14.98587 * TMath::Exp(-mom/1.18236) + 1.81751 * exp(-mom/4.86394));
  };
  //*/
  
  // - strict cut
  /*
  if(mom<2.44) return chi2pid < 3*sigma; // 3-sigma cut at low p
  else if(2.44<=mom && mom<4.6) {
    // 1/2 distance cut at mid p
    return chi2pid < sigma * (0.00869 +
      14.98587 * TMath::Exp(-mom/1.18236) + 1.81751 * exp(-mom/4.86394));
  } else {
    // 1-sigma in chi2pid for high p
    return chi2pid < sigma * (-1.14099 + 
      24.14992 * TMath::Exp(-mom/1.36554) + 2.66876 * exp(-mom/6.80552));
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

  objDIS->CalculateKinematics(trEle);
  return objDIS;
};


// return Breit frame rapidity for hadron had (qA or qB)
Float_t EventTree::GetBreitRapidity(Int_t had) {
  if(had!=qA && had!=qB) return UNDEF;
  hadMom[had].SetPtEtaPhiE(hadPt[had],hadEta[had],hadPhi[had],hadE[had]);
  hadMom[had].Boost(this->GetDISObj()->BreitBoost);
  return hadMom[had].Rapidity();
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


EventTree::~EventTree() {
};


