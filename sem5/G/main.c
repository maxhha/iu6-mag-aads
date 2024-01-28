#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #define DEBUG

int main(void) {
    int h, w;
    scanf("%d%d", &h, &w);

    int row[w];
    memset(row, 0, w * sizeof(int));
    int max_size = 0;

    int n = 0, i = 0;
    char c = 0;
    while(i < w && scanf("%c", &c) == 1) {
        if (c == '.') {
            row[i] = -1;
            n++;
            i++;
        } else if (c == '#') {
            if (max_size < n) {
                max_size = n;
            }
            for (int j = i-1; j >= 0 && row[j]; j--) {
                row[j] = n;
            }

            n = 0;
            i++;
        }
        #ifdef DEBUG
        for (int k = 0; k < w; k++) {
            printf("%-3d ", row[k]);
        }
        printf("\n");
        #endif
    }

    if (n > 0) {
        #ifdef DEBUG
        printf("fix border\n");
        #endif
        if (max_size < n) {
            max_size = n;
        }
        row[i] = n;
        for (int j = i-1; j >= 0 && row[j]; j--) {
            row[j] = n;
        }
        n = 0;
        #ifdef DEBUG
        for (int k = 0; k < w; k++) {
            printf("%-3d ", row[k]);
        }
        printf("\n");
        #endif
    }
    #ifdef DEBUG
    printf("first row:\n");
    for (int k = 0; k < w; k++) {
        printf("%-3d ", row[k]);
    }
    printf("\n");
    #endif

    for(int j = 1; j < h; j++) {
        int i = 0;

        while(i < w && scanf("%c", &c) == 1) {
            if (c == '.') {
                if (row[i] == 0) {
                    row[i] = -1;
                }
                i++;
            } else if (c == '#') {
                row[i] = 0;
                i++;
            }
        }

        int max_n = 0;
        n = 0;

        for (i = 0; i < w; i++) {
            if (row[i] == -1) {
                n += 1;
            } else if (row[i] > 0) {
                n += 1;
                if (row[i] > max_n) {
                    max_n = row[i];
                }
            } else {
                if (max_n == 0) {
                    n = 0;
                } else {
                    n += max_n;
                    max_n = 0;
                }

                if (max_size < n) {
                    max_size = n;
                }

                for (int k = i - 1; k >= 0 && row[k] != 0; k--) {
                    row[k] = n;
                }
                n = 0;
            }
        }

        if (n > 0) {
            #ifdef DEBUG
            printf("fix border\n");
            #endif
            if (max_n == 0) {
                n = 0;
            } else {
                n += max_n;
                max_n = 0;
            }

            if (max_size < n) {
                max_size = n;
            }
            row[i] = n;
            for (int k = i - 1; k >= 0 && row[k] != 0; k--) {
                row[k] = n;
            }
            n = 0;
            #ifdef DEBUG
            for (int k = 0; k < w; k++) {
                printf("%-3d ", row[k]);
            }
            printf("\n");
            #endif
        }

        #ifdef DEBUG
        for (int k = 0; k < w; k++) {
            printf("%-3d ", row[k]);
        }
        printf("\n");
        #endif
    }

    printf("%d\n", max_size);

    return 0;
}
