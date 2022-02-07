#!/bin/bash

ext=png
for tw in 2 3; do
  # 1D binning
  ./pwPlot.py bruspin.DIS.x/asym_mcmc_BL0.root $tw '$x$' $ext
  ./pwPlot.py bruspin.DIS.mh/asym_mcmc_BL0.root $tw '$M_h$ [GeV]' $ext
  # 2D binning
  #for b in 0 1 2; do
    #./pwPlot.py bruspin.DIS.z.mh/asym_mcmc_BL$b.root $tw '$z$' $ext
    #./pwPlot.py bruspin.DIS.pt.mh/asym_mcmc_BL$b.root $tw '$p_T$ [GeV]' $ext
  #done
  # 2D binning, stacked
  ./pwPlot.py bruspin.DIS.z.mh/asym_mcmc_BL0.root $tw '$z$' $ext 1
  ./pwPlot.py bruspin.DIS.pt.mh/asym_mcmc_BL0.root $tw '$p_T$ [GeV]' $ext 1
done


# batch rename
mkdir -p asymout
rm -r asymout
mkdir -p asymout
for file in bruspin.DIS.*/*.png; do
  cp -v $file asymout/$(echo $file | sed 's/bruspin.DIS.//g'|sed 's/\//_/g'|sed 's/\.m/_m/g')
done
