//Run with 
//brufit sPlot.C
{

  sPlot RF;
  RF.SetUp().SetOutDir("outsPlot/");
  ///////////////////////////////Load Variables
  RF.SetUp().LoadVariable("diphM[0.08,0.2]");//should be same name as variable in tree  
  RF.SetUp().SetIDBranchName("Idx");

  /////////////////////////////Make Model Signal
  RF.SetUp().FactoryPDF("Gaussian::Signal( diphM, pi0mu[0.135,0,2], pi0sigma[0.02,0.001,0.1] )");
  RF.SetUp().LoadSpeciesPDF("Signal",1);



  ////////////////////////////////Additional background
  RF.SetUp().FactoryPDF("Chebychev::BG(diphM,{b0[0.0,-1,1],b1[0.0,-1,1]})");
  RF.SetUp().LoadSpeciesPDF("BG",1);


  ///////////////////////////Load Data
  RF.LoadData("tree","splot.x/X0.18_/TreeData.root");


  Here::Go(&RF);
  //Proof::Go(&RF,4); //run proof with 4 workers
  
  //new TCanvas;
  //RF.DrawWeighted("M1>>(100,0,10)","Signal");
  //compare to true signal
  //FiledTree::Read("MyModel","Data.root")->Tree()->Draw("M1","Sig==1","same");
  
  //RF.DeleteWeightedTree();
}
