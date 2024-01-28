#include <stdlib.h>
#include <stdio.h>

int main(void) {
    long int a;
    scanf("%ld", &a);

    int is_prime = 1;

    for (long int i = 2; i * i <= a && is_prime; i++) {
        is_prime = a % i != 0;
    }

    if (is_prime) {
        printf("prime\n");
    } else {
        printf("composite\n");
    }

    return 0;
}
