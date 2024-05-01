#!/bin/bash
# plot stringspinner asymmetries compared to CLAS12 results

if [ $# -lt 2 ]; then
  echo """
  USAGE: $0 [set_number] [prefixes]...

    [set_number]:   defines the binning scheme; see the script
    [prefixes]...:  directory name prefix, e.g., stringspinner.GLGT_5.thetaLT_0
                    (specify multiple, if you want)
  """ >&2
  exit 2
fi
setnum=$1
shift
prefixes=$@

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

    sssFiles=()

    case $setnum in
      1)
        sssFileAppend() { sssFiles+=("bruspin/$1.pm.mcgen.x.minuit/asym_minuit_BL$bl.root"); }
        clasDir="bruspin/TRANSVERSITY2022.rga.pm.bibending.all.x.minuit"
        titleArgs=(-x '$x$')
        ;;
      2)
        sssFileAppend() { sssFiles+=("bruspin/$1.pm.mcgen.m.minuit/asym_minuit_BL$bl.root"); }
        clasDir="bruspin/TRANSVERSITY2022.rga.pm.bibending.all.m.minuit"
        titleArgs=(-x '$M_h$ [GeV]')
        ;;
      3)
        sssFileAppend() { sssFiles+=("bruspin/$1.pm.mcgen.zm.minuit/asym_minuit_BL$bl.root"); }
        clasDir="bruspin/TRANSVERSITY2022.rga.pm.bibending.all.zm.minuit"
        titleArgs=(-x '$z$' -e '$M_h$ bin '$bl)
        ;;
      4)
        sssFileAppend() { sssFiles+=("bruspin/$1.pm.mcgen.ptm.minuit/asym_minuit_BL$bl.root"); }
        clasDir="bruspin/TRANSVERSITY2022.rga.pm.bibending.all.ptm.minuit"
        titleArgs=(-x '$p_T$ [GeV]' -e '$M_h$ bin '$bl)
        ;;
    esac

    for prefix in ${prefixes[@]}; do
      sssFileAppend $prefix
    done

    legendLabels="$(echo "${prefixes[@]}" | sed 's/ /;/g');CLAS12"

    pwPlot.py                             \
      -s $s                               \
      -l "'$legendLabels'"                \
      "${titleArgs[@]}"                   \
      "${sssFiles[@]}"                    \
      $clasDir/asym_minuit_BL$bl.root

  done
done

