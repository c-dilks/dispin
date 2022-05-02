#!/bin/bash

exe="samplingFractionTreeMaker"
if [ $# -eq 0 ]; then
  echo "usage: $0 [ARGUMENTS]"
  echo ""
  echo "the first argument must be the directory of outroot files to analyze;"
  echo "the remaining arguments can be any of the options listed below:"
  echo ""
  ./${exe}.exe 2>/dev/null | grep OPTIONS -A100

  exit
fi

args=$*
indir=`echo $args|awk '{print $1}'`
opts=`echo $args|sed 's/[^ ]* *//'`

job="job.${exe}.condor";
log="logfiles"
mkdir -p $log
# rm -vf ${log}/${exe}.*

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
  app "Arguments = -f $file $opts"
  app "Log    = ${log}/${exe}.${suffix}.log"
  app "Output = ${log}/${exe}.${suffix}.out"
  app "Error  = ${log}/${exe}.${suffix}.err"
  app "Queue"
  app ""
done

echo "submitting $job..."
condor_submit $job
