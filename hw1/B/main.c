#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*
+-+
++-
+-+

1 0 2 2 2 0 0
1 1 0 0 2 2 2
1 0 3 0 2 0 2
1 1 1 1 1 1 1

*/

// #define DEBUG

int main(void) {
    int n, m;
    scanf("%d%d", &n, &m);

    int row[m];
    int i_number = 0;
    int i_index = 1;
    char c;
    for (int i = 0; i < m; i += (c == '-' || c == '+')) {
        scanf("%c", &c);
        if (c == '-') {
            row[i] = 0;
        } else if (c != '+') {
            continue;
        } else if (i > 0 && row[i - 1]) {
            row[i] = row[i - 1];
        } else {
            row[i] = i_index;
            i_index++;
            i_number++;
        }
    }

    #ifdef DEBUG
    for (int i = 0; i < m; i++) {
        printf("%3d ", row[i]);
    }
    printf("\n");
    #endif

    for (int j = 1; j < n; j++) {
        for (int i = 0; i < m; i += (c == '+' || c == '-')) {
            scanf("%c", &c);

            if (c == '-') {
                row[i] = 0;
            } else if (c != '+') {

            } else if (i > 0 && row[i - 1] > 0) {
                if (row[i] == 0 || row[i] == row[i - 1]) {
                    row[i] = row[i - 1];
                } else {
                    int u = row[i - 1];
                    int r = row[i];
                    for (int k = 0; k < m; k++) {
                        if (row[k] == r) {
                            row[k] = u;
                        }
                    }
                    i_number--;
                }
            } else if (row[i] == 0) {
                row[i] = i_index;
                i_index++;
                i_number++;
            }
        }
        #ifdef DEBUG
        for (int i = 0; i < m; i++) {
            printf("%3d ", row[i]);
        }
        printf(" | %d\n", i_number);
        #endif
    }
    printf("%d\n", i_number);

    return 0;
}
