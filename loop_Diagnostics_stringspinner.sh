#!/usr/bin/env bash

cutVer=default
diagnostics.exe "outroot.sss.GLGT_1.4/*.root" plots.sss.GLGT_1.4.root 0x34 $cutVer &
diagnostics.exe "outroot.sss.GLGT_0.2/*.root" plots.sss.GLGT_0.2.root 0x34 $cutVer &
diagnostics.exe "outroot.sss.GLGT_5.0/*.root" plots.sss.GLGT_5.0.root 0x34 $cutVer &
wait

# # these jobs are pretty I/O intensive, just run one at a time and be patient!
# ls -d outroot.sss.GLGT_* | while read d; do
#   # diagnostics.exe "$d/*.root" plots.$d.root |& tee logfiles/$d.log
#   MhDecompose.exe "$d/*.root" plots.$d.root mhdecomp.$d.root |& tee logfiles/$d.log
# done
