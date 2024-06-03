#!/bin/bash

# software paths
export DISPIN_HOME=$(dirname $(realpath $0))
export BRUFIT=${DISPIN_HOME}/deps/brufit
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${BRUFIT}/lib

# kludge to set JAVA_HOME, which is only needed for j2root
[ -z "$JAVA_HOME" ] && export JAVA_HOME=$(realpath $(dirname $(realpath $(which java)))/..)

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
  export PYTHIADIR=$(realpath $(dirname $(which $pythia_search))/..) &&
  export STRINGSPINNER=${DISPIN_HOME}/deps/StringSpinner

# set python environment
if type virtualenvwrapper.sh > /dev/null; then
  echo "Found virtualenvwrapper; calling 'workon dispin'"
  workon dispin
else
  echo "WARNING: virtualenvwrapper not found; either install it or make your own virtual environment in your preferred way"
fi

# print results
echo ""
echo "===== DISPIN ENVIRONMENT ====="
env|grep --color -w DISPIN_HOME
env|grep --color -w BRUFIT
env|grep --color -w LD_LIBRARY_PATH
env|grep --color -w JAVA_HOME
env|grep --color -w JYPATH
env|grep --color -w PYTHIADIR
env|grep --color -w STRINGSPINNER
echo "=============================="

# checks
[ -z "$PYTHIADIR" ] && echo "WARNING: Pythia8 not found!" >&2
