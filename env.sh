#!/bin/bash

# software paths
export DISPIN_HOME=$(dirname $(realpath $0))
export BRUFIT=${DISPIN_HOME}/deps/brufit
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${BRUFIT}/lib

# PROOF logs (your path may be different)
jobdir=$(echo $DISPIN_HOME | sed 's,'"$HOME/"',,' | sed 's,\/,-,g')
export PROOF_LOG=${HOME}/.proof/${jobdir}/last-lite-session

# set dependency environment variables
pushd deps

pushd j2root
source setup.sh
popd

pushd clasqaDB
source env.sh
popd

popd

# print results
env|grep --color -w DISPIN_HOME
env|grep --color -w BRUFIT
env|grep --color -w LD_LIBRARY_PATH
env|grep --color -w JYPATH
env|grep --color -w PROOF_LOG

# brufit alias
alias brufit="root $BRUFIT/macros/LoadBru.C"
alias brufitq="brufit -b -q"
