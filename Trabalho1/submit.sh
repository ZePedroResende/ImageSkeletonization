#!/bin/sh
qsub -v file=$1 -qmei -lnodes=1:ppn=32,walltime=10:00 ./run.sh 
