#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #define DEBUG

/*
   -2 -1  0  1  2
-2
-1
 0        K
 1
 2

*/

#define SWAP(a, b, T) { T t = (a); (a) = (b); (b) = t;}
#define min(a, b) ((a) > (b) ? (b) : (a))
#define max(a, b) ((a) < (b) ? (b) : (a))

int main(void) {
    int n, m;
    scanf("%d%d", &n, &m);

    unsigned long arr[n * m];
    memset(arr, 0, n * m * sizeof(unsigned long));
    arr[0] = 1;

    if (n < m) SWAP(n, m, int)

    for (int i = 1; i < n + m; i++) {
        for (
            int y = i < n ? 0 : i - n + 1,
                x = i < n ? i : n - 1;
            x >= 0 && y < m;
            x--, y++
        ) {
            #define STEP(dx, dy) (x + dx >= 0 && x + dx < n && y + dy >= 0 && y + dy < m ? arr[x + dx + (y + dy) * n] : 0)
            arr[x + y * n] = STEP(-2, 1) + STEP(-2, -1) + STEP(-1, -2) + STEP(1, -2);
        }

        #ifdef DEBUG
        for (int j = 0; j < m; j++) {
            for (int i = 0; i < n; i++) {
                printf(" %-3lu", arr[i + j*n]);
            }
            printf("\n");
        }
        printf("\n");
        #endif
    }

    printf("%lu\n", arr[n*m - 1]);

    return 0;
}
