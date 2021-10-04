#!/bin/bash
# compare results from analysis_pi0.sh, e.g., sFit vs. sideband method

if [ $# -ne 1 ]; then
  echo "USAGE: $0 [varName]"
  exit 1
fi
varName=$1

minimizer="minuit"

bluefile="bruspin.sfit.${varName}/asym_${minimizer}_BL0.root"
redfile="bruspin.pi0.sig.${varName}/asymBGcorr_${minimizer}_BL0.root"
bluetitle="sFit method"
redtitle="sideband method"

#bluefile="bruspin.sbg.${varName}/asym_${minimizer}_BL0.root"
#redfile="bruspin.pi0.bg.${varName}/asym_${minimizer}_BL0.root"
#bluetitle="sFit using BG weights"
#redtitle="regular fit of sideband data"

root -l -b -q CompareBruAsym.C'("'"$bluetitle"'","'$bluefile'","'"$redtitle"'","'$redfile'")'
