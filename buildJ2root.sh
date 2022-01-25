#!/bin/bash
# build j2root (clean)
echo """

IMPORTANT: currently running with versions
- jdk/1.8.0_31
- python/2.7.18 - only needed to run scons (build configuration)

"""
set -e
pushd deps/j2root
mkdir -p build
mkdir -p target
rm -r build
rm -r target
scons
mvn package
popd
