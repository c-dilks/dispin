#!/bin/bash
# plot stringspinner asymmetries compared to CLAS12 results

if [ $# -lt 2 ]; then
  echo """
  USAGE: $0 [set_number] [prefix]

    [set_number]: defines the binning scheme; see the script
    [prefix]:     directory name prefix, e.g., stringspinner.GLGT_5.thetaLT_0
  """ >&2
  exit 2
fi
setnum=$1
prefix=$2

case $setnum in
  1) blList=(0) ;;
  2) blList=(0) ;;
  3) blList=(0 1) ;;
  4) blList=(0 1) ;;
  *)
    echo "ERROR: unknown set_number" >&2
    exit 1
    ;;
esac

for s in 2 3; do
  for bl in ${blList[*]}; do

    case $setnum in
      1)
        sssDir="bruspin/$prefix.pm.mcgen.x.minuit"
        clasDir="bruspin/TRANSVERSITY2022.rga.pm.bibending.all.x.minuit"
        titleArgs=(-x '$x$')
        ;;
      2)
        sssDir="bruspin/$prefix.pm.mcgen.m.minuit"
        clasDir="bruspin/TRANSVERSITY2022.rga.pm.bibending.all.m.minuit"
        titleArgs=(-x '$M_h$ [GeV]')
        ;;
      3)
        sssDir="bruspin/$prefix.pm.mcgen.zm.minuit"
        clasDir="bruspin/TRANSVERSITY2022.rga.pm.bibending.all.zm.minuit"
        titleArgs=(-x '$z$' -e '$M_h$ bin '$bl)
        ;;
      4)
        sssDir="bruspin/$prefix.pm.mcgen.ptm.minuit"
        clasDir="bruspin/TRANSVERSITY2022.rga.pm.bibending.all.ptm.minuit"
        titleArgs=(-x '$p_T$ [GeV]' -e '$M_h$ bin '$bl)
        ;;
    esac

    pwPlot.py                        \
      -s $s                          \
      -l 'StringSpinner;CLAS12'      \
      "${titleArgs[@]}"              \
      $sssDir/asym_minuit_BL$bl.root \
      $clasDir/asym_minuit_BL$bl.root

  done
done

