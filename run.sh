#!/bin/bash
module load gcc/5.3.0
cd $HOME/CPD/PCP/Trabalho
make clean
make
./bin/skeleton_seq images/${file}.pbm
