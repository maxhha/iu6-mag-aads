#! /bin/env bash
gcc -O3 -fopenmp main.c && echo "[COMPILED]" && ./a.out
