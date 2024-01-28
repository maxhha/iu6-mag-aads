#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// #define DEBUG

typedef struct s_graph {
    int size;
    int edges[];
} *graph_t;

#define graph_edge(g, i, j) ((g)->edges[(i) * (g->size) + (j)])

graph_t create_graph(int size) {
    graph_t g = (graph_t) malloc(sizeof (struct s_graph) + size * size * sizeof(int));
    if (g == NULL) {
        return NULL;
    }

    g->size = size;
    return g;
}

void free_graph(graph_t g) {
    free(g);
}

int scanf_graph(graph_t g) {
    for (int i = 0; i < g->size; i++) {
        for(int j = 0; j < g->size; j++) {
            if (scanf("%d", &graph_edge(g, i, j)) != 1) {
                return 1;
            }
        }
    }
    return 0;
}

int find_farthest_edge_for_graph(const graph_t g, int vertex) {
    int stack[g->size];
    char visited[g->size];
    memset(visited, 0, g->size);
    visited[vertex] = 1;

    int *stack_p = stack;
    *stack_p = vertex;
    int path_len = 2;
    int search_continue_from = 0;

    #ifdef DEBUG
    printf("*stack_p   i   edge  action\n");
    #endif

    while (stack_p >= stack) {
        int searching = 1;

        for (int i = search_continue_from; searching && i < g->size; i++) {
            #ifdef DEBUG
            printf("%8d  %3d  %4d", *stack_p, i, graph_edge(g, *stack_p, i));
            #endif
            if (*stack_p == i || !graph_edge(g, *stack_p, i)) {
                #ifdef DEBUG
                printf("  skip\n");
                #endif

                continue;
            }

            if (stack_p > stack && *(stack_p - 1) == i) {
                #ifdef DEBUG
                printf("  come\n");
                #endif
                continue;
            }

            if (visited[i] && visited[i] < path_len) {
                #ifdef DEBUG
                printf("  short\n");
                #endif
                continue;
            }

            #ifdef DEBUG
            printf("  enter\n");
            #endif

            visited[i] = path_len;
            path_len++;
            stack_p++;
            *stack_p = i;
            searching = 0;
            search_continue_from = 0;
        }

        if (searching) {
            search_continue_from = *stack_p + 1;
            stack_p--;
            path_len--;
        }
    }

    int max_len = 0;
    int max_i = 0;
    for (int i = 0; i < g->size; i++) {
        if (visited[i] > max_len) {
            max_len = visited[i];
            max_i = i;
        }
    }

    return max_i;
}

int main(void) {
    int n, s;
    scanf("%d %d", &n, &s);
    graph_t g = create_graph(n);
    if (scanf_graph(g)) {
        printf("failed to read graph\n");
        goto fail_scanf;
    }
    printf("%d\n", find_farthest_edge_for_graph(g, s - 1) + 1);
fail_scanf:
    free_graph(g);

    return 0;
}
