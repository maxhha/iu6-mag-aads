#! /bin/env bash
(
    cd out;
    g++ -mavx2 -O3 -pipe -fopenmp -lpthread -lm -o a.out ../src/MU16Data.cpp ../src/TextureFiltr.cpp && \
        echo "[COMPILED]" && \
        ./a.out
)

# -msse4.2
