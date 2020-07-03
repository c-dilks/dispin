#!/bin/bash
condor.buildSpinroot.sh outroot -i2
waitForCondor.sh
catSpinroot.exe
asymFit.exe 4 0
