#!/bin/bash

export DISPIN_HOME=$(dirname $(realpath $0))

pushd deps

pushd j2root
source setup.sh
popd

pushd clasqaDB
source env.sh
popd

popd
