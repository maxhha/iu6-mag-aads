#include <stdio.h>
#include <stdlib.h>

long int magic_f(long int nn, long int w, long int h) {
    long int x = nn / w;
    long int y = nn / h;
    return x * y;
}

int main(void) {
	long int w, h, n;
    scanf("%ld%ld%ld", &w, &h, &n);

    long int l = -1, r = 1;

    while (magic_f(r, w, h) < n) {
        r *= 2;
    }

    // printf(" -- %ld %ld\n", l, r);
    while (r - l > 1) {
        long int m = (r + l) / 2;
        // printf(" -- %ld %ld %ld\n", l, r, m);
        if (n > magic_f(m, w, h)) {
            l = m;
        } else {
            r = m;
        }
    }

	if (magic_f(r, w, h) < n) {
		r++;
	}

    // printf(" -- %ld %ld %ld %ld\n", l, r, magic_f(l, w, h), magic_f(r, w, h));

    printf("%ld\n", r);

	return 0;
}
