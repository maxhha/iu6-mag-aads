#include <stdlib.h>
#include <stdio.h>

int main(void) {
    int n, k;
    scanf("%d%d", &n, &k);

    /*

    k+x >= (n+x)/3
    3 * (k + x) >= n + x
    3k + 3x >= n + x
    3k + 3x - x >= n
    3k + 2x >= n
    2x >= n - 3k
    x >= (n - 3k) / 2
    */
   int x = ((n - 3 * k) + 1) / 2;
   printf("%d\n", x < 0 ? 0 : x);
}
