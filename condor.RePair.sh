#!/bin/bash

exe="RePair"
if [ $# -ne 2 ]; then
  echo "======================================="
  echo "WARNING: condor scripts are deprecated!"
  echo "======================================="
  echo "usage: $0 [input outrootDir] [output outrootDir]"
  echo " N.B.: output directory must exist"
  exit 1
fi
indir=$1
outdir=$2

job="job.${exe}.condor";
log="logfiles"
mkdir -p $log
rm -vf ${log}/${exe}.*


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
  app "Arguments = $file $outdir"
  app "Log    = ${log}/${exe}.${suffix}.log"
  app "Output = ${log}/${exe}.${suffix}.out"
  app "Error  = ${log}/${exe}.${suffix}.err"
  app "Queue"
  app ""
done

echo "submitting $job..."
condor_submit $job
