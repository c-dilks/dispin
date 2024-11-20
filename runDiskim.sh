#!/bin/bash
# produces `diskim` file, and subsequently, `outroot` file

if [ $# -lt 1 ]; then
  echo "USAGE: $0 [skim file] [optional:outrootDir] [optional:datastream=data/mcrec/mcgen] [optional:hipotype=skim/dst] [optional:set_helicity]"
  echo "       you can also specify a directory of DST files, but you must also supply the \"dst\" hipotype argument"
  exit 2
fi
skimfile=$1
outrootdir="outroot"
datastream="data"
hipotype="skim"
setHelicity=0
if [ $# -ge 2 ]; then outrootdir="$2"; fi
if [ $# -ge 3 ]; then datastream="$3"; fi
if [ $# -ge 4 ]; then hipotype="$4"; fi
if [ $# -ge 5 ]; then setHelicity=$5; fi

# check setup
if [ -z "$DISPIN_HOME" ]; then
  echo "ERROR: you must source env.sh first" >&2; exit 1;
fi
if [ ! -d "diskim" ]; then
  echo "ERROR: you must create or link a diskim directory" >&2; exit 1;
fi
if [ ! -d $outrootdir ]; then
  echo "ERROR: you must create or link outroot directory $outrootdir" >&2; exit 1;
fi

# check if running with radiative corrections; if so, use `radroot` dir in place of `outroot` dir
if [[ "$datastream" =~ rad$ ]]; then
  outrootdir=$(echo $outrootdir | sed 's/^outroot/radroot/')
  if [ ! -d $outrootdir ]; then
    echo "ERROR: you must create or link radroot directory $outrootdir" >&2; exit 1;
  fi
fi

# produce (intermediate) diskim file
diskimfile="diskim/$(echo $skimfile|sed 's/^.*\///g').root"
run-groovy skimDihadrons.groovy $skimfile $datastream $hipotype $setHelicity
sleep 1

# run calcKinematics
calcKinematics.exe $diskimfile $outrootdir $datastream injgen
sleep 1

# cleanup intermediate diskim file
rm -v $diskimfile
