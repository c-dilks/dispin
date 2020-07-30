#!/bin/bash

#condor.buildSpinroot.sh outroot -i2 # vs. Mh
#condor.buildSpinroot.sh outroot -i16 # vs. x, for Q2 bins
#condor.buildSpinroot.sh outroot -i24 # vs. Mh, for PhPerp bins
#condor.buildSpinroot.sh outroot -i32 # vs. z, for Mh bins
#condor.buildSpinroot.sh outroot -i4 #
#waitForCondor.sh

slurm.buildSpinroot.sh outroot.inbending -i2 # vs. Mh
#slurm.buildSpinroot.sh outroot.inbending -i73 # vs. xF, for z bins
sleep 1
waitForSlurm.sh

sleep 1

catSpinroot.exe
sleep 1

fitmode=5
asymFit.exe $fitmode 0 | tee spinroot/fitOutput.txt

sleep 1
cp -v spinroot/multiGrCanv_M.png ${HOME}/drop/case${fitmode}.png
