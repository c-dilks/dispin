#!/bin/bash
# fit stringspinner data

if [ $# -lt 1 ]; then
  echo "USAGE: $0 [set_number]" >&2
  exit 2
fi
setnum=$1

case $setnum in
  1) root -b -q $BRUFIT/macros/LoadBru.C asymBruFit.C'("catTreeData.sss.0x34.idx.root", "", "bruspin/stringspinner.pm.mcgen.x.minuit", "minuit", "", 0x34, 1, 6)' ;;
  2) root -b -q $BRUFIT/macros/LoadBru.C asymBruFit.C'("catTreeData.sss.0x34.idx.root", "", "bruspin/stringspinner.pm.mcgen.m.minuit", "minuit", "", 0x34, 2, 6)' ;;
  3) root -b -q $BRUFIT/macros/LoadBru.C asymBruFit.C'("catTreeData.sss.0x34.idx.root", "", "bruspin/stringspinner.pm.mcgen.zm.minuit", "minuit", "", 0x34, 32, 3, 2)' ;;
  4) root -b -q $BRUFIT/macros/LoadBru.C asymBruFit.C'("catTreeData.sss.0x34.idx.root", "", "bruspin/stringspinner.pm.mcgen.ptm.minuit", "minuit", "", 0x34, 42, 3, 2)' ;;
  *)
    echo "ERROR: unknown set_number" >&2
    exit 1
    ;;
esac
