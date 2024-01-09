#!/bin/bash
# plot stringspinner asymmetries compared to CLAS12 results

if [ $# -lt 1 ]; then
  echo "USAGE: $0 [set_number]" >&2
  exit 2
fi
setnum=$1

case $setnum in
  1)
    sssDir="bruspin/stringspinner.pm.mcgen.x.minuit"
    clasDir="bruspin/TRANSVERSITY2022.rga.pm.bibending.all.x.minuit"
    ;;
  2)
    sssDir="bruspin/stringspinner.pm.mcgen.m.minuit"
    clasDir="bruspin/TRANSVERSITY2022.rga.pm.bibending.all.m.minuit"
    ;;
  3)
    sssDir="bruspin/stringspinner.pm.mcgen.zm.minuit"
    clasDir="bruspin/TRANSVERSITY2022.rga.pm.bibending.all.zm.minuit"
    ;;
  4)
    sssDir="bruspin/stringspinner.pm.mcgen.ptm.minuit"
    clasDir="bruspin/TRANSVERSITY2022.rga.pm.bibending.all.ptm.minuit"
    ;;
  *)
    echo "ERROR: unknown set_number" >&2
    exit 1
    ;;
esac

for s in 2 3; do
  pwPlot.py \
    -s $s \
    -l 'StringSpinner;CLAS12' \
    $sssDir/asym_minuit_BL0.root \
    $clasDir/asym_minuit_BL0.root
done
