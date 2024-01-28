#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #define DEBUG

/*
  f i x p r e f i x s u f f i x
f 1 _ _ _ _ _ 1 _ _ _ _ 1 1 _ _
i _ 2 _ _ _ _ _ 2 _ _ _ _ _ 2 _
x _ _ 3 _ _ _ _ _ 3 _ _ _ _ _ 3
p _ _ _ 4 _ _ _ _ _ _ _ _ _ _ _
r _ _ _ _ 5 _ _ _ _ _ _ _ _ _ _
...
*/

#define SWAP(a, b, T) { T t = (a); (a) = (b); (b) = t;}

typedef struct s_buf {
    int size;
    int data[];
} *buf_t;

int main(void) {
    char s[1000001];
    fgets(s, 1000001, stdin);
    int l = strlen(s);
    if (s[l - 1] == '\n') {
        s[l - 1] = 0;
        l--;
    }

    if (l < 3) {
        printf("Not found\n");
        return 0;
    }

    buf_t b1 = (buf_t) malloc(sizeof (struct s_buf) + sizeof(int) * l);
    buf_t b2 = (buf_t) malloc(sizeof (struct s_buf) + sizeof(int) * l);
    b1->size = 0;
    b2->size = 0;

    buf_t curr_buf = b1;
    buf_t prev_buf = b2;

    int current_max = 1;

    for (int i = 0; i < l; i++) {
        if (s[i] == s[0]) {
            curr_buf->data[curr_buf->size] = i;
            curr_buf->size++;
        }
    }

    #ifdef DEBUG
    {
        printf("  ");
        for (int i = 0; i < l; i++) {
            if (i > 0) {
                printf(" ");
            }
            printf("%c", s[i]);
        }
        printf("\n");
        printf("%c ", s[0]);
        for (int i = 0, p = 0; p < curr_buf->size && i < l; i++) {
            if (i > 0) {
                printf(" ");
            }
            if (curr_buf->data[p] == i) {
                printf("Y");
                p++;
            } else {
                printf(" ");
            }
        }
        printf("\n");
    }
    #endif  // DEBUG

    int result_max = 0;

    int j = 1;
    while (curr_buf->size >= 3) {
        SWAP(curr_buf, prev_buf, buf_t)
        curr_buf->size = 0;

        for (int i = 0; i < prev_buf->size; i++) {
            if (prev_buf->data[i] == l - 1) {
                if (prev_buf->size >= 3) {
                    result_max = j;
                }
            } else if (s[j] == s[prev_buf->data[i] + 1]) {
                curr_buf->data[curr_buf->size] = prev_buf->data[i] + 1;
                curr_buf->size++;
            }
        }

        #ifdef DEBUG
        {
            printf("%c ", s[j]);
            for (int i = 0, p = 0; p < curr_buf->size && i < l; i++) {
                if (i > 0) {
                    printf(" ");
                }
                if (curr_buf->data[p] == i) {
                    printf("Y");
                    p++;
                } else {
                    printf(" ");
                }
            }
            printf("\n");
        }
        #endif  // DEBUG

        j++;
    }

    if (result_max == 0) {
        printf("Not found\n");
    } else {
        s[result_max] = '\0';
        printf("%s\n", s);
    }

    free(b1);
    free(b2);
}
