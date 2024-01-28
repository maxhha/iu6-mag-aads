#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SWAP(a, b, T) { T t = a; a = b; b = t; }

int main(void) {
    int n;
    scanf("%d", &n);

    long int heap[100000];
    int heap_i = 0;

    for (int i = 0 ; i < n; i++) {
        int c;
        scanf("%d", &c);
        if (c == 0) {
            long int v;
            scanf("%ld", &v);
            heap[heap_i] = v;
            int j = heap_i;
            heap_i++;

            for (int pj = (j - 1) / 2; j > 0 && heap[pj] < heap[j]; j = pj, pj = (j - 1) / 2) {
                SWAP(heap[pj], heap[j], long int)
            }
        } else {
            printf("%ld\n", heap[0]);
            heap_i--;
            SWAP(heap[heap_i], heap[0], long int)

            int j = 0;
            while (j < heap_i) {
                int a = j*2 + 1;
                int b = j*2 + 2;

                if ((a >= heap_i || heap[j] >= heap[a]) && (b >= heap_i || heap[j] >= heap[b])) {
                    break;
                }

                int swap_i = b >= heap_i ? a : heap[a] >= heap[b] ? a : b;
                SWAP(heap[j], heap[swap_i], long int)
                j = swap_i;
            }
        }
        // printf("\n> ----%d--- \n>", c);
        // for (int k = 0, kl = 0; k < heap_i; k++) {
        //     printf("%4ld ", heap[k]);
        //     if (k >= kl) {
        //         kl = kl * 2 + 2;
        //         printf("\n>");
        //     }
        // }
        // printf("\n");
    }

    return 0;
}

/*
/---v/----v-v
0 1 2 3 4 5 6
\-^\--^-^-^

i * 2 + 1
i * 2 + 2

3 -> 7 8
4 ->


       1
    9     8
   3 4   2 0

*/
