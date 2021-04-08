#!/bin/bash
# builds diskim files, and subsequently, outroot files


if [ $# -lt 2 ];then
  echo "USAGE: $0 [train directory] [outroot dir] [optional:data/mcrec/mcgen] [optional:skim/dst]"
  exit
fi
traindir=$1
outrootdir=$2
datastream="data"
hipotype="skim"
if [ $# -ge 2 ]; then datastream="$2"; fi
if [ $# -ge 3 ]; then hipotype="$3"; fi

# check setup
if [ -z "$DISPIN_HOME" ]; then
  echo "ERROR: you must source env.sh first"; exit
fi
if [ ! -d "diskim" ]; then
  echo "ERROR: you must create or link a diskim directory"; exit;
fi
if [ ! -d "outroot" ]; then
  echo "ERROR: you must create or link an outroot directory"; exit;
fi

# cleanup
rm -vf diskim/*.root
rm -vf outroot/*.root


# build list of files to process
jobsuffix=$(echo $traindir|sed 's/\/$//'|sed 's/^.*\///g')
joblist=jobs.${jobsuffix}.slurm
> $joblist
if [ "$hipotype" == "skim" ]; then
  for skimfile in ${traindir}/*.hipo; do
    echo "./runDiskim.sh $skimfile $outrootdir $datastream $hipotype" >> $joblist
  done
elif [ "$hipotype" == "dst" ]; then
  for rundir in `ls -d ${traindir}/*/ | sed 's/\/$//'`; do
    echo "./runDiskim.sh $rundir $outrootdir $datastream $hipotype" >> $joblist
  done
else
  echo "ERROR: unknown hipotype"
  exit
fi



# write job descriptor
slurm=job.${jobsuffix}.slurm
> $slurm

function app { echo "$1" >> $slurm; }

app "#!/bin/bash"

app "#SBATCH --job-name=dispin_diskim"
app "#SBATCH --account=clas12"
app "#SBATCH --partition=production"

app "#SBATCH --mem-per-cpu=2000"
app "#SBATCH --time=16:00:00"

app "#SBATCH --array=1-$(cat $joblist | wc -l)"
app "#SBATCH --ntasks=1"

app "#SBATCH --output=/farm_out/%u/%x-%j-%N.out"
app "#SBATCH --error=/farm_out/%u/%x-%j-%N.err"

app "srun \$(head -n\$SLURM_ARRAY_TASK_ID $joblist | tail -n1)"


# launch jobs
printf '%70s\n' | tr ' ' -
echo "JOB LIST: $joblist"
cat $joblist
printf '%70s\n' | tr ' ' -
echo "JOB DESCRIPTOR: $slurm"
cat $slurm
printf '%70s\n' | tr ' ' -
#exit # exit before job submission (for testing)
echo "submitting to slurm..."
sbatch $slurm
squeue -u `whoami`
