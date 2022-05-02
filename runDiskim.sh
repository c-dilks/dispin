#!/bin/bash
# produces `diskim` file, and subsequently, `outroot` file

if [ $# -lt 1 ]; then
  echo "USAGE: $0 [skim file] [optional:outrootDir] [optional:datastream=data/mcrec/mcgen] [optional:hipotype=skim/dst]"
  echo "       you can also specify a directory of DST files, but you must also supply the \"dst\" hipotype argument"
  exit 2
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
  echo "ERROR: you must source env.sh first"; exit 1;
fi
if [ ! -d "diskim" ]; then
  echo "ERROR: you must create or link a diskim directory"; exit 1;
fi
if [ ! -d $outrootdir ]; then
  echo "ERROR: you must create or link outroot directory $outrootdir"; exit 1;
fi

# check if running with radiative corrections
if [[ "$datastream" =~ rad$ ]]; then
  useRad=1
  radstream=$datastream
  datastream=$(echo $radstream | sed 's/rad$//')
  radrootdir=$(echo $outrootdir | sed 's/^outroot/radroot/')
  if [ ! -d $radrootdir ]; then
    echo "ERROR: you must create or link radroot directory $radrootdir"; exit 1;
  fi
else
  useRad=0
fi

# produce (intermediate) diskim file
diskimfile="diskim/$(echo $skimfile|sed 's/^.*\///g').root"
run-groovy skimDihadrons.groovy $skimfile $datastream $hipotype
sleep 1

# run calcKinematics
if [ "$datastream" = "data" ]; then
  calcKinematics.exe $diskimfile $outrootdir $datastream
elif [ "$datastream" = "mcrec" ]; then
  calcKinematics.exe $diskimfile $outrootdir $datastream injgen
elif [ "$datastream" = "mcgen" ]; then
  calcKinematics.exe $diskimfile $outrootdir $datastream
else
  echo "ERROR: unknown datastream $datastream; diskim file was produced, stopping here."
  exit 1
fi
sleep 1

# if using radiative corrections, run calcKinematics a 2nd time,
# with the RC-corrected beam energy model
if [ $useRad = 1 ]; then
  if [ "$datastream" = "data" ]; then
    calcKinematics.exe $diskimfile $radrootdir $radstream
  elif [ "$datastream" = "mcrec" ]; then
    calcKinematics.exe $diskimfile $radrootdir $radstream injgen
  elif [ "$datastream" = "mcgen" ]; then
    calcKinematics.exe $diskimfile $radrootdir $radstream
  fi
  sleep 1
fi

# cleanup intermediate diskim file
rm -v $diskimfile
