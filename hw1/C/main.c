#include <stdio.h>
#include <stdlib.h>

// #define DEBUG

#define SWAP(a, b, T) { T t = a; a = b; b = t; }

void my_qsort(long int *a, long int l, long int r) {
    #ifdef DEBUG
    // printf("my_qsort(%ld, %ld)\n", l, r);

    // for (long int i = l +1; i < r; i++) {
    //     printf("%ld ", a[i]);
    // }
    // printf("\n");
    #endif
    if (r - l <= 2) {
        return;
    }

    long int a_m = a[(l + r) / 2];
    // printf("a_m = %d\n", a_m);
    long int l_i, e_i = l_i = l + 1;
    for (long int i = l + 1; i < r; i++) {
        if (a[i] < a_m) {
            if (l_i == i) {
                l_i++;
                e_i++;
            } else if (l_i == e_i) {
                SWAP(a[l_i], a[i], long int)
                l_i++;
                e_i++;
            } else {
                SWAP(a[l_i], a[i], long int)
                SWAP(a[e_i], a[i], long int)
                l_i++;
                e_i++;
            }
        } else if (a[i] == a_m) {
            if (e_i == i) {
                e_i++;
            } else {
                SWAP(a[e_i], a[i], long int)
                e_i++;
            }
        }
    }
    my_qsort(a, l, l_i);
    my_qsort(a, e_i - 1, r);
}

int main(void) {
    long int n, l;
    scanf("%ld%ld", &n, &l);
    long int *a = (long int *) malloc(sizeof(long int) * (n + 1));

    for (long int i = 0; i < n; i++) {
        scanf("%ld", &a[i]);
    }

    my_qsort(a, -1, n);

    #ifdef DEBUG
    for (long int i = 0; i < n; i++) {
        printf("%ld ", a[i]);
    }
    printf("\n");
    #endif

    long int m = a[0];
    if (l - a[n - 1] > m) {
        m = l - a[n - 1];
    }
    m *= 2;
    for (long int i = 1; i < n; i++) {
        long int d = a[i] - a[i - 1];
        if (d > m) {
            m = d;
        }
    }

    printf("%.1f\n", ((double) m) / 2.0);

    free(a);

    return 0;
}
