#!/bin/bash

condor.buildSpinroot.sh outroot -i2
waitForCondor.sh
sleep 1

catSpinroot.exe
sleep 1

fitmode=4
asymFit.exe $fitmode 0
sleep 1
mv -v spinroot/multiGrCanv_M.png case${fitmode}.png
