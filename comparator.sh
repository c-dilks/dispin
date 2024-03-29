#!/bin/bash
#
# run certain `CompareBruAsym` commands

if [ $# -ne 1 ]; then echo "USAGE: $0 [set]"; exit 1; fi
setnum=$1

case $setnum in
  1)
    root CompareBruAsym.C'''(
      "initial amps = 0.1",  "bruspin.dnp2020__init_10__ss_1",
      "initial amps = -0.1", "bruspin.dnp2020__init_-10__ss_1"
    )'''
    ;;
  2)
    root CompareBruAsym.C'''(
      "PW amps",          "bruspin.dnp2020.long__ss_1__burn_1000",
      "PW amps + DSIDIS", "bruspin.dsidis.long__ss_1__burn_1000"
    )'''
    ;;
  3)
    root CompareBruAsym.C'''(
      "PRL amps",             "bruspin.prl.ss_1",
      "PRL amps + 3 UU amps", "bruspin.denom.10amp"
    )'''
    ;;
  4)
    root CompareBruAsym.C'''(
      "step size 0.01, 30k steps", "bruspin.prl.ss_1",
      "step size 0.03, 3k steps",  "bruspin.prl.ss_3"
    )'''
    ;;
  5)
    root CompareBruAsym.C'''(
      "MCMC",    "bruspin.prl.ss_1",
      "Minuit",  "bruspin.prl.minuit"
    )'''
    ;;
  6)
    root CompareBruAsym.C'''(
      "Minuit 318",  "bruspin.prl.minuit,
      "Minuit fa18",    "bruspin.prl.minuit.fa18"
    )'''
    ;;
  7)
    root CompareBruAsym.C'''(
      "MCMC fa18",    "bruspin.prl.mcmc.fa18",
      "Minuit fa18",  "bruspin.prl.minuit.fa18"
    )'''
    ;;
  8)
    root CompareBruAsym.C'''(
      "Minuit", "bruspin.mc.disabled/asym_minuit_BL0.root",
      "MCMC",   "bruspin.mc.disabled/asym_mcmc_BL0.root"
    )'''
    ;;
  9)
    # not much different, how we exclude helicity in MC
    root CompareBruAsym.C'''(
      "MC int. disabled", "bruspin.mc.disabled/asym_mcmc_BL0.root",
      "MC int., with helicity undefined", "bruspin.mc.spinAbsent/asym_mcmc_BL0.root"
    )'''
    ;;
  10)
    # effect of acceptance corrections
    root CompareBruAsym.C'''(
      "MC int. disabled", "bruspin.mc.disabled/asym_mcmc_BL0.root",
      "MC int., with helicity 50/50", "bruspin.mc.spin5050/asym_mcmc_BL0.root"
    )'''
    ;;
  11)
    # all +ve helicity -> nonsense fit results
    root CompareBruAsym.C'''(
      "MC int. disabled", "bruspin.mc.disabled/asym_mcmc_BL0.root",
      "MC int., with all helicity +", "bruspin.mc.spinPositive/asym_mcmc_BL0.root"
    )'''
    ;;
  12)
    root CompareBruAsym.C'''(
      "MC int., with helicity undefined", "bruspin.dpwg.mh.spinAbsent/asym_minuit_BL0.root",
      "MC int. with helicity 50/50", "bruspin.dpwg.mh/asym_minuit_BL0.root"
    )'''
    ;;
  13)
    root CompareBruAsym.C'''(
      "#DeltaY<0.3", "bruspin.dpwg.x.yh/asym_mcmc_BL0.root",
      "#DeltaY>0.7", "bruspin.dpwg.x.yh/asym_mcmc_BL2.root"
    )'''
    ;;
  14)
    root CompareBruAsym.C'''(
      "old MC (PRL)", "bruspin.dpwg.mh.MC_OLD/asym_minuit_BL0.root",
      "new MC (BG-merged)", "bruspin.dpwg.mh.MC_NEW/asym_minuit_BL0.root"
    )'''
    ;;
  15)
    root CompareBruAsym.C'''(
      "CFR",        "bruspin.long.x.yh/asym_minuit_BL2.root",
      "CFR+DSIDIS", "bruspin.long.x.yh.dsidis/asym_minuit_BL2.root"
    )'''
    ;;
  16)
    root CompareBruAsym.C'''(
      "xF>0",      "bruspin.XFtest.XFgt0.z.mh/asym_minuit_BL0.root",
      "no xF cut", "bruspin.XFtest.XFfull.z.mh/asym_minuit_BL0.root"
    )'''
    ;;

  ### StringSpinner
  5550)
    root CompareBruAsym.C'''(
      "StringSpinner",            "bruspin/bruspin.sss/asym_minuit_BL0.root",
      "CLAS12 Transversity 2022", "bruspin/TRANSVERSITY2022.rga.pm.bibending.all.m.minuit/asym_minuit_BL0.root"
    )'''
    ;;
  *)
    echo "ERROR: bad set"
    ;;
esac

