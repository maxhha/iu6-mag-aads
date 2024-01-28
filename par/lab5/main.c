#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <omp.h>
#include <math.h>

// #define DEBUG

void mul_matrix(const double *a, const double *b, double *r, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            for (int k = 0; k < n; k++) {
                r[i * n + j] += a[i * n + k] * b[k * n + j];
            }
        }
    }
}

/*

1 2 3
4 5 6
7 8 9

1 2 3
4 5 6
7 8 9

2x2 = 8




*/

void block_mul_matrix(const double *a, const double *b, double *r, int n, int parts_n, int num_threads) {
    memset(r, 0, sizeof(double) * n * n);
    int parts_n3 = parts_n * parts_n * parts_n;

    #ifdef DEBUG
    printf("pt\tm\tb_i\tb_j\tb_k\ts_i\ts_j\ts_k\n");
    #endif

    #pragma omp parallel num_threads(num_threads)
    {
        #pragma omp for
        for (int magic = 0; magic < parts_n3; magic++) {
            int b_k = magic % parts_n;
            int b_j = (magic / parts_n) % parts_n;
            int b_i = (magic / parts_n / parts_n) % parts_n;

            int s_i = b_i * n / parts_n, f_i = (b_i + 1) * n / parts_n;
            int s_j = b_j * n / parts_n, f_j = (b_j + 1) * n / parts_n;
            int s_k = b_k * n / parts_n, f_k = (b_k + 1) * n / parts_n;

            #ifdef DEBUG
            printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", omp_get_thread_num(), magic, b_i, b_j, b_k, s_i, s_j, s_k);
            #endif

            for (int i = s_i; i < f_i; i++) {
                for (int j = s_j; j < f_j; j++) {
                    double d = 0;
                    for (int k = s_k; k < f_k; k++) {
                        d += a[i * n + k] * b[k * n + j];
                    }
                    r[i * n + j] += d;
                }
            }
        }
    }
}

void opt_block_mul_matrix(const double *a, const double *b, double *r, int n, int parts_n, int num_threads) {
    memset(r, 0, sizeof(double) * n * n);
    int parts_n3 = parts_n * parts_n * parts_n;
    int part_size = n / parts_n;

    #ifdef DEBUG
    printf("pt\tm\tb_i\tb_j\tb_k\ts_i\ts_j\ts_k\n");
    #endif

    double *bT = (double *) malloc(n * n * sizeof(double));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            bT[i * n + j] = b[j * n + i];
        }
    }

    #pragma omp parallel num_threads(num_threads)
    {
        #pragma omp for
        for (int magic = 0; magic < parts_n3; magic++) {
            // div_t d1 = div(magic, parts_n);
            // #define b_k d1.rem
            // div_t d2 = div(d1.quot, parts_n);
            // #define b_j d2.rem
            // int b_i = d2.quot % parts_n;
            int b_k = magic % parts_n;
            int d1 = magic / parts_n;
            int b_j = d1 % parts_n;
            int b_i = (d1 / parts_n) % parts_n;

            int s_i = b_i * part_size, f_i = s_i + part_size;
            int s_j = b_j * part_size, f_j = s_j + part_size;
            int s_k = b_k * part_size, f_k = s_k + part_size;

            #ifdef DEBUG
            // printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", omp_get_thread_num(), magic, b_i, b_j, b_k, s_i, s_j, s_k);
            #endif

            for (int i = s_i; i < f_i; i++) {
                const double *a_i = a + i * n;
                for (int j = s_j; j < f_j; j++) {
                    const double *bT_j = bT + j * n;

                    double d = 0;
                    for (int k = s_k; k < f_k; k++) {
                        d += a_i[k] * bT_j[k];
                    }
                    r[i * n + j] += d;
                }
            }
        }
    }
    free(bT);
}

int is_matrix_equal(const double *a, const double *b, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (fabs(a[i * n + j] - b[i * n + j]) > 1e-3) {
                return 0;
            }
        }
    }
    return 1;
}

int main() {
    #ifdef DEBUG

    double a[16] = {
        1, 2, 3, 4,
        5, 6, 7, 8,
        9, 10, 11, 12, 13
    };
    double b[16] = {
        1, -1, 1, 1,
        -1, 1, -1, 1,
        1, -1, 3, 1,
        -1, 1, 1, 1
    };
    double r[16] = {0};
    block_mul_matrix(a, b, r, 4, 2, 4);

    printf("\n");
    printf("block_mul_matrix:\n");
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            printf("%5.1lf ", r[i*4 + j]);
        }
        printf("\n");
    }

    double r2[16] = {0};
    mul_matrix(a, b, r2, 4);

    printf("\n");
    printf("mul_matrix:\n");
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            printf("%5.1lf ", r2[i*4 + j]);
        }
        printf("\n");
    }
    #else

    // #define N 500
    #define N 2000

    double *a = (double *) malloc(N*N*sizeof(double));
    double *b = (double *) malloc(N*N*sizeof(double));
    double *r = (double *) malloc(N*N*sizeof(double));
    double *r2 = (double *) malloc(N*N*sizeof(double));
    #define PARTS_N 4
    int parts_n[PARTS_N] = {2, 5, 10, 50};
    #define THREADS_N 4
    int num_threads[THREADS_N] = {2, 4, 8, 16};

    for (int i = 0; i < N*N; i++) {
        a[i] = i % 31;
        b[i] = i % 127 - 61;
    }

    memset(r, 0, sizeof(double) * N * N);
    printf("A[%dx%d] B[%dx%d]\n", N, N, N, N);

    double wt_start = omp_get_wtime();
    mul_matrix(a, b, r, N);
    double wt_end = omp_get_wtime();

    printf("\n");
    printf("mul_matrix: %.4lf\n", wt_end - wt_start);

    printf("\n");
    printf("block_mul_matrix\n");
    printf("num_threads  parts_n  time    correct?\n");

    for (int t_i = 0; t_i < THREADS_N; t_i++) {
        for (int parts_n_i = 0; parts_n_i < PARTS_N; parts_n_i++) {
            if (N % parts_n[parts_n_i]) {
                continue;
            }

            memset(r2, 0, sizeof(double) * N * N);
            wt_start = omp_get_wtime();
            block_mul_matrix(a, b, r2, N, parts_n[parts_n_i], num_threads[t_i]);
            wt_end = omp_get_wtime();
            printf("%-11d  %-7d  %.4lf    %s\n", num_threads[t_i], parts_n[parts_n_i], wt_end - wt_start, is_matrix_equal(r, r2, N) ? " +" : "not");
        }
    }

    printf("\n");
    printf("opt_block_mul_matrix\n");
    printf("num_threads  parts_n  time    correct?\n");

    for (int t_i = 0; t_i < THREADS_N; t_i++) {
        for (int parts_n_i = 0; parts_n_i < PARTS_N; parts_n_i++) {
            if (N % parts_n[parts_n_i]) {
                continue;
            }

            memset(r2, 0, sizeof(double) * N * N);
            wt_start = omp_get_wtime();
            opt_block_mul_matrix(a, b, r2, N, parts_n[parts_n_i], num_threads[t_i]);
            wt_end = omp_get_wtime();
            printf("%-11d  %-7d  %.4lf    %s\n", num_threads[t_i], parts_n[parts_n_i], wt_end - wt_start, is_matrix_equal(r, r2, N) ? " +" : "not");
        }
    }

    #endif


    return 0;
}
