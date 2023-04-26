#!/bin/bash
grep -HE '.*' /farm_out/`whoami`/*.err |\
grep -vE '█' |\
grep -vE '═' |\
grep -vE 'Physics Division' |\
grep -vE '\.err:     $' |\
grep -vE '\.err:Info in' |\
grep -vE '\.err:Opening connections to workers' |\
grep -vE '\.err:Setting up worker servers' |\
grep -vE '\.err:\[TProof::Progress' |\
grep -vE '\.err: Query processing time' |\
grep -vE 'merging output objects' |\
grep -vE 'all output objects have been merged' |\
grep -vE 'has no streamer or dictionary.*will not be saved' |\
grep -vE 'Corner (Full |)Plot is already in the list' |\
grep -vE 'Autocorrelation Plot is already in the list' |\
grep -vE 'unknown branch -> Spin_idx' |\
grep -vE 'DataSourceDump.*opened file with events' |\
grep --color -E '^.*\.err:'
