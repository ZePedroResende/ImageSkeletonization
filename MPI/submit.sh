#!/bin/sh
qsub -v file=$1 -qmei -lnodes=2:ppn=48:r662,walltime=10:00 ./run.sh 
