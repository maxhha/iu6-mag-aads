#pragma once

#define ARRAY_SIZE 1000
#define MEASUREMENT_REPEATS 100000

typedef double dvec[4];
typedef float fvec[4];
typedef fvec fmatrix[4];

typedef struct {
    fmatrix *origin;
    fmatrix *aligned;
} s_aligned_fmatrix_array;

typedef s_aligned_fmatrix_array *t_aligned_fmatrix_array;
