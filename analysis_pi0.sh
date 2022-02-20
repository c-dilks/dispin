#!/bin/bash
# analyze pi0 pi+ dihadrons for chosen binning

# brufit alias
function brufit { root -b -q $BRUFIT/macros/LoadBru.C $*; }


# single-bin sPlot
splotdir="splot.singlebin"
mkdir -p $splotdir
rm -r $splotdir
brufit -b -q sPlotBru.C'("'$splotdir'",1,1)'


# loop over binning schemes
for iv in 1 2 3 4; do
#for iv in 1; do

  # convert iv number to variable name string
  case $iv in
    1) varName="x";;
    2) varName="m";;
    3) varName="z";;
    4) varName="pt";;
    *) echo "bad iv"; exit 1;;
  esac


  # USE SFIT FOR BG CORRECTION #########################

  # -- fit diphM and calculate sWeights
  splotdir="splot.$varName"
  mkdir -p $splotdir
  rm -r $splotdir
  brufit -b -q sPlotBru.C'("'$splotdir'",'$iv',6)'
  
  # -- perform sFit
  brudir="bruspin.sfit.$varName"
  mkdir -p $brudir
  rm -r $brudir
  brufit -b -q asymBruFit.C'("catTreeData.rga_inbending_all.0x3b.idx.trimmed.root","catTreeMC.mc.PRL.0x3b.idx.trimmed.root","'$brudir'","minuit","'$splotdir'",'$iv',6)'
  errorPrintProof.sh | tee $brudir/ERRORS.log


  # USE SIDEBAND FOR BG CORRECTION #########################
  # TODO
  #brufit -b -q asymBruFit.C'("...")' # sig+bg
  #brufit -b -q asymBruFit.C'("...")' # bg
  #diphotonFit.exe catTreeData.rga_inbending_all.0x3b.root $iv 6
  #root -b -q bgCorrector.C'("...")'
done
