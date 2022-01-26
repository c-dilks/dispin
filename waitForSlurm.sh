#!/bin/bash
# runs squeue in a loop which does not exit until no jobs
# remain in the slurm queue
# - assume optional additional args is job name(s)

while [ 1 ]; do
  if [ $# -gt 0 ]; then
    n=$(squeue -r -u $(whoami) -n $* | wc -l)
  else
    n=$(squeue -r -u $(whoami) | wc -l)
  fi
  let n--
  if [ $n -gt 0 ]; then echo "$n slurm jobs remain..."; sleep 30
  else echo "slurm done"; break
  fi
done
