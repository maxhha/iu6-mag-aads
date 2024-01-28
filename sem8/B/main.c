#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define max(a,b) ((a) > (b) ? (a) : (b))

int main(void) {
    int n, m;
    scanf("%d%d", &n, &m);
    int arr[n*m];
    scanf("%d", &arr[0]);
    for (int i = 1; i < m; i++) {
        scanf("%d", &arr[i]);
        arr[i] += arr[i-1];
    }
    for (int j = 1; j < n; j++) {
        scanf("%d", &arr[j * m]);
        arr[j * m] += arr[(j - 1) * m];
        for (int i = 1; i < m; i++) {
            scanf("%d", &arr[j*m + i]);
            arr[j*m+i] += max(arr[(j-1)*m+i], arr[j*m+i-1]);
        }
    }
    printf("%d\n", arr[n*m-1]);
    if (n + m == 2) {
        printf("\n");
        return 0;
    }
    char path[n+m-2];
    int x = m - 1, y = n - 1, p = n + m - 3;
    while (x > 0 && y > 0) {
        if (arr[x - 1 + y*m] > arr[x + (y - 1) * m]) {
            path[p] = 'R';
            p--;
            x--;
        } else {
            path[p] = 'D';
            p--;
            y--;
        }
    }
    while (x > 0) {
        path[p] = 'R';
        x--;
        p--;
    }
    while (y > 0) {
        path[p] = 'D';
        y--;
        p--;
    }
    for (p = 0; p < n + m - 2; p++) {
        if (p > 0) {
            printf(" ");
        }
        printf("%c", path[p]);
    }
    printf("\n");
}
