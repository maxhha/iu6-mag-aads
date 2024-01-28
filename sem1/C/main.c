#include <stdio.h>
#include <stdlib.h>

#define min(a, b) ((a) < (b) ? (a) : (b))

int main(void) {
	int n, m;
	long int a[100000];
    scanf("%d%d", &n, &m);

    for (int i = 0; i < n; i++) {
        scanf("%ld", &a[i]);
    }

    for (int j = 0; j < m; j++) {
        long int s;
        scanf("%ld", &s);
        int l = -1, r = n;
        // printf(" -- %d %d\n", l, r);
        while (r - l > 1) {
            int m = (r + l) / 2;
            // printf(" -- %d %d %d\n", l, r, m);
            if (s > a[m]) {
                l = m;
            } else {
                r = m;
            }
        }

        // printf(" -- %d %d %d %d\n", l, r, a[l], a[r]);

        if (r >= n) {
            printf("%ld\n", a[n - 1]);
        } else if (a[r] == s) {
            printf("%ld\n", s);
        } else if (l < 0) {
            printf("%ld\n", a[0]);
        } else if (s - a[l] <= a[r] - s) {
            printf("%ld\n", a[l]);
        } else {
            printf("%ld\n", a[r]);
        }
    }

	return 0;
}
