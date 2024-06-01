#!/bin/bash
# build brufit (clean)
set -e

if [ -z "$BRUFIT" ]; then
  echo "ERROR: BRUFIT env var not set"
  exit 1
fi
if [ $# -ne 1 ]; then
  echo "USAGE: $0 [NUM_CPUS]"
  exit 2
fi
ncpu=$1
pushd $BRUFIT

# clean
echo "CLEANING..."
rm -rfv build
rm -fv macros/*.so

# build main shared lib
unset CMAKE_GENERATOR
mkdir -p build
pushd build
cmake -DCMAKE_C_COMPILER=$(which gcc) ../
make -j$ncpu install
cmake -DCMAKE_C_COMPILER=$(which gcc) ../
make -j$ncpu install
popd

# build aux shared lib(s)
echo """
build finished!

>>> now running a brufit tutorial to build remaining
    shared libraries (e.g. PDFExpand)...
"""
pushd tutorials/SphHarmonic
root -b -q $BRUFIT/macros/LoadBru.C GenSphHarmonicMoments.C > $DISPIN_HOME/tmp/build.log
echo """

>>> done, see tmp/build.log for stdout

"""
popd

# finish
popd
echo "DONE"
