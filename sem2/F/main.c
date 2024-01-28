#include <stdio.h>
#include <stdlib.h>

int main(void) {
    int heap[100000];
    int heap_size = 0;

    int n;
    scanf("%d", &n);

    for (int j = 0; j < n; j++) {
        scanf("%d", &heap[j]);

         for (int i = j; i > 0 && heap[(i - 1) / 2] > heap[i]; i = (i - 1) / 2) {
            int t = heap[i];
            heap[i] = heap[(i - 1) / 2];
            heap[(i - 1) / 2] = t;
        }
    }

    for (int j = 0; j < n; j++) {
        if (j > 0) {
            printf(" ");
        }

        printf("%d", heap[0]);
        heap_size = n - j - 1;
        heap[0] = heap[heap_size];

        int i = 0, ai, bi;

        while ((ai = i * 2 + 1) < heap_size) {
            bi = ai + 1;

            if (heap[i] < heap[ai] && (bi >= heap_size || heap[i] < heap[bi])) {
                break;
            }

            int ni = heap[ai] < heap[bi] ? ai : bi;

            int t = heap[ni];
            heap[ni] = heap[i];
            heap[i] = t;

            i = ni;
        }
    }

    printf("\n");

    return 0;
}
