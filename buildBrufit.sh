#!/bin/bash
# build brufit (clean)
set -e

if [ -z "$BRUFIT" ]; then
  echo "ERROR: BRUFIT env var not set"
  exit 1
fi
pushd $BRUFIT

# clean
echo "CLEANING..."
rm -rfv build
rm -fv macros/*.so

# build main shared lib
mkdir -p build
pushd build
cmake -DCMAKE_C_COMPILER=$(which gcc) ../
make install
cmake -DCMAKE_C_COMPILER=$(which gcc) ../
make install
popd

# build aux shared lib(s)
echo """
build finished!

>>> now running a brufit tutorial to build remaining
    shared libraries (e.g. PDFExpand)...

"""
pushd tutorials/SphHarmonic
root -b -q $BRUFIT/macros/LoadBru.C $* GenSphHarmonicMoments.C
popd

# finish
popd
echo "DONE"
