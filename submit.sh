#!/bin/sh
qsub -qmei -lnodes=1:ppn=4:k20,walltime=10:00 ./run.sh
