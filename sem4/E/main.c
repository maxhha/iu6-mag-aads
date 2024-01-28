#include <stdlib.h>
#include <stdio.h>

#define DATA_T long int
#define SWAP(a, b, T) { T t = a; a = b; b = t; }

void my_qsort(DATA_T *arr, int l, int r) {
    if (r - l <= 1) {
        return;
    }

    int x = arr[(l + r) / 2];
    int f_e = l;
    int f_b = l;

    for (int i = l; i < r; i++) {
        if (arr[i] < x) {
            SWAP(arr[i], arr[f_e], DATA_T)
            if (f_e != f_b) {
                SWAP(arr[i], arr[f_b], DATA_T)
            }
            f_e++;
            f_b++;
        } else if (arr[i] == x) {
            SWAP(arr[i], arr[f_b], DATA_T)
            f_b++;
        }
    }
    my_qsort(arr, l, f_e);
    my_qsort(arr, f_b, r);
}

int main(void) {
    long int *arr = (long int *) malloc(sizeof (long int) * 100000);
    int n;
    scanf("%d", &n);

    for (int i = 0; i < n; i++) {
        scanf("%ld", &arr[i]);
    }

    my_qsort(arr, 0, n);

    for (int i = 0; i < n; i++) {
        if (i > 0) {
            printf(" ");
        }
        printf("%ld", arr[i]);
    }

    printf("\n");
    free(arr);

    return 0;
}
