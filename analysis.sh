#!/bin/bash

if [ $# -lt 1 ]; then echo "USAGE: $0 [set_number] [outroot dir]"; exit; fi
setnum=$1
outrootDir=outroot
if [ $# -ge 2 ]; then outrootDir=$2; fi

case $setnum in
  1)
    slurm.buildSpinroot.sh $outrootDir -b -i1 # vs. x 
    ;;
  2)
    slurm.buildSpinroot.sh $outrootDir -b -i2 # vs. Mh 
    ;;
  3)
    slurm.buildSpinroot.sh $outrootDir -b -i32 # vs. z , for 2 Mh bins
    ;;
  4)
    slurm.buildSpinroot.sh $outrootDir -b -i42 # vs. PhPerp, for 2 Mh bins 
    ;;
  *)
    echo "ERROR: unknown set_number"
    exit
    ;;
esac


sleep 1
#waitForSlurm.sh dispin
waitForCondor.sh

sleep 1
catSpinroot.exe

sleep 1
fitmode=42 # |m|<=2
#fitmode=8001 # |m|<=2 + DSIDIS mods

# chi2 fit
asymFit.exe $fitmode 2 | tee spinroot/fitOutput_chi2.txt
pushd spinroot
mv asym_${fitmode}{,_chi2}.root
mv table_${fitmode}{,_chi2}.dat
for file in multi*.png; do mv $file result_chi2_${file}; done
popd

# ML fit
asymFit.exe $fitmode 0 | tee spinroot/fitOutput_mlm.txt
pushd spinroot
mv asym_${fitmode}{,_mlm}.root
mv table_${fitmode}{,_mlm}.dat
for file in multi*.png; do mv $file result_mlm_${file}; done
popd

sleep 1
mkdir -p spinroot_final_${setnum}
rm -r spinroot_final_${setnum}
mkdir -p spinroot_final_${setnum}
mv spinroot/* spinroot_final_${setnum}/
