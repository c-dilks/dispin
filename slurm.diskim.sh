#!/bin/bash
# builds diskim files, and subsequently, outroot files

if [ -z "$DISPIN_HOME" ]; then source env.sh; fi

if [ $# -ne 1 ];then echo "USAGE: $0 [train directory]"; exit; fi
traindir=$1


# build list of files, and cleanup outdat and outmon directories
jobsuffix=$(echo $traindir|sed 's/^.*\///g')
joblist=jobs.${jubsuffix}.slurm
echo "writing joblist to $joblist"
> $joblist
for skimfile in ${train}/skim*.hipo; do
  diskimfile="$(echo $skimfile|sed 's/^.*\///g').root"
  cmd="run-groovy skimDihadrons.groovy $skimfile"
  cmd="$cmd && calcKinematics.exe $diskimfile"
  cmd="$cmd && rm $diskimfile"
  echo $cmd >> $joblist
done


# write job descriptor
slurm=job.${jobsuffix}.slurm
> $slurm
rm -v /farm_out/`whoami`/clasqa*

function app { echo "$1" >> $slurm; }

app "#!/bin/bash"

app "#SBATCH --job-name=dispin_diskim"
app "#SBATCH --account=clas12"
app "#SBATCH --partition=production"

app "#SBATCH --mem-per-cpu=6000"
app "#SBATCH --time=6:00:00"

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
exit # exit before job submission
echo "submitting to slurm..."
sbatch $slurm
squeue -u `whoami`
