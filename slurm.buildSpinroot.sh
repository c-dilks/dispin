#!/bin/bash

# default args for crosscheck: -i2 -t2 -l1 -m1 -b

exe="buildSpinroot"
if [ $# -eq 0 ]; then
  echo "usage: $0 [ARGUMENTS]"
  echo ""
  echo "the first argument must be the directory of outroot files to analyze;"
  echo "the remaining arguments can be any of the options listed below:"
  echo ""
  ./${exe}.exe 2>/dev/null | grep OPTIONS -A100 --color
  exit
fi

args=$*
indir=`echo $args|awk '{print $1}'`
opts=`echo $args|sed 's/[^ ]* *//'`

rm -vf spinroot/*.root


slurm=job.${exe}.slurm
> $slurm

function app { echo "$1" >> $slurm; }

nruns=$(ls ${indir}/*.root | wc -l)
let nruns--

app "#!/bin/bash"
app "#SBATCH --job-name=dispin_${exe}"
app "#SBATCH --account=clas12"
app "#SBATCH --partition=production"
app "#SBATCH --mem-per-cpu=500"
app "#SBATCH --time=30:00"
app "#SBATCH --requeue"
app "#SBATCH --array=0-${nruns}"
app "#SBATCH --ntasks=1"
app "#SBATCH --output=/farm_out/%u/%x-%A_%a.out"
app "#SBATCH --error=/farm_out/%u/%x-%A_%a.err"
app "dataList=($(pwd)/${indir}/*.root)"

app "srun ${exe}.exe -f \${dataList[\$SLURM_ARRAY_TASK_ID]} $opts"

echo "job script"
printf '%70s\n' | tr ' ' -
cat $slurm
printf '%70s\n' | tr ' ' -
echo "submitting to slurm..."
sbatch $slurm
squeue -u `whoami`
