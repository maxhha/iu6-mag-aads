#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #define DEBUG

/*
       1
  2        2
 3   3   3   3
4 4 4 4 4 4 4 4

*/

#define TREE_DATA_T int
typedef struct s_tree {
    int n;
    int size;
    int *arr;
    TREE_DATA_T data[];
} *tree_p;

tree_p create_tree(int n) {
    int size = 1;
    while (size < n) {
        size *= 2;
    }
    if (size > 1) {
        size = size * size - 1;
    }
    tree_p t = (tree_p) malloc(sizeof(struct s_tree) + sizeof (TREE_DATA_T) * size);
    if (t == NULL) {
        return NULL;
    }
    t->n = n;
    t->size = size;
    memset(t->data, 0, sizeof (TREE_DATA_T) * size);
    return t;
}

int read_tree(tree_p t) {
    int *data = t->data +
    for (int i = 0; i < t->n; i++) {

    }
}

int main(void) {
    int n, q;
    scanf("%d%d", &n, &q);



    return 0;
}
