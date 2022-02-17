#!/bin/bash
# make a directory of symlinks pointing to all of the outroot files, satisfying the pattern
set -e

if [ $# -ne 1 ]; then
  echo """
  USAGE: $0 [pattern]
  - pattern is a grep pattern for deciding which directories to include
  - output directory will be called 'outroot.[pattern].all, so do not include
    the word 'outroot' in your pattern
  """
  exit 2
fi
pattern=$1
wd=$(pwd -P)

allsetDir=outroot.${pattern}.all
mkdir -p $allsetDir
rm -r $allsetDir

dirList=$(ls -d outroot* | grep "$pattern" | grep -vE 'all$' | grep -vE 'subset$' | grep -vE 'diph$')

mkdir -pv $allsetDir
pushd $allsetDir

for dir in $dirList; do
  ln -sv $wd/$dir/*.root ./
done

popd

echo ""
echo ""
echo "produced allset $allsetDir"
ls -l --color $allsetDir
