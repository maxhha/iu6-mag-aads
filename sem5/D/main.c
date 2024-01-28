#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// #define DEBUG

typedef struct {
    int step;
    int pos;
} s_state;

int main(void) {
    int field[20*20];
    int n, sx, sy, fx, fy;
    scanf("%d%d%d%d%d", &n, &sx, &sy, &fx, &fy);

    for (int i = 0; i < n * n; i++) {
        field[i] = n * n;
    }

    sx--; sy--; fx--; fy--;

    s_state stack[20*20];
    s_state *stack_p = stack;
    stack_p->pos = sy * n + sx;
    stack_p->step = 0;
    field[stack_p->pos] = 1;
    int path_len = 1;

    while (stack_p >= stack) {
        if (path_len < field[stack_p->pos]) {
            field[stack_p->pos] = path_len;
        }


        #ifdef DEBUG
        printf("\n");
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (field[i * n + j] == n * n) {
                    printf(" .  ");
                } else {
                    printf("%2d%c ", field[i * n + j], " <"[stack_p->pos == i * n + j]);
                }
            }
            printf("\n");
        }

        for (int k = 0; k < n; k++) printf("====");
        printf("\n");
        #endif

        if (stack_p->pos == fy * n + fx) {
            stack_p--;
            path_len--;
            continue;
        }

        div_t xy = div(stack_p->pos, n);
        int x = xy.rem;
        int y = xy.quot;

        /*
        .......
        ..0.1..
        .7...2.
        ...H...
        .6...3.
        ..5.4..
        .......
        */

        #define MOVE(dx, dy) \
            if (x + dx >= 0 && x + dx < n && y + dy >= 0 && y + dy < n && field[(y + dy) * n + x + dx] > path_len + 1) { \
                stack_p++; \
                stack_p->pos = (y + dy) * n + x + dx; \
                stack_p->step = 0; \
                path_len++; \
                continue; \
            }

        if (stack_p->step == 0) {
            stack_p->step++;
            MOVE(-1, -2)
        }

        if (stack_p->step == 1) {
            stack_p->step++;
            MOVE(1, -2)
        }

        if (stack_p->step == 2) {
            stack_p->step++;
            MOVE(2, -1)
        }

        if (stack_p->step == 3) {
            stack_p->step++;
            MOVE(2, 1)
        }

        if (stack_p->step == 4) {
            stack_p->step++;
            MOVE(1, 2)
        }

        if (stack_p->step == 5) {
            stack_p->step++;
            MOVE(-1, 2)
        }

        if (stack_p->step == 6) {
            stack_p->step++;
            MOVE(-2, 1)
        }

        if (stack_p->step == 7) {
            stack_p->step++;
            MOVE(-2, -1)
        }

        stack_p--;
        path_len--;
    }

    printf("%d\n", field[fy * n + fx] - 1);

    return 0;
}
