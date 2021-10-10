#!/bin/bash
# compare results from analysis_pi0.sh, e.g., sFit vs. sideband method

if [ $# -ne 2 ]; then
  echo "USAGE: $0 [varName] [testNum]"
  exit 1
fi
varName=$1
testNum=$2

minimizer="minuit"

case $testNum in
  1)
    bluetitle="sWeight method"
    redtitle="sideband method"
    bluefile="bruspin.sfit.${varName}/asym_${minimizer}_BL0.root"
    redfile="bruspin.pi0.sig.${varName}/asymBGcorr_${minimizer}_BL0.root"
    ;;
  2)
    bluetitle="sWeight method"
    redtitle="no BG correction"
    bluefile="bruspin.sfit.${varName}/asym_${minimizer}_BL0.root"
    redfile="bruspin.pi0.sig.${varName}/asym_${minimizer}_BL0.root"
    ;;
  3)
    bluetitle="sideband BG correction"
    redtitle="no BG correction"
    bluefile="bruspin.pi0.sig.${varName}/asymBGcorr_${minimizer}_BL0.root"
    redfile="bruspin.pi0.sig.${varName}/asym_${minimizer}_BL0.root"
    ;;
  4)
    bluetitle="fit using BG sWeights"
    redtitle="regular fit of sideband data"
    bluefile="bruspin.sbg.${varName}/asym_${minimizer}_BL0.root"
    redfile="bruspin.pi0.bg.${varName}/asym_${minimizer}_BL0.root"
    ;;
  *)
    echo "bad testNum"
    exit 1
    ;;
esac

root -l CompareBruAsym.C'("'"$bluetitle"'","'$bluefile'","'"$redtitle"'","'$redfile'")'
