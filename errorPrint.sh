#!/bin/bash
grep --color -vE '█|═|Physics Division|^     $|^Info in|^Opening connections to workers|^Setting up worker servers|^\[TProof::Progress|^ Query processing time|merging output objects|all output objects have been merged|has no streamer or dictionary.*will not be saved' /farm_out/`whoami`/*.err
