#include <stdlib.h>
#include <stdio.h>

int main(void) {
    long int a;
    scanf("%ld", &a);

    long int i = 2;
    int is_first = 1;
    long int c = 0;

    while (i * i <= a) {
        while (a % i == 0) {
            a /= i;
            c++;
        }

        if (c > 0) {
            if (is_first) {
                is_first = 0;
            } else {
                printf("*");
            }
            if (c == 1) {
                printf("%ld", i);
            } else {
                printf("%ld^%ld", i, c);
            }
            c = 0;
        }

        i++;
    }

    if (a > 1) {
        if (!is_first) {
            printf("*");
        }
        printf("%ld", a);
    }

    printf("\n");

    return 0;
}
