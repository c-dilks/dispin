#!/bin/bash

grep -i error $PROOF_LOG/worker-*-*.log > tempolog.1
sort tempolog.1 | uniq > tempolog.2
grep --color -i error tempolog.2
#grep --color -i error tempolog.1
rm tempolog.*

