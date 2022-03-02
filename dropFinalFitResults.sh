#!/bin/bash
# put fit results in a tarball, so we can scp it to a local computer, 
# rather than trying to view them over ssh

if [ $# -lt 1 ]; then
  echo "USAGE: $0 [path]"
  echo "path will be globbed, as in [path]*/asym*.root"
  exit 2
fi
tar czvf ~/drop/t.tar.gz ${1}*/asym*.root
