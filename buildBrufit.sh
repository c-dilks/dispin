#!/bin/bash
# build brufit (clean)
pushd deps/brufit
mkdir -p build
rm -r build
mkdir -p build
cd build
cmake ../ && make install && cmake ../ && make install && popd && exit 0
popd
exit 1
