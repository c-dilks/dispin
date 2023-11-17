#!/bin/bash

# software paths
export DISPIN_HOME=$(dirname $(realpath $0))
export BRUFIT=${DISPIN_HOME}/deps/brufit
export STRINGSPINNER=${DISPIN_HOME}/deps/StringSpinner
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${BRUFIT}/lib

# set dependency environment variables
pushd deps

pushd j2root
source setup.sh
popd

pushd clas12-qadb
source env.sh
popd

popd

# find pythia
pythia_search=pythia8-config
which $pythia_search &&
  export PYTHIADIR=$(realpath $(dirname $(which $pythia_search))/..)

# print results
echo ""
echo "===== DISPIN ENVIRONMENT ====="
env|grep --color -w DISPIN_HOME
env|grep --color -w BRUFIT
env|grep --color -w LD_LIBRARY_PATH
env|grep --color -w JYPATH
env|grep --color -w PYTHIADIR
echo "=============================="

# checks
[ -z "$PYTHIADIR" ] && echo "ERROR: Pythia8 not found!" >&2
