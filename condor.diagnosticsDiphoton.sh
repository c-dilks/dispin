#!/bin/bash

exe="diagnosticsDiphoton"
if [ $# -lt 1 ]; then
  echo "usage: $0 [input outrootDir] [ARGUMENTS]"
  echo ""
  echo "[ARGUMENTS] are those of diagnosticsDiphoton.exe listed below, except input outroot file":
  ./diagnosticsDiphoton.exe
  exit 1
fi
indir=$1
opts=`echo $*|sed 's/[^ ]* *//'`
echo "indir = $indir"
echo "opts = $opts"

job="job.${exe}.condor";
log="logfiles"
mkdir -p $log
rm -vf ${log}/${exe}.*

outdir="diagdiph/set"

mkdir -p $outdir
echo "check if $outdir directory is empty..."
if [ -z "$(ls $outdir/*.root)" ]; then
  echo "... yes"
else
  echo "... no"
  echo "ERROR: you must empty $outdir directory"
  exit 1
fi

function app { echo "$1" >> $job; }

echo "generate condor job descriptor: $job"
> $job
app "Executable = ${exe}.exe"
app "Universe = vanilla"
app "notification = never"
app "getenv = True"
app ""


for file in ${indir}/*.root; do
  suffix=`echo $file|sed 's/^.*\///g'|sed 's/\.root$//g'`
  echo "prepare to analyze $file"
  app "Arguments = $file $opts"
  app "Log    = ${log}/${exe}.${suffix}.log"
  app "Output = ${log}/${exe}.${suffix}.out"
  app "Error  = ${log}/${exe}.${suffix}.err"
  app "Queue"
  app ""
done

echo "submitting $job..."
condor_submit $job
sleep 5

waitForCondor.sh

catFile="diagdiph/cat__$(echo $indir | sed 's/\//_/g')__$(echo $opts | sed 's/ /__/g').root"
echo $catFile
touch $catFile; rm $catFile
hadd $catFile $outdir/*.root

