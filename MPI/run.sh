#!/bin/bash
module load gnu/openmpi-eth/1.8.4
cd $HOME/CPD/PCP/MPI
make clean
make
mpirun -np 48 -mca btl self,sm,tcp /bin/skeleton_mpi images/${file}.pbm
