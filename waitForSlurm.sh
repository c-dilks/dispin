#!/bin/bash
# runs squeue in a loop which does not exit until no jobs
# remain in the slurm queue

while [ 1 ]; do
  n=$(squeue -r -u $(whoami) | wc -l)
  let n--
  if [ $n -gt 0 ]; then echo "$n slurm jobs remain..."; sleep 30
  else echo "slurm done"; break
  fi
done
