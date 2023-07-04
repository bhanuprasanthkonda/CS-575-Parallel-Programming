#!/bin/csh

echo "ArrSize,NON-SIMD Mul,SIMD,MULT,NON-SIMD Mul Sum,SIMD Mul Sum,MULT SUM"
foreach ArrSize (1024 2048 4096 8192 16384 32768 65536 131072 262144)
    g++ -DARRAYSIZE=$ArrSize project4.cpp -o project4 -lm -fopenmp
    ./project4
end