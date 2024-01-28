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

    int not_changed = 0;

    for (int i = 1; i < n; i++) {
        int j;
        for (j = i; j > 0 && arr[j - 1] > arr[j]; j--) {
            int t = arr[j - 1];
            arr[j - 1] = arr[j];
            arr[j] = t;
        }
        not_changed += i == j;
    }

    for (int i = 0; i < n; i++) {
        if (i > 0) {
            printf(" ");
        }
        printf("%d", arr[i]);
    }
    printf("\n%d\n", not_changed);

    return 0;
}
