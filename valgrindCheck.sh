#!/bin/bash

valgrind \
  --leak-check=full \
  --log-file="valgrind.log" \
  --num-callers=40 \
  --suppressions=$ROOTSYS/etc/valgrind-root.supp \
  catSpinroot.exe
  #--track-origins=yes \
