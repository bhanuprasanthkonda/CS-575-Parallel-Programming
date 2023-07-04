#!/bin/bash


echo "processors,elements,mega-autocorrelations computed per second"

for b in 4 6 8 10 12 14 16 
do
    module load openmpi
    mpic++ main.cpp -o main -lm
    mpiexec -mca btl self,tcp -np $b ./main
done