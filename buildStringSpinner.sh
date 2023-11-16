#!/bin/bash
# build StringSpinner (clean)
set -e

if [ $# -ne 2 ]; then
  echo """USAGE: $0 [PYTHIA_INSTALLATION] [NUM_CPUS]
  - [PYTHIA_INSTALLATION] should be the path to the Pythia 8 installation,
    - e.g., /usr if you used your distro's package manager
    - run \`locate libpythia\` to help find it
  """
  exit 2
fi
pythiaInstallation=$1
ncpu=$2
pushd deps/StringSpinner

# clean
echo "CLEANING..." # TODO
make clean

# build
./configure $pythiaInstallation
make dis

# finish
popd
echo "DONE"
