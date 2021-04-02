#!/bin/bash

ext=png
for tw in 0 2 3; do
  # 1D binning
  python3 pwPlot.py bruspin.DIS.x/asym_mcmc_BL0.root $tw '$x$' $ext
  python3 pwPlot.py bruspin.DIS.mh/asym_mcmc_BL0.root $tw '$M_h$ [GeV]' $ext
  # 2D binning
  #for b in 0 1 2; do
    #python3 pwPlot.py bruspin.DIS.z.mh/asym_mcmc_BL$b.root $tw '$z$' $ext
    #python3 pwPlot.py bruspin.DIS.pt.mh/asym_mcmc_BL$b.root $tw '$p_T$ [GeV]' $ext
  #done
  # 2D binning, stacked
  python3 pwPlot.py bruspin.DIS.z.mh/asym_mcmc_BL0.root $tw '$z$' $ext 1
  python3 pwPlot.py bruspin.DIS.pt.mh/asym_mcmc_BL0.root $tw '$p_T$ [GeV]' $ext 1
done
