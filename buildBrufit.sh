#!/bin/bash
# build brufit (clean)
pushd deps/brufit
mkdir -p build
rm -r build
mkdir -p build
cd build
cmake -DCMAKE_C_COMPILER=$(which gcc) ../ && \
  make install && \
  cmake -DCMAKE_C_COMPILER=$(which gcc) ../ && \
  make install && \
  popd && exit 0
popd
exit 1
