#pragma once

#include "common.h"

void dot_prod_vector_arrays_asm(double *res, const dvec *a, const dvec *b, int n);
void mul_matrixes_asm(fmatrix *res, const fmatrix *a, const fmatrix *b, int n);
