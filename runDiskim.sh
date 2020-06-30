#!/bin/bash
# produces `diskim` file, and subsequently, `outroot` file

if [ $# -ne 1 ]; then
  echo "USAGE: $0 [skim file]"
fi

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

skimfile=$1
diskimfile="diskim/$(echo $skimfile|sed 's/^.*\///g').root"
run-groovy skimDihadrons.groovy $skimfile && sleep 1 && calcKinematics.exe $diskimfile
sleep 1
rm -v $diskimfile
