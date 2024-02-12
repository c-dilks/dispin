#!/bin/bash
# builds diskim files, and subsequently, outroot files

set -e
set -u

if [ $# -lt 1 ];then
  echo "USAGE: $0 [dataset source name]"
  set +e
  getDatasetSourceInfo.rb
  set -e
  echo ""
  echo "Alternatively,"
  echo "   $0 [train directory] [outroot dir] [optional:data/mcrec/mcgen] [optional:skim/dst]"
  echo "      - append \"rad\" to the end of datastream to re-run calcKinematics with RC beam model"
  exit 2
fi

# use `getDatasetSourceInfo.rb` to get info
if [ $# -eq 1 ]; then
  datasetSource=$1
  traindir=$(getDatasetSourceInfo.rb $datasetSource 'source')
  outrootdir=outroot.$(getDatasetSourceInfo.rb $datasetSource 'dataset')
  datastream=$(getDatasetSourceInfo.rb $datasetSource 'stream')
  hipotype="skim"
# otherwise, use user-specified info
else
  traindir=$1
  outrootdir=$2
  datastream="data"
  hipotype="skim"
  if [ $# -ge 3 ]; then datastream="$3"; fi
  if [ $# -ge 4 ]; then hipotype="$4"; fi
fi

# check setup
if [ -z "${DISPIN_HOME-}" ]; then
  echo "ERROR: you must source env.sh first"; exit 1
fi
if [ ! -d "diskim" ]; then
  echo "ERROR: you must create or link a diskim directory"; exit 1
fi
if [ ! -d $outrootdir ]; then
  echo "ERROR: you must create or link an outroot directory"
  if [[ "`hostname`" =~ "ifarm" ]]; then
    volDir=/volatile/clas12/users/$LOGNAME/$outrootdir
    echo """Suggestion: you are on ifarm, use /volatile by running:

mkdir -p $volDir && ln -sv $volDir ./
    """
  fi
  exit 1
fi
echo "check if $outrootdir directory is empty..."
if [ -z "$(ls $outrootdir/*.root)" ]; then
  echo "... yes"
else
  echo "... no"
  echo "ERROR: you must empty $outrootdir directory"
  exit 1
fi

# check if hipo files are cached
if [ "$hipotype" == "skim" ]; then
  checkIfCached.sh $traindir
  if [ -s tmp/difflist ]; then
    echo "Stopping $0 execution. (comment out to override this check)"
    exit 1
  fi
fi


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
  exit 1
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
app "#SBATCH --time=24:00:00"

app "#SBATCH --array=1-$(cat $joblist | wc -l)"
app "#SBATCH --ntasks=1"

app "#SBATCH --output=/farm_out/%u/%x-%A_%a.out"
app "#SBATCH --error=/farm_out/%u/%x-%A_%a.err"

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
