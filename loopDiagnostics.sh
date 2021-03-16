#!/bin/bash

ls -d outroot.rga_* | sed 's/\/$//' |
while read directory; do
  outfile=`echo $directory | sed 's/^outroot/plots/'`.root
  echo "submit job diagnostics.exe on directory $directory"
  diagnostics.exe "$directory/*.root" $outfile 0x34 2>&1 > $outfile.log &
done
echo "jobs submitted, monitor plots.*.log for progress"
