#!/bin/bash
# build j2root (clean)
echo """

IMPORTANT: you may need to convince your system to use Python 2
- if on ifarm: module switch python/2...

"""
set -e
pushd deps/j2root
mkdir -p build
mkdir -p target
rm -r build
rm -r target
mvn package
scons build/native
scons
popd
