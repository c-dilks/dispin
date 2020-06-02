#!/bin/bash

pushd deps

pushd j2root
source setup.sh
popd

pushd clasqaDB
source env.sh
popd

popd
