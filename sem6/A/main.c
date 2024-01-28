#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// #define DEBUG

#define min(a, b) ((a) > (b) ? (b) : (a))

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

/*

3 == 5 == 6
|    ||
2    4
|
1

*/

typedef struct {
    int capacity;
    int size;
    int *data;
} s_vec;

typedef s_vec *vec_t;

vec_t create_vec(void) {
    int capacity = 8;
    vec_t v = (vec_t) malloc(sizeof (s_vec));
    if (v == NULL) {
        goto fail_malloc;
    }
    v->capacity = capacity;
    v->size = 0;

    v->data = (int *) malloc(sizeof (int) * capacity);
    if (v->data == NULL) {
        goto fail_data;
    }
    return v;

fail_data:
    free(v);
fail_malloc:
    return NULL;
}

int append_vec(vec_t v, int val) {
    // printf("append_vec %p %d\n", v, val);
    if (v->size < v->capacity) {
        // printf("simple_add\n");
        v->data[v->size] = val;
        v->size++;
        // printf("v->size %d\n", v->size);
        return 0;
    }
    int new_cap = v->capacity * 2;
    int *data = (int *) malloc(sizeof (int) * new_cap);
    if (data == NULL) {
        return 1;
    }
    memcpy(data, v->data, v->capacity * sizeof(int));

    free(v->data);
    v->data = data;
    v->capacity = new_cap;
    v->data[v->size] = val;
    v->size++;

    return 0;
}

void free_vec(vec_t v) {
    free(v->data);
    free(v);
}

void find_connectivity_points(char *points, const graph_t g) {
    int stack[g->size];
    char visited[g->size];
    memset(visited, 0, g->size);
    int time = 0;
    int time_in[g->size];
    int time_out[g->size];
    memset(time_in, 0, g->size);
    memset(time_out, 0, g->size);

    #ifdef DEBUG
    printf("*stack_p   i   edge  action\n");
    #endif

    for(int start = 0; start < g->size; start++) {
        if (visited[start]) {
            continue;
        }

        int *stack_p = stack;
        *stack_p = start;
        int search_continue_from = 0;
        visited[start] = 1;
        time_in[start] = time;
        time_out[start] = time;
        time++;
        int children = 0;

        while (stack_p >= stack) {
            int searching = 1;

            for (int i = search_continue_from; searching && i < g->size; i++) {
                #ifdef DEBUG
                printf("%8d  %3d  %4d", *stack_p, i, graph_edge(g, *stack_p, i));
                #endif
                if (!graph_edge(g, *stack_p, i) || (stack_p > stack && i == *(stack_p - 1))) {
                    #ifdef DEBUG
                    printf("  skip\n");
                    #endif

                    continue;
                }

                if (visited[i]) {
                    time_out[*stack_p] = min(time_out[*stack_p], time_in[i]);
                    #ifdef DEBUG
                    printf("  time_out[%d]=%d\n", *stack_p, time_out[*stack_p]);
                    #endif
                    continue;
                }

                #ifdef DEBUG
                printf("  enter\n");
                #endif

                children += *stack_p == start;
                stack_p++;
                *stack_p = i;
                searching = 0;
                search_continue_from = 0;
                visited[i] = 1;
                time_in[i] = time;
                time_out[i] = time;
                time++;
            }

            if (searching) {
                if (stack_p - 1 > stack) {
                    time_out[*(stack_p - 1)] = min(time_out[*(stack_p - 1)], time_out[*stack_p]);
                    #ifdef DEBUG
                    printf("%8d  %3d  %4d  return time_out[%d]=%d", *(stack_p - 1), *stack_p, graph_edge(g, *(stack_p - 1), *stack_p), *(stack_p - 1), time_out[*(stack_p - 1)]);
                    #endif
                    if (time_out[*stack_p] >= time_in[*(stack_p - 1)]) {
                        #ifdef DEBUG
                        printf(" c");
                        #endif
                        points[*(stack_p - 1)] = 1;
                    }
                    #ifdef DEBUG
                    printf("\n");
                    #endif
                }

                search_continue_from = *stack_p + 1;
                stack_p--;
            }
        }

        if (children > 1) {
            points[start] = 1;
        }
    }
}

int main(void) {
    int n, m;
    scanf("%d%d", &n, &m);

    vec_t *ropes_by_tree = (vec_t *) malloc(sizeof (vec_t) * n);
    if (ropes_by_tree == NULL) {
        printf("failed to malloc ropes_by_tree");
        return 0;
    }
    for (int i = 0; i < n; i++) {
        ropes_by_tree[i] = create_vec();
        if (ropes_by_tree[i] == NULL) {
            printf("failed to malloc ropes_by_tree create_vec");
            return 0;
        }
    }

    for(int i = 0; i < m; i++) {
        int a, b, c;
        scanf("%d%d%d", &a, &b, &c);

        append_vec(ropes_by_tree[a - 1], i);
        append_vec(ropes_by_tree[b - 1], i);
        append_vec(ropes_by_tree[c - 1], i);
    }

    #ifdef DEBUG
    printf("ropes_by_tree:\n");
    for (int i = 0; i < n; i++) {
        printf("%d:", i+1);
        for (int j = 0; j < ropes_by_tree[i]->size; j++) {
            printf(" %2d", ropes_by_tree[i]->data[j]+1);
        }
        printf("\n");
    }
    #endif

    char *bridges = (char *) malloc(sizeof (char) * m);
    memset(bridges, 0, sizeof (char) * m);

    graph_t g = create_graph(m);

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < ropes_by_tree[i]->size; j++) {
            int from_rope = ropes_by_tree[i]->data[j];
            for (int k = j+1; k < ropes_by_tree[i]->size; k++) {
                int to_rope = ropes_by_tree[i]->data[k];
                graph_edge(g, to_rope, from_rope) = 1;
                graph_edge(g, from_rope, to_rope) = 1;
            }
        }
    }

    #ifdef DEBUG
    printf("\nrope g:\n    ");
    for (int i = 0; i < m; i++) {
        printf("%2d ", i+1);
    }
    printf("\n");
    for (int i = 0; i < m; i++) {
        printf("%2d  ", i+1);
        for (int j = 0; j < m; j++) {
            if (graph_edge(g, i, j)) {
                printf("%2d ", graph_edge(g, i, j));
            } else {
                printf(" _ ");
            }
        }
        printf("\n");
    }
    #endif

    find_connectivity_points(bridges, g);

    for (int i = 0; i < n; i++) {
        if (ropes_by_tree[i]->size == 1) {
            bridges[ropes_by_tree[i]->data[0]] = 1;
        }
    }

    int count_bridges = 0;
    for (int i = 0; i < m; i++) {
        if (bridges[i]) {
            count_bridges++;
        }
    }

    printf("%d\n", count_bridges);

    for (int i = 0; i < m; i++) {
        if (bridges[i]) {
            printf("%d\n", i+1);
        }
    }

    free_graph(g);
    free(bridges);

    for (int i = 0; i < n; i++) {
        free_vec(ropes_by_tree[i]);
    }

    free(ropes_by_tree);

    return 0;
}
