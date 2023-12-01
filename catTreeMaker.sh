#!/bin/bash

# arguments
if [ $# -lt 2 ]; then
  echo """USAGE: $0 [outroot dir] [data/mc] [options]
  options:
    -p pairType
    -o output catTree.root file name"""
  exit 2
fi
outrootDir=$1
dataType=$2
shift
shift
pairType="0x34"
outfile=""
while getopts "p:o:" o; do
  case "$o" in
    p) pairType=$OPTARG ;;
    o) outfile=$OPTARG ;;
    *) echo "ERROR: unknown option"; exit 1 ;;
  esac
done
echo "outrootDir=$outrootDir"
echo "dataType=$dataType"
echo "pairType=$pairType"

# check env
if [ -z "$DISPIN_HOME" ]; then
  echo "ERROR: you must source env.sh first"; exit
fi

# buildSpinroot.exe args:
# -n 1 : single bin
# -i 1 : vs. x (doesn't matter, but keeping this convention for asymBrufit.exe)
# -b   : 2d phi binning (for chi2 fit, if you want to use it)
# -p   : pairType
# -s   : MC
args="$outrootDir -b -i1 -n 1 -p $pairType"
if [ "$dataType" == "mc" ]; then args="$args -s"; fi
echo "args = $args"


# build output filename
if [ -z "$outfile" ]; then 
  if [ "$dataType" == "data" ]; then suffix="Data"
  elif [ "$dataType" == "mc" ]; then suffix="MC"
  else suffix=""; fi
  suffix="${suffix}.$(echo $outrootDir | sed 's/outroot\.//g' | sed 's/\/$//').$pairType"
  outfile=catTree${suffix}.root
fi
echo "outfile=$outfile"
mkdir -p `dirname $outfile`


# call buildSpinroot.exe on slurm
slurm.buildSpinroot.sh $args
sleep 3
waitForSlurm.sh dispin_buildSpinroot
echo "EXIT PREMATURELY, FOR DEBUGGING!" && exit


# concatenate spinroot files to catTree.root
sleep 3
touch $outfile; rm $outfile
${ROOTSYS}/bin/hadd $outfile spinroot/tree*.root

# add index variable
root -b -q IndexCatTree.C'("'$outfile'")'
