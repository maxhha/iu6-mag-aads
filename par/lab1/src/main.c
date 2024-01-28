#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "common.h"

#include "asm_scalar_prod.h"
#include "c_scalar_prod.h"

#define GET_RND ((double)rand() / (double)(RAND_MAX))

typedef void scalar_prod(double *, const dvec *, const dvec *, int);

t_aligned_fmatrix_array create_aligned_fmatrix_array(int size) {
    t_aligned_fmatrix_array s = (t_aligned_fmatrix_array) malloc(sizeof (s_aligned_fmatrix_array));
    if (s == NULL) {
        goto fail_malloc_s;
    }

    s->origin = (fmatrix *) malloc(sizeof (fmatrix) * (size) + 64);
    if (s->origin == NULL) {
        goto fail_malloc_origin;
    }

    s->aligned = (fmatrix *) ((int64_t) s->origin + ((64 - (((int64_t) s->origin) & 0x3F)) & 0x3F));

    return s;

fail_malloc_origin:
    free(s);

fail_malloc_s:

    return NULL;
}

void free_aligned_fmatrix_array(t_aligned_fmatrix_array s) {
    free(s->origin);
    free(s);
}

void measure_func(scalar_prod *dot)
{
    dvec a[ARRAY_SIZE], b[ARRAY_SIZE];

    for (int i = 0; i < ARRAY_SIZE; ++i)
    {
        a[i][0] = GET_RND;
        a[i][1] = GET_RND;
        a[i][2] = GET_RND;
        a[i][3] = GET_RND;

        b[i][0] = GET_RND;
        b[i][1] = GET_RND;
        b[i][2] = GET_RND;
        b[i][3] = GET_RND;
    }

    double res[ARRAY_SIZE];

    clock_t start = clock();

    for (int m = 0; m < MEASUREMENT_REPEATS; ++m)
        dot(res, a, b, ARRAY_SIZE);

    clock_t end = clock();

    printf("\t%.3g s\n", ((double)(end - start)) / CLOCKS_PER_SEC / MEASUREMENT_REPEATS);
}

int main(void)
{
    t_aligned_fmatrix_array a = create_aligned_fmatrix_array(2);
    if (a == NULL) {
        printf("null\n\n");
        return;
    }

    // printf("%p %p %p\n", a, a->origin, a->aligned);

    for (int i = 0; i < 2; i++) {
        printf("%c:\n", "AB"[i]);
        for (int j = 0; j < 16; j++) {
            scanf("%f", &a->aligned[i][j / 4][j % 4]);
        }
        printf("\n========\n");
    }

    fmatrix r;
    mul_matrixes_asm(&r, a->aligned, a->aligned + 1, 1);
    printf("A x B:\n");
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (j > 0) {
                printf(" ");
            }
            printf("%3.3f", r[i][j]);
        }
        printf("\n");
    }

    free_aligned_fmatrix_array(a);

    return EXIT_SUCCESS;
}
