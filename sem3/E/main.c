#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SWAP(a, b) { long int t = (a); (a) = (b); (b) = t; }
// #define DEBUG

void merge_sort(long int *a, int l, int r) {
    if (r - l == 1) {
        return;
    }
    if (r - l == 2) {
        if (a[l] > a[r - 1]) {
            SWAP(a[l], a[r - 1])
        }
        return;
    }

    int m = (l + r) / 2;
    merge_sort(a, l, m);
    merge_sort(a, m, r);

    int res_i = l, l_i = l, r_i = m;

    /*
    ]3 4 5|1 2 6

    1] 4 5|3|2 6
    1 2] 5|3 4|6
    1 2 3]|5 4|6

    1 2 3 4] 5|6
    1 2 3 4 5 6
    */

    // #ifdef DEBUG
    // printf("-- res_i   l_i   r_i\n");
    // #endif


    while (l_i < r_i && r_i < r) {

        #ifdef DEBUG
        // printf("-- %-7d %-5d %-5d\n", res_i - l, l_i - l, r_i - l);
        printf("[");
        for (int i = l; i < r; i++) {
            if (i > l) {
                printf(" %c", i == res_i ? '|' : i == l_i ? '>' : i == r_i ? '>' : ' ');
            }
            printf("%ld", a[i]);
        }
        printf("]\n\n");
        #endif

        if (a[l_i] <= a[r_i]) {
            l_i++;
        } else {
            long int t = a[r_i];
            memmove((void *) &a[l_i + 1], (void *) &a[l_i], sizeof (long int) * (r_i - l_i));
            a[l_i] = t;
            r_i++;
            l_i++;
        }


        // if (a[l_i] > a[r_i]) {
        //     SWAP(a[res_i], a[r_i])
        //     if (res_i == l_i) {
        //         l_i = r_i;
        //     }
        //     r_i++;
        //     res_i++;
        // } else if (res_i == l_i) {
        //     l_i++;
        //     res_i++;
        // } else {
        //     long int t = a[l_i];
        //     memmove((void *) &a[res_i + 1], (void *) &a[res_i], sizeof (long int) * (l_i - res_i));
        //     a[res_i] = t;

        //     res_i++;
        //     l_i++;

        //     if (l_i == r_i) {
        //         l_i = res_i;
        //     }
        // }
    }

    // #ifdef DEBUG
    // // printf("-- %-7d %-5d %-5d\n", res_i - l, l_i - l, r_i - l);
    // printf("[");
    // for (int i = l; i < r; i++) {
    //     if (i > l) {
    //         printf(" %c", i == res_i ? '|' : i == l_i ? '>' : i == r_i ? '>' : ' ');
    //     }
    //     printf("%ld", a[i]);
    // }
    // printf("]\n\n");
    // #endif

    // if (l_i < r_i) {
    //     while (res_i < l_i) {
    //         #ifdef DEBUG
    //         // printf("-- %-7d %-5d %-5d\n", res_i - l, l_i - l, r_i - l);
    //         printf("[");
    //         for (int i = l; i < r; i++) {
    //             if (i > l) {
    //                 printf(" %c", i == res_i ? '|' : i == l_i ? '>' : i == r_i ? '>' : ' ');
    //             }
    //             printf("%ld", a[i]);
    //         }
    //         printf("]\n\n");
    //         #endif

    //         if (l_i + 1 < r_i) {
    //             SWAP(a[res_i], a[l_i])
    //             res_i++;
    //             l_i++;
    //         } else {
    //             SWAP(a[res_i], a[l_i])
    //             res_i++;
    //         }
    //     }
    // }

    // while (i < m) {
    //     buf[buf_i++] = a[i++];
    // }

    // while (j < r) {
    //     buf[buf_i++] = a[j++];
    // }

    // memcpy(a + l, buf, n * sizeof (long int));
}

int main(void) {
    int n;
    long int a[100000];

    scanf("%d", &n);

    for (int i = 0; i < n; i++) {
        scanf("%ld", &a[i]);
    }

    merge_sort(a, 0, n);

    for (int i = 0; i < n; i++) {
        if (i > 0) {
            printf(" ");
        }
        printf("%ld", a[i]);
    }
    printf("\n");

    return 0;
}
