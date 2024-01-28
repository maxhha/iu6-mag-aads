#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #define DEBUG

int main(void) {
    int n, w;
    scanf("%d%d", &n, &w);
    int p[n];
    char ww[w+2];
    memset(ww, 0, sizeof(char) * (w+2));
    ww[0] = 1;

    for (int i = 0; i < n; i++){
        int p_i;
        scanf("%u", &p_i);
        p[i] = p_i;
        for (int j = w; j >= p_i; j--) {
            ww[j] |= ww[j - p_i];
            #ifdef DEBUG
            if (ww[j]) {
                printf("%d ", j);
            }
            #endif
        }
        #ifdef DEBUG
        printf("\n");
        #endif
    }

    int a = w;
    while (a > 0 && !ww[a]) a--;

    printf("%d\n", a);

    int pp[n];
    int m = 0;
    for (int i = n - 1; i >= 0 && a > 0; i--) {
        if (p[i] <= a && ww[a - p[i]]) {
            pp[m] = p[i];
            m++;
            a -= p[i];
        }
    }
    printf("%d\n", m);

    for (int i = m - 1; i >= 0; i--) {
        printf("%d%c", pp[i], i == 0 ? '\0' : ' ');
    }

    printf("\n");

    return 0;
}
