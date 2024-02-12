#!/bin/bash
# build StringSpinner (clean)
set -e

if [ $# -ne 1 ]; then
  echo """USAGE: $0 [NUM_CPUS]
  """
  exit 2
fi
ncpu=$1
[ -z "$PYTHIADIR" ] && echo "ERROR: source env.sh" >&2 && exit 1
pushd deps/StringSpinner

# clean
echo "CLEANING..."
make clean

# hack: use a modified Makefile for dispin compatibility
if [ ! -f Makefile.ignore ]; then
  mv -v Makefile{,.ignore}
  ln -sv ../Makefile.StringSpinner Makefile
fi
ls -l Makefile

# build
./configure ${PYTHIADIR}
make dis

# hack: revert to the original Makefile
mv -v Makefile{.ignore,}
ls -l Makefile

# finish
popd
echo "DONE"
