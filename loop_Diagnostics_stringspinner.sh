#!/usr/bin/env bash

cutVer=loose
diagnostics.exe "outroot.rga.inbending.fa18/nSidis_0050*.root" plots.nocuts.rga.inbending.fa18.root       0x34 $cutVer &
diagnostics.exe "outroot.mc.richard.inbending/*.root"          plots.nocuts.mc.richard.inbending.root     0x34 $cutVer &
diagnostics.exe "outroot.mc.inbending.bg45/45nA_job_308*.root" plots.nocuts.mc.inbending.bg45.root        0x34 $cutVer &
diagnostics.exe "outroot.sss.aug24_LU_2.inbending/*.root"      plots.nocuts.sss.aug24_LU_2.inbending.root 0x34 $cutVer &
wait

# # these jobs are pretty I/O intensive, just run one at a time and be patient!
# ls -d outroot.sss.GLGT_* | while read d; do
#   # diagnostics.exe "$d/*.root" plots.$d.root |& tee logfiles/$d.log
#   MhDecompose.exe "$d/*.root" plots.$d.root mhdecomp.$d.root |& tee logfiles/$d.log
# done
