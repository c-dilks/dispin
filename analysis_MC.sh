#!/bin/bash

if [ $# -ne 1 ]; then
  echo "USAGE: $0 [which helicity MC]"
  exit
fi
h=$1

slurm.buildSpinroot.sh outroot.mcrec.injgen -b -i2 -h$h
sleep 1
waitForSlurm.sh
sleep 1
catSpinroot.exe
sleep 1
fitmode=42
asymFit.exe $fitmode 2 | tee spinroot/fitOutput.txt

mkdir -p spinroot_inj_$h
rm -r spinroot_inj_$h
mkdir -p spinroot_inj_$h
cp -v spinroot/asym*.root spinroot_inj_$h/
cp -v spinroot/table*.dat spinroot_inj_$h/
cp -v spinroot/fitOutput.txt spinroot_inj_$h/
cp -v spinroot/multi*.png spinroot_inj_$h/
