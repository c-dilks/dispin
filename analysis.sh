#!/bin/bash

#condor.buildSpinroot.sh outroot -i2 # vs. Mh
#condor.buildSpinroot.sh outroot -i16 # vs. x, for Q2 bins
#condor.buildSpinroot.sh outroot -i24 # vs. Mh, for PhPerp bins
#condor.buildSpinroot.sh outroot -i32 # vs. z, for Mh bins
condor.buildSpinroot.sh outroot -i4 # vs. z, for Mh bins

waitForCondor.sh
sleep 1

catSpinroot.exe
sleep 1

fitmode=4
asymFit.exe $fitmode 0 | tee spinroot/fitOutput.txt

#sleep 1
#mv -v spinroot/multiGrCanv_M.png case${fitmode}.png
