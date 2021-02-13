#!/bin/bash

# arguments
if [ $# -lt 1 ]; then echo "USAGE: $0 [outroot dir]"; exit; fi
outrootDir=$1

# check env
if [ -z "$DISPIN_HOME" ]; then
  echo "ERROR: you must source env.sh first"; exit
fi

# buildSpinroot.exe args:
# -n 1 : single bin
# -i 1 : vs. x (doesn't matter, but keeping this convention for asymBrufit.exe)
# -b   : 2d phi binning (for chi2 fit, if you want to use it)
args="$outrootDir -b -i1 -n 1"


# call buildSpinroot.exe on condor or slurm
if [[ $(hostname) =~ "ifarm" ]]; then
  echo "on ifarm, will use SLURM"
  slurm.buildSpinroot.sh $args
  sleep 3
  waitForSlurm.sh dispin
else
  echo "not on ifarm, will use CONDOR"
  condor.buildSpinroot.sh $args
  sleep 3
  waitForCondor.sh
fi

# concatenate spinroot files to cat.root
sleep 3
catSpinroot.exe
