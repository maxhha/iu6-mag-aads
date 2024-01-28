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

    for (int i = n - 1; i > 0; i--) {
        for (int j = 0; j < i; j++) {
            if (arr[j] > arr[j+1]) {
                int t = arr[j+1];
                arr[j+1] = arr[j];
                arr[j] = t;
                swapped++;
            }
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
