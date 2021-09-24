#!/bin/bash
# analyze pi0 pi+ dihadrons for chosen binning

function brufit { root -b -q $BRUFIT/macros/LoadBru.C $*; }

for iv in 1 2 3 4; do

  # convert iv number to variable name string
  case $iv in
    1) varName="x";;
    2) varName="m";;
    3) varName="z";;
    4) varName="pt";;
    *) echo "bad iv"; exit 1;;
  esac


  # USE SFIT FOR BG CORRECTION #########################
  splotdir="splot.$varName"
  brudir="bruspin.sfit.$varName"
  mkdir -p $splotdir
  mkdir -p $brudir
  rm -r $splotdir $brudir
  brufit -b -q sPlotBru.C'("'$splotdir'",'$iv',6)'
  brufit -b -q asymBruFit.C'("'$brudir'","minuit","'$splotdir'",'$iv',6)'
  errorPrintProof.sh | tee $brudir/ERRORS.log


  # USE SIDEBAND FOR BG CORRECTION #########################
  # TODO
  #brufit -b -q asymBruFit.C'("...")' # sig+bg
  #brufit -b -q asymBruFit.C'("...")' # bg
  #diphotonFit.exe catTreeData.rga_inbending_all.0x3b.root $iv 6
  #root -b -q bgCorrector.C'("...")'
done
