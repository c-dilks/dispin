#!/bin/bash
# runs Orthogonality.C looping over final analysis bins

#BuildOrtho.exe -d outroot -o ortho.x.root -i1 &
#BuildOrtho.exe -d outroot -o ortho.m.root -i2 &
#BuildOrtho.exe -d outroot -o ortho.z.root -i3
#exit

for iv in x m z; do
  for b in {0..11}; do
    root -b -q Orthogonality.C'('$b',0,"'ortho.m.root'",true)'
  done
done
