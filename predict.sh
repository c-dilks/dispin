#!/bin/bash
# call CompareAsym.C with predictShift==true

if [ $# -ne 2 ]; then
  echo "usage: $0 [inj1] [inj2]"
  exit
fi

file1=spinroot_injCHI/spinroot_inj_${1}/asym_42.root
file2=spinroot_injCHI/spinroot_inj_${2}/asym_42.root
root -l CompareAsyms.C'("'$file1'","'$file2'",1)'
