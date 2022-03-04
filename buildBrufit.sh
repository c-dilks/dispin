#!/bin/bash
# build brufit (clean)
set -e
pushd deps/brufit
mkdir -p build
rm -r build
mkdir -p build
pushd build
cmake -DCMAKE_C_COMPILER=$(which gcc) ../
make install
cmake -DCMAKE_C_COMPILER=$(which gcc) ../
make install
echo """
build finished!

>>> now running a brufit tutorial to build remaining
    shared libraries (e.g. PDFExpand)...

"""
popd
pushd tutorials/SphHarmonic
root -b -q $BRUFIT/macros/LoadBru.C $* GenSphHarmonicMoments.C
popd
popd
echo "DONE"
