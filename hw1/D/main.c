#include <stdio.h>
#include <stdlib.h>

long int magic_f(long int a, long int b, long int w, long int h, long int d) {
    long int x_n = w / (d*2 + a);
    long int y_n = h / (d*2 + b);
    long int n1 = x_n * y_n;

    x_n = w / (d*2 + b);
    y_n = h / (d*2 + a);
    long int n2 = x_n * y_n;
    return n1 > n2 ? n1 : n2;
}

long int max(long int a, long int b) {
    return a > b ? a : b;
}

int main(void) {
    long int n, a, b, w, h;
    scanf("%ld%ld%ld%ld%ld", &n, &a, &b, &w, &h);

    long int l = -1, r = max(max((w - a) / 2, (w - b) / 2), max((h - a) / 2, (h - b) / 2)) + 1;
    // printf("%ld\t%ld\n", l, r);


    while (r - l > 1) {
        long int m = (l + r) / 2;
        long int f = magic_f(a, b, w, h, m);
        // printf("%ld\t%ld\t%ld\t%ld\n", l, m, r, f);
        if (f >= n) {
            l = m;
        } else {
            r = m;
        }
    }

    printf("%ld\n", l);

    return 0;
}
