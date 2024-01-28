#include <stdio.h>
#include <stdlib.h>

int main(void) {
    int n;
    int arr[10000];

    scanf("%d", &n);
    if (n <= 0) {
        printf("\n0\n");
        return 0;
    }

    for (int i = 0; i < n; i++) {
        scanf("%d", &arr[i]);
    }

    int swapped = 0;

    for (int i = 0; i < n - 1; i++) {
        int min_i = i;
        int j;
        for (j = i + 1; j < n; j++) {
            if (arr[min_i] > arr[j]) {
                min_i = j;
            }
        }

        if (min_i != i) {
            int t = arr[min_i];
            arr[min_i] = arr[i];
            arr[i] = t;
            swapped++;
        }
    }

    for (int i = 0; i < n; i++) {
        if (i > 0) {
            printf(" ");
        }
        printf("%d", arr[i]);
    }
    printf("\n%d\n", swapped);

    return 0;
}
