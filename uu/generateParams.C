const int nAmp = 5;
const int nPar = 3;
int a,p;

void generateParams() {

  float param[nAmp][nPar];
  for(a=0; a<nAmp; a++) {
    param[a][0] = -0.5;
    param[a][1] = 0;
    param[a][2] = 0.5;
  };

  int i[nAmp];
  gSystem->RedirectOutput("params.dat","w");
  for(i[0]=0; i[0]<nPar; i[0]++) {
    for(i[1]=0; i[1]<nPar; i[1]++) {
      for(i[2]=0; i[2]<nPar; i[2]++) {
        for(i[3]=0; i[3]<nPar; i[3]++) {
          for(i[4]=0; i[4]<nPar; i[4]++) {
            for(a=0; a<nAmp; a++) printf("%f ",param[a][i[a]]);
            printf("\n");
          };
        };
      };
    };
  };
  gSystem->RedirectOutput(0);
};


