#!/bin/bash

if [ $# -ne 1 ]; then echo "USAGE: $0 [set_number]"; exit; fi
setnum=$1

case $setnum in
  1)
    condor.buildSpinroot.sh outroot.inbending -i1 # vs. x 
    ;;
  2)
    condor.buildSpinroot.sh outroot.inbending -i2 # vs. Mh 
    ;;
  3)
    condor.buildSpinroot.sh outroot.inbending -i3 # vs. z 
    ;;
  4)
    condor.buildSpinroot.sh outroot.inbending -i42 # vs. PhPerp, for 2 Mh bins 
    ;;
  *)
    echo "ERROR: unknown set_number"
    exit
    ;;
esac


sleep 1
#waitForSlurm.sh
waitForCondor.sh

sleep 1
catSpinroot.exe

sleep 1
fitmode=42 # |m|<=2
asymFit.exe $fitmode 0 | tee spinroot/fitOutput.txt

sleep 1
mkdir -P spinroot_final_${setnum}
mv spinroot/* spinroot_final_${setnum}/
