#include <stdio.h>
#include <stdlib.h>

// #define DEBUG

#ifdef DEBUG
    #define LOG_DEBUG(...) printf(__VA_ARGS__)
#else
    #define LOG_DEBUG
#endif


int main(void) {
	int n, k;
    int a[10000];
    scanf("%d%d", &n, &k);

    for (int i = 0; i < n; i++) {
        scanf("%d", &a[i]);
    }

    int l = 0, r = a[n-1] - a[0] + 1;
    while (r - l > 1) {
        int m = (r + l) / 2;

        LOG_DEBUG(" -- %d %d %d\n", l, r, m);

        #ifdef DEBUG
        for (int ii = 0; ii < n; ii++) {
            printf("%2d  ", a[ii]);
        }
        printf("\n");
        #endif

        int cc = 1, cp = a[0];
        LOG_DEBUG("[C] ");
        for (int j = 1; j < n; j++) {
            if (a[j] - cp >= m) {
                cc++;
                cp = a[j];
                LOG_DEBUG("[C] ");
            } else {
                LOG_DEBUG("[ ] ");
            }
        }
        LOG_DEBUG("%d %d\n\n", cc, k);

        if (cc >= k) {
            l = m;
        } else {
            r = m;
        }
    }

    printf("%d\n", l);

	return 0;
}
