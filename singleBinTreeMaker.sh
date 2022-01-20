#!/bin/bash

# arguments
if [ $# -lt 2 ]; then
  echo "USAGE: $0 [outroot dir] [data/mc] [optional extra args for buildSpinroot.exe]"
  echo "  if you pass extra args such as -p, do NOT pass -b, -i, or -n"
  exit
fi
argset=$*
outrootDir=`echo $argset|awk '{print $1}'`
dataType=`echo $argset|awk '{print $2}'`
argsExtra=`echo $argset|sed 's/[^ ]* *//'|sed 's/[^ ]* *//'`
echo "outrootDir=$outrootDir"
echo "dataType=$dataType"
echo "argsExtra=$argsExtra"

# check env
if [ -z "$DISPIN_HOME" ]; then
  echo "ERROR: you must source env.sh first"; exit
fi

# buildSpinroot.exe args:
# -n 1 : single bin
# -i 1 : vs. x (doesn't matter, but keeping this convention for asymBrufit.exe)
# -b   : 2d phi binning (for chi2 fit, if you want to use it)
args="$outrootDir -b -i1 -n 1 $argsExtra"
if [ "$dataType" == "mc" ]; then args="$args -s"; fi
echo "args = $args"


# build output filename
if [ "$dataType" == "data" ]; then suffix="Data"
elif [ "$dataType" == "mc" ]; then suffix="MC"
else suffix=""; fi
pairtype="$(echo $args | grep '\-p' | sed 's/^.*-p//g' | awk '{print $1}')"
if [ -n "$pairtype" ]; then pairtype=".$pairtype"; fi
suffix="${suffix}.$(echo $outrootDir | sed 's/outroot\.//g' | sed 's/\/$//')$pairtype"
outfile=catTree${suffix}.root
echo "outfile=$outfile"


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


# concatenate spinroot files to catTree.root
sleep 3
touch $outfile; rm $outfile
${ROOTSYS}/bin/hadd $outfile spinroot/tree*.root

# add index variable
root -b -q IndexCatTree.C'("'$outfile'")'
