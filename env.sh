#!/bin/bash

pushd deps/j2root
source setup.sh
popd

CLASSPATH="${CLASSPATH}:${PWD}/deps/j2root/src/main/java/*"
export CLASSPATH=$(echo $CLASSPATH | sed 's/^://')
