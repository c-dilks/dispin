#!/bin/bash
# produces `diskim` file, and subsequently, `outroot` file

if [ $# -lt 1 ]; then
  echo "USAGE: $0 [skim file] [optional:outrootDir] [optional:datastream=data/mcrec/mcgen] [optional:hipotype=skim/dst]"
  echo "       you can also specify a directory of DST files, but you must also supply the \"dst\" hipotype argument"
  exit
fi
skimfile=$1
outrootdir="outroot"
datastream="data"
hipotype="skim"
if [ $# -ge 2 ]; then outrootdir="$2"; fi
if [ $# -ge 3 ]; then datastream="$3"; fi
if [ $# -ge 4 ]; then hipotype="$4"; fi

# check setup
if [ -z "$DISPIN_HOME" ]; then
  echo "ERROR: you must source env.sh first"; exit
fi
if [ ! -d "diskim" ]; then
  echo "ERROR: you must create or link a diskim directory"; exit;
fi
if [ ! -d $outrootdir ]; then
  echo "ERROR: you must create or link an outroot directory"; exit;
fi

diskimfile="diskim/$(echo $skimfile|sed 's/^.*\///g').root"
run-groovy skimDihadrons.groovy $skimfile $datastream $hipotype
sleep 1
echo "PREMATURE EXIT"; exit
if [ "$datastream" = "data" ]; then
  calcKinematics.exe $diskimfile $outrootdir
elif [ "$datastream" = "mcrec" ]; then
  calcKinematics.exe $diskimfile $outrootdir $datastream injgen
  #calcKinematics.exe $diskimfile $outrootdir $datastream injrec
elif [ "$datastream" = "mcgen" ]; then
  calcKinematics.exe $diskimfile $outrootdir $datastream
fi
sleep 1
rm -v $diskimfile
