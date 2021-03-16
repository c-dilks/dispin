#!/bin/bash

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
      "step size 0.02, 3k steps", "bruspin.prl.ss_2",
      "step size 0.03, 3k steps", "bruspin.prl.ss_3"
    )'''
    ;;
  *)
    echo "ERROR: bad set"
    ;;
esac

