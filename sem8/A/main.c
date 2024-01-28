#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*

1 1
2 2
3 4
4 7

*/

int main(void) {
    int n;
    int MOD = 1000000007;
    scanf("%d", &n);

    long int arr[3];
    // memset(arr, 0, sizeof(int) * n);
    arr[0] = 1;
    arr[1] = 2;
    arr[2] = 4;
    long int sum = arr[0] + arr[1] + arr[2];
    if (n < 3) {
        printf("%d\n", arr[n-1]);
        return 0;
    }

    for (int i = 3; i < n - 1; i++) {
        long int t = arr[i % 3];
        arr[i % 3] = sum;
        sum = ((sum - t) + sum) % MOD;
    }

    printf("%ld\n", sum);
    return 0;
}
