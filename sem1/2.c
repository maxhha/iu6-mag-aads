#include <stdio.h>
#include <stdlib.h>

int magic_f(int nn, int w, int h) {
    int x = nn / w;
    int y = nn / h;
    return x * y;
}

int main(void) {
	int w, h, n;
    scanf("%d%d%d", &w, &h, &n);

    int l = -1, r = 2147483647;
    printf(" -- %d %d\n", l, r);
    while (r - l > 1) {
        int m = (r + l) / 2;
        printf(" -- %d %d %d\n", l, r, m);
        if (n > magic_f(m, w, h)) {
            l = m;
        } else {
            r = m;
        }
    }

    printf(" -- %d %d %d %d\n", l, r, magic_f(l, w, h), magic_f(r, w, h));

    printf("%d\n", r);

	return 0;
}
