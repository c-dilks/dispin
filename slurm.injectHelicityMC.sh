#!/bin/bash

exe="injectHelicityMC"
if [ $# -eq 0 ]; then
  echo "usage: $0 [outroot directory] [mcroot directory]"
  exit
fi

args=$*
indir=`echo $args|awk '{print $1}'`
opts=`echo $args|sed 's/[^ ]* *//'`

rm -vf mcroot/*.root

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
app "#SBATCH --time=2:00:00"
app "#SBATCH --array=0-${nruns}"
app "#SBATCH --ntasks=1"
app "#SBATCH --output=/farm_out/%u/%x-%A_%a.out"
app "#SBATCH --error=/farm_out/%u/%x-%A_%a.err"
app "dataList=($(pwd)/${indir}/*.root)"

app "srun ${exe}.exe \${dataList[\$SLURM_ARRAY_TASK_ID]} $opts"

echo "job script"
printf '%70s\n' | tr ' ' -
cat $slurm
printf '%70s\n' | tr ' ' -
echo "submitting to slurm..."
sbatch $slurm
squeue -u `whoami`
