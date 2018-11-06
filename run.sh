#!/bin/sh

source /share/apps/intel/parallel_studio_xe_2019/compilers_and_libraries_2019/linux/bin/compilervars.sh intel64

echo "Compiling..."

make

echo "Running the tests"

./bin/lab1

echo "You can now view timings.dat"
