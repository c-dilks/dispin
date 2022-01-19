#!/bin/bash
# list bruspin directories associated to cancelled slurm jobs; pipe stdout through
# `xargs rm -rvi` to remove them
errorPrintFindInLog.rb cancel asymBruFit.C i | grep Proc | cut -d'"' -f2
