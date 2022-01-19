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
  echo """
  build finished
  IMPORTANT: run a brufit test fit at least once to build any remaining
             shared libraries (e.g. PDFExpand)
  """ && \
  popd && exit 0
popd
exit 1
