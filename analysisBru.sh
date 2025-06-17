#!/bin/bash
# fit with Brufit (cf. analysis.sh)

if [ $# -lt 4 ]; then
  echo """
  USAGE: $0 [set_number] [catTreeDataFile] [catTreeMCFile] [prefix]

    [set_number]      : defines the binning scheme; see the script
    [catTreeDataFile] : catTree file for data
    [catTreeMCFile]   : catTree file for MC
    [prefix]          : directory name prefix, e.g., sss.GLGT_5.thetaLT_0
  """ >&2
  exit 2
fi
setnum=$1
catTreeDataFile=$2
catTreeMCFile=$3
prefix=$4

case $setnum in
  1) root -b -q $BRUFIT/macros/LoadBru.C asymBruFit.C'("'$catTreeDataFile'", "'$catTreeMCFile'", "bruspin/'$prefix'.pm.x.minuit",   "minuit", "", 0x34, 1,  6, -1, -1)' ;;
  2) root -b -q $BRUFIT/macros/LoadBru.C asymBruFit.C'("'$catTreeDataFile'", "'$catTreeMCFile'", "bruspin/'$prefix'.pm.m.minuit",   "minuit", "", 0x34, 2,  6, -1, -1)' ;;
  3) root -b -q $BRUFIT/macros/LoadBru.C asymBruFit.C'("'$catTreeDataFile'", "'$catTreeMCFile'", "bruspin/'$prefix'.pm.zm.minuit",  "minuit", "", 0x34, 32, 3, 2,  -1)' ;;
  4) root -b -q $BRUFIT/macros/LoadBru.C asymBruFit.C'("'$catTreeDataFile'", "'$catTreeMCFile'", "bruspin/'$prefix'.pm.ptm.minuit", "minuit", "", 0x34, 42, 3, 2,  -1)' ;;
  *)
    echo "ERROR: unknown set_number" >&2
    exit 1
    ;;
esac
