#!/bin/bash

export DISPIN_HOME=$(dirname $(realpath $0))
export BRUFIT=${DISPIN_HOME}/deps/brufit
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${BRUFIT}/lib

pushd deps

pushd j2root
source setup.sh
popd

pushd clasqaDB
source env.sh
popd

popd

env|grep --color -w DISPIN_HOME
env|grep --color -w BRUFIT
env|grep --color -w LD_LIBRARY_PATH
env|grep --color -w JYPATH
