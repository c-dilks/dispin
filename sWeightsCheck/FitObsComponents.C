//Run with 
//brufit FitObsComponents.C
{
 
   
  FitManager RF;
  RF.SetUp().SetOutDir("outObs/");
  ///////////////////////////////Load Variables
  RF.SetUp().LoadVariable(TString("PhiH")+Form("[%f,%f]",-TMath::Pi(),TMath::Pi()));
  RF.SetUp().LoadVariable(TString("PhiR")+Form("[%f,%f]",-TMath::Pi(),TMath::Pi()));
  RF.SetUp().LoadVariable(TString("Theta")+Form("[%f,%f]",0.0,TMath::Pi())); // (not in PDF, but signal and BG have clearly different shapes)
  RF.SetUp().LoadVariable(TString("Pol")+Form("[%f,%f]",-1.0,1.0));
  RF.SetUp().LoadVariable(TString("Depol2")+Form("[%f,%f]",0.0,2.5));
  RF.SetUp().LoadVariable(TString("Depol3")+Form("[%f,%f]",0.0,2.5));

  RF.SetUp().LoadCategory("Spin_idx[SpinP=1,SpinM=-1,SpinOff=0]");

  RF.SetUp().SetIDBranchName("Idx");

  ///////////////////////////////Make additional cut on an AuxVar
  //RF.SetUp().AddCut("AUX>2"); //Additional cut based on vars or aux vars
 
  /////////////////////////////Make Model Signal

  RF.SetUp().LoadParameter("AmpT3L0Mp0Lv0P0[0.0,-1,1]");
  RF.SetUp().LoadParameter("AmpT2L1Mp1Lv0P0[0.0,-1,1]");
  RF.SetUp().LoadParameter("AmpT3L1Mp1Lv0P0[0.0,-1,1]");
  RF.SetUp().LoadParameter("AmpT3L1Mm1Lv0P0[0.0,-1,1]");
  RF.SetUp().LoadParameter("AmpT2L2Mp2Lv0P0[0.0,-1,1]");
  RF.SetUp().LoadParameter("AmpT3L2Mp2Lv0P0[0.0,-1,1]");
  RF.SetUp().LoadParameter("AmpT3L2Mm2Lv0P0[0.0,-1,1]");

  RF.SetUp().LoadFormula("ModT3L0Mp0Lv0P0=@Pol[]*@Depol3[]*@Spin_idx[]*sin(@PhiH[])");
  RF.SetUp().LoadFormula("ModT2L1Mp1Lv0P0=@Pol[]*@Depol2[]*@Spin_idx[]*sin(@PhiH[]-@PhiR[])");
  RF.SetUp().LoadFormula("ModT3L1Mp1Lv0P0=@Pol[]*@Depol3[]*@Spin_idx[]*sin(@PhiR[])");
  RF.SetUp().LoadFormula("ModT3L1Mm1Lv0P0=@Pol[]*@Depol3[]*@Spin_idx[]*sin(2*@PhiH[]-@PhiR[])");
  RF.SetUp().LoadFormula("ModT2L2Mp2Lv0P0=@Pol[]*@Depol2[]*@Spin_idx[]*sin(2*@PhiH[]-2*@PhiR[])");
  RF.SetUp().LoadFormula("ModT3L2Mp2Lv0P0=@Pol[]*@Depol3[]*@Spin_idx[]*sin(-@PhiH[]+2*@PhiR[])");
  RF.SetUp().LoadFormula("ModT3L2Mm2Lv0P0=@Pol[]*@Depol3[]*@Spin_idx[]*sin(3*@PhiH[]-2*@PhiR[])");

  RF.SetUp().FactoryPDF("RooComponentsPDF::PWfit(1,{PhiH,PhiR,Theta,Pol,Depol2,Depol3,Spin_idx},=AmpT3L0Mp0Lv0P0;ModT3L0Mp0Lv0P0:AmpT2L1Mp1Lv0P0;ModT2L1Mp1Lv0P0:AmpT3L1Mp1Lv0P0;ModT3L1Mp1Lv0P0:AmpT3L1Mm1Lv0P0;ModT3L1Mm1Lv0P0:AmpT2L2Mp2Lv0P0;ModT2L2Mp2Lv0P0:AmpT3L2Mp2Lv0P0;ModT3L2Mp2Lv0P0:AmpT3L2Mm2Lv0P0;ModT3L2Mm2Lv0P0)"); // copied from output log

  
  RF.SetUp().LoadSpeciesPDF("PWfit",1);

  ////////////////////////////Make Bootstrap
  // RF.Data().BootStrap(400);
  ////////////////////////////Make Bins
  //RF.Bins().LoadBinVar("Eg",4,3,4);
   
  ///////////////////////////Load Data
  //RF.Data().BootStrap(2);

  RF.LoadData("tree","bruspin.sfit.x/X0.18_/TreeData.root");
  RF.LoadSimulated("tree","bruspin.sfit.x/X0.18_/TreePWfit.root","PWfit");
  
  //////////////////////////Load Weight
  RF.Data().LoadWeights("Signal","outsPlot/Weights.root");

 
  //Or try an mcmc minimser 1000-># of points, 200->burnin 200 ~ 1/step size
  //RF.SetMinimiser(new RooMcmcSeq(1000,200,200));

  Here::Go(&RF);
  //OR run with PROOF-LITE on N=4 cores (you can change the 4)
  // Proof::Go(&RF,4);
  //OR run with FARM
  // Farm::Go(&RF,false);
}
