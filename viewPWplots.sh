#!/bin/bash
# wrapper for pwPlot.py

if [ $# -eq 0 ]; then
  echo "USAGE $0 [brufit asym.root] [stackPlots(0/1;def=0)] [xtitle(def='')]"
  echo " - xtitle must be specified between 'single quotes'"
  exit
fi
bruFile=$1
xtitle='~'
stackPlots=0
if [ $# -ge 2 ]; then stackPlots=$2; fi
if [ $# -ge 3 ]; then xtitle="$3"; fi
bruDir=$(echo $bruFile | grep -o '^.*\/')
imList=""

# build schemeList ############
schemeList=""
schemeList="$schemeList 0" # twist2 m=0
schemeList="$schemeList 2 3" # twist2 and twist3
schemeList="$schemeList 4" # DSIDIS
###############################

for scheme in $schemeList; do
  python3 pwPlot.py $bruFile $scheme $xtitle png $stackPlots &&
  imFile=$(ls -t ${bruDir}*.png | head -n1)
  imList="$imList $imFile"
done

sxiv $imList

