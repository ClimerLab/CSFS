#! /bin/bash

#SBATCH -p general # use the general partition
#SBATCH -J compile_csfs # job name
#SBATCH -o results-%j.out # output file name
#SBATCH -t 0-00:10 # time limit

#SBATCH -N 1 # number of nodes
#SBATCH -n 1 # number of cores (AKA tasks)

#SBATCH --mail-user=kpsc59@mail.umsl.edu  # email address for notifications
#SBATCH --mail-type=BEGIN,END,FAIL         # which type of notifications to send

# load modules
module load openmpi/4.1.5_gcc_12.3.0
module load cplex/2211

module list

# Comple program
srun make
