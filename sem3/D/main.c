#include <stdio.h>
#include <stdlib.h>

int main(void) {
    long int x;
    scanf("%ld", &x);

    long int i = 1, a_i = 1;
    long int j = 1, b_j = 1;
    long int n = 1, c_n = 1;

    while (n < x) {
        if (a_i == b_j) {
            i++;
            j++;
            a_i = i * i;
            b_j = j * j * j;
        }
        else if (a_i < b_j) {
            i++;
            a_i = i * i;
        } else {
            j++;
            b_j = j * j * j;
        }

        n++;
        c_n = a_i < b_j ? a_i : b_j;
    }

    printf("%ld\n", c_n);

    return 0;
}
