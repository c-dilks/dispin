const int nAmp = 5;
const int nPar = 5; // must be odd 
int a,p;

void generateParams() {

  float param[nAmp][nPar];
  /*
  for(a=0; a<nAmp; a++) {
    param[a][0] = -0.5;
    param[a][1] = 0;
    param[a][2] = 0.5;
  };
  */
  // lattice in the range |D|<dlim, with nstep points in the range (0,dlim]
  int nstep = (nPar-1)/2;
  float dlim = 1;
  printf("generating lattice with points:\n");
  printf("------------\n");
  for(a=0; a<nAmp; a++) {
    for(int k=0; k<2*nstep+1; k++) {
      param[a][k] = k*dlim/((float)nstep)-dlim;
      if(a==0) printf("%.3f\n",param[a][k]);
    };
  };
  printf("------------\n");

  

  int i[nAmp];
  int cnt=0;
  gSystem->RedirectOutput("params.dat","w");
  for(i[0]=0; i[0]<nPar; i[0]++) {
    for(i[1]=0; i[1]<nPar; i[1]++) {
      for(i[2]=0; i[2]<nPar; i[2]++) {
        for(i[3]=0; i[3]<nPar; i[3]++) {
          for(i[4]=0; i[4]<nPar; i[4]++) {
            for(a=0; a<nAmp; a++) printf("%f ",param[a][i[a]]);
            printf("\n");
            cnt++;
          };
        };
      };
    };
  };
  gSystem->RedirectOutput(0);
  printf("%d total points\n",cnt);
};


