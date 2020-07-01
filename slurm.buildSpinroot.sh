#!/bin/bash

# default args for crosscheck: -i2 -t2 -l1 -m1 -b

outrootdir="outroot"

exe="buildSpinroot"
if [ $# -eq 0 ]; then
  echo "usage: $0 [OPTIONS]"
  ./${exe}.exe 2>/dev/null | grep OPTIONS -A100 --color
  exit
fi
args=$*

rm -vf spinroot/*.root


slurm=job.${exe}.slurm
> $slurm

function app { echo "$1" >> $slurm; }

nruns=$(ls ${outrootdir}/*.root | wc -l)
let nruns--

app "#!/bin/bash"
app "#SBATCH --job-name=dispin_${exe}"
app "#SBATCH --account=clas12"
app "#SBATCH --partition=production"
app "#SBATCH --mem-per-cpu=500"
app "#SBATCH --time=2:00:00"
app "#SBATCH --array=0-${nruns}"
app "#SBATCH --ntasks=1"
app "#SBATCH --output=/farm_out/%u/%x-%j-%N.out"
app "#SBATCH --error=/farm_out/%u/%x-%j-%N.err"
app "dataList=($(pwd)/${outrootdir}/*.root)"

app "srun ${exe}.exe -f \${dataList[\$SLURM_ARRAY_TASK_ID]} $args"

echo "job script"
printf '%70s\n' | tr ' ' -
cat $slurm
printf '%70s\n' | tr ' ' -
echo "submitting to slurm..."
sbatch $slurm
squeue -u `whoami`
