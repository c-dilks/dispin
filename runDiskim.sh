#!/bin/bash
# produces `diskim` file, and subsequently, `outroot` file

if [ $# -lt 1 ]; then
  echo "USAGE: $0 [skim file] [optional:data/mcrec/mcgen]"
  exit
fi
skimfile=$1
datastream="data"
hipotype="skim"
if [ $# -ge 2 ]; then datastream="$2"; fi
if [ $# -ge 3 ]; then hipotype="$3"; fi

# check setup
if [ -z "$DISPIN_HOME" ]; then
  echo "ERROR: you must source env.sh first"; exit
fi
if [ ! -d "diskim" ]; then
  echo "ERROR: you must create or link a diskim directory"; exit;
fi
if [ ! -d "outroot" ]; then
  echo "ERROR: you must create or link an outroot directory"; exit;
fi

diskimfile="diskim/$(echo $skimfile|sed 's/^.*\///g').root"
run-groovy skimDihadrons.groovy $skimfile $datastream $hipotype
sleep 1
if [ "$datastream" = "data" ]; then
  calcKinematics.exe $diskimfile
elif [ "$datastream" = "mcrec" ]; then
  calcKinematics.exe $diskimfile $datastream injgen
  #calcKinematics.exe $diskimfile $datastream injrec
elif [ "$datastream" = "mcgen" ]; then
  calcKinematics.exe $diskimfile $datastream
fi
sleep 1
rm -v $diskimfile
