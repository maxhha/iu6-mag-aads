#!/bin/env bash
gcc -std=gnu11 -lpthread main.c && echo "[COMPILED]" && ./a.out
