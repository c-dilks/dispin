#!/bin/bash

dir="spinroot_injpw"

# twist 2
analysis_MC.sh 2 500 ${dir}_500
for h in {6..10}; do
  analysis_MC.sh $h 500 ${dir}_500
  analysis_MC.sh $h 501 ${dir}_501
  analysis_MC.sh $h 502 ${dir}_502
done

# twist 3
analysis_MC.sh 3 600 ${dir}_600
for h in {11..15}; do
  analysis_MC.sh $h 600 ${dir}_600
  analysis_MC.sh $h 601 ${dir}_601
  analysis_MC.sh $h 602 ${dir}_602
done
