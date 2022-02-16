#!/bin/bash
# make a directory of symlinks to a subset of outroot files in a particular directory
set -e

if [ $# -ne 2 ]; then
  echo "USAGE: $0 [outroot directory] [number of files to link]"
  exit 2
fi
outrootDir=$1
count=$2

subsetDir=${outrootDir}.subset
mkdir -p $subsetDir
rm -r $subsetDir
mkdir -p $subsetDir

pushd $outrootDir
outrootDir=$(pwd -P)
popd

pushd $subsetDir
ls -S $outrootDir/*.root | head -n$count | while read f; do
  ln -sv $f ./
done
popd

echo ""
echo ""
echo "produced subset $subsetDir"
ls -l --color $subsetDir
