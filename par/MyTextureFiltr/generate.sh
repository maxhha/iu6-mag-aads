#! /bin/env bash
(
    cd out;
    g++ -o a.out ../src/Codegen.cpp && \
        echo "[COMPILED]" && \
        ./a.out
)
