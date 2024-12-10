#!/bin/bash
# fit stringspinner data

if [ $# -lt 2 ]; then
  echo """
  USAGE: $0 [set_number] [prefix]

    [set_number]: defines the binning scheme; see the script
    [prefix]:     directory name prefix, e.g., sss.GLGT_5.thetaLT_0
  """ >&2
  exit 2
fi
setnum=$1
prefix=$2

# NOTE: useDepol MUST be set to 0 here, so that depolarization is NOT used in the fit!
case $setnum in
  1) root -b -q $BRUFIT/macros/LoadBru.C asymBruFit.C'("catTrees/catTreeData.'$prefix'.0x34.idx.root", "", "bruspin/'$prefix'.pm.mcgen.x.minuit",   "minuit", "", 0x34, 1,  6, -1, -1, -1, 0)' ;;
  2) root -b -q $BRUFIT/macros/LoadBru.C asymBruFit.C'("catTrees/catTreeData.'$prefix'.0x34.idx.root", "", "bruspin/'$prefix'.pm.mcgen.m.minuit",   "minuit", "", 0x34, 2,  6, -1, -1, -1, 0)' ;;
  3) root -b -q $BRUFIT/macros/LoadBru.C asymBruFit.C'("catTrees/catTreeData.'$prefix'.0x34.idx.root", "", "bruspin/'$prefix'.pm.mcgen.zm.minuit",  "minuit", "", 0x34, 32, 3, 2,  -1, -1, 0)' ;;
  4) root -b -q $BRUFIT/macros/LoadBru.C asymBruFit.C'("catTrees/catTreeData.'$prefix'.0x34.idx.root", "", "bruspin/'$prefix'.pm.mcgen.ptm.minuit", "minuit", "", 0x34, 42, 3, 2,  -1, -1, 0)' ;;
  *)
    echo "ERROR: unknown set_number" >&2
    exit 1
    ;;
esac
