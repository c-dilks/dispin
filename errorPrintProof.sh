#!/bin/bash

proofdir=$HOME/.proof
jobdir=$(pwd -P | sed 's,'"$HOME/"',,' | sed 's,\/,-,')
logdir=$proofdir/$jobdir/last-lite-session

grep -i error $logdir/*.log > tempolog.1
sort tempolog.1 | uniq > tempolog.2
grep --color -i error tempolog.2
rm tempolog.*

