#include <stdio.h>
#include <stdlib.h>

int main(void) {
	int n, m;
	int a[40000];
    scanf("%d%d", &n, &m);

    for (int i = 0; i < n; i++) {
        scanf("%d", &a[i]);
    }

    for (int j = 0; j < m; j++) {
        int s;
        scanf("%d", &s);
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

        if (r >= n || a[r] != s) {
            printf("0\n");
            continue;
        }

        printf("%d", r + 1);
        // printf("\n -------\n");

        l = -1, r = n;
        // printf(" -- %d %d\n", l, r);
        while (r - l > 1) {
            int m = (r + l) / 2;
            // printf(" -- %d %d %d\n", l, r, m);
            if (s >= a[m]) {
                l = m;
            } else {
                r = m;
            }
        }

        // printf(" -- %d %d %d %d\n", l, r, a[l], a[r]);
        printf(" %d\n", l + 1);
    }

	return 0;
}
