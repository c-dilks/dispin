#!/usr/bin/env bash

# these jobs are pretty I/O intensive, just run one at a time and be patient!
ls -d outroot.sss.GLGT_* | while read d; do
  # diagnostics.exe "$d/*.root" plots.$d.root |& tee logfiles/$d.log
  MhDecompose.exe "$d/*.root" plots.$d.root mhdecomp.$d.root |& tee logfiles/$d.log
done
