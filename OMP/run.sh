#!/bin/bash
module load gcc/5.3.0
cd $HOME/dev/PCP/
make clean && make
./bin/skeleton_seq haskell.pgm
