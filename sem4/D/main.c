#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAGIC_N 2000000

int main(void) {
    char *arr = (char *) malloc(MAGIC_N);
    memset((void *) arr, 0, MAGIC_N);

    int k;
    scanf("%d", &k);

    int c = 0;

    for (long int i = 2; i < MAGIC_N; i++) {
        if (arr[i]) {
            continue;
        }

        c++;
        if (c == k) {
            printf("%ld\n", i);
            break;
        }

        for (long int j = i*2; j < MAGIC_N; j += i) {
           arr[j] = 1;
        }
    }

    if (c != k) {
        printf("oops\n");
    }

    return c != k;
}
