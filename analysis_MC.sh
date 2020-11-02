#!/bin/bash

if [ $# -lt 1 ]; then
  echo "USAGE: $0 [which helicity MC]"
  exit
fi
h=$1

fitmode=42
if [ $# -ge 2 ]; then fitmode=$2; fi
outdir="spinroot_inj"
if [ $# -ge 3 ]; then outdir=$3; fi

outdir=${outdir}_${h}

echo "helicityMC = $h"
echo "fitmode = $fitmode"
echo "outdir = $outdir"
echo ""

slurm.buildSpinroot.sh mcroot -b -i2 -h$h
sleep 5
waitForSlurm.sh dispin
sleep 1
catSpinroot.exe
sleep 1
asymFit.exe $fitmode 0 | tee spinroot/fitOutput.txt

mkdir -p ${outdir}
rm -r ${outdir}
mkdir -p ${outdir}
cp -v spinroot/asym*.root ${outdir}/
cp -v spinroot/table*.dat ${outdir}/
cp -v spinroot/fitOutput.txt ${outdir}/
cp -v spinroot/multi*.png ${outdir}/
