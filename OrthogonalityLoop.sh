#!/bin/bash
# runs Orthogonality.C looping over final analysis bins

outrootDir="outroot.22gev.proton.all"
printLatex="false"

# BuildOrtho.exe -d $outrootDir -o ortho.x.root -i1 &
# BuildOrtho.exe -d $outrootDir -o ortho.m.root -i2 &
# BuildOrtho.exe -d $outrootDir -o ortho.zm.root -i32 &
# BuildOrtho.exe -d $outrootDir -o ortho.pm.root -i42 &
# wait
# exit

outfile="orthotablesLU.tex"
#outfile="orthotablesUU.tex"

> $outfile

for iv in x m; do
  for b in {0..5}; do
    let bin=b+1
    if [ "$iv" == "x" ]; then bincap='$x$'; fi
    if [ "$iv" == "m" ]; then bincap='$M_h$'; fi
    root -b -q Orthogonality.C'('$b',0,"'ortho.${iv}.root'",'$printLatex')'
    cat ortho.${iv}.${b}.tex |\
      sed "s/BINCAP/$bincap bin $bin/g" |\
      sed "s/BINLAB/${iv}${b}/g" >> $outfile
    #exit # for testing
  done
done

for iv in zm pm; do
  for b in {0..2} {16..18}; do
    if [ $b -lt 10 ]; then
      mbin='$M_h<0.63$ GeV'
      let ibin=b+1
    else
      mbin='$M_h>0.63$ GeV'
      let ibin=b-15
    fi
    if [ "$iv" == "zm" ]; then bincap='$z$'; fi
    if [ "$iv" == "pm" ]; then bincap='$P_h^\\perp$'; fi
    root -b -q Orthogonality.C'('$b',0,"'ortho.${iv}.root'",'$printLatex')'
    cat ortho.${iv}.${b}.tex |\
      sed "s/BINCAP/$bincap bin $ibin and $mbin/g" |\
      sed "s/BINLAB/${iv}${b}/g" >> $outfile
  done
done
