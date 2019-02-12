#!/bin/bash
module load gcc/5.3.0
module load gnu/openmpi_eth/1.8.4
cd $HOME/CPD/PCP/MPI
make clean
make
mpirun -np 2 -mca btl self,sm,tcp bin/skeleton_mpi images/seahorse.ascii.pbm images/out_1.pbm
#python testing.py

