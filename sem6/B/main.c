#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// #define DEBUG

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

typedef struct s_graph {
    int size;
    vec_t edges[];
} *graph_t;

graph_t create_graph(int size) {
    graph_t g = (graph_t) malloc(sizeof (struct s_graph) + size * sizeof(vec_t));
    if (g == NULL) {
        goto fail_malloc;
    }
    g->size = size;
    memset(g->edges, 0, size * sizeof(vec_t));

    for (int i = 0; i < size; i++) {
        g->edges[i] = create_vec();
        if (g->edges[i] == NULL) {
            goto fail_create_vec;
        }
    }

    return g;

fail_create_vec:
    for (int i = 0; i < size && g->edges[i]; i++) {
        free_vec(g->edges[i]);
    }
    free(g);
fail_malloc:
    return NULL;
}

int append_edge_graph(graph_t g, int from_v, int to_v) {
    return append_vec(g->edges[from_v], to_v);
}

void free_graph(graph_t g) {
    for (int i = 0; i < g->size && g->edges[i]; i++) {
        free_vec(g->edges[i]);
    }
    free(g);
}

typedef struct { int v; int i; } s_stack;

void topological_sort(int *poses, const graph_t g) {
    s_stack stack[g->size];
    char visited[g->size];
    memset(visited, 0, g->size);

    int *r = poses;

    #ifdef DEBUG
    printf("topological_sort\n");
    printf(" v  i   action\n");
    #endif

    for(int start = 0; start < g->size; start++) {
        if (visited[start]) {
            continue;
        }

        s_stack *stack_p = stack;
        stack_p->v = start;
        stack_p->i = 0;
        visited[start] = 1;

        while (stack_p >= stack) {
            vec_t v = g->edges[stack_p->v];

            int searching = 1;

            while (searching && stack_p->i < v->size) {
                int i = v->data[stack_p->i];
                stack_p->i++;

                #ifdef DEBUG
                printf("%2d %2d ", stack_p->v+1, i+1);
                #endif

                if (visited[i]) {
                    #ifdef DEBUG
                    printf("  visited\n");
                    #endif
                    continue;
                }

                #ifdef DEBUG
                printf("  enter\n");
                #endif

                stack_p++;
                stack_p->v = i;
                stack_p->i = 0;
                searching = 0;
                visited[i] = 1;
            }

            if (searching) {
                #ifdef DEBUG
                printf("%2d      ret\n", stack_p->v);
                #endif
                *r = stack_p->v;
                r++;
                stack_p--;
            }
        }
    }
}

int condensation(int *groups, const int *sorted, const graph_t g) {
    s_stack stack[g->size];
    char visited[g->size];
    memset(visited, 0, g->size);

    int group_n = 0;

    #ifdef DEBUG
    printf("condensation\n");
    printf(" v  i   action\n");
    #endif

    for(const int *start = sorted + g->size - 1; start >= sorted; start--) {
        if (visited[*start]) {
            continue;
        }

        s_stack *stack_p = stack;
        stack_p->v = *start;
        stack_p->i = 0;
        visited[*start] = 1;
        groups[*start] = group_n;

        while (stack_p >= stack) {
            int searching = 1;
            vec_t v = g->edges[stack_p->v];

            while (searching && stack_p->i < v->size) {
                int i = v->data[stack_p->i];
                stack_p->i++;

                #ifdef DEBUG
                printf("%2d %2d ", stack_p->v+1, i+1);
                #endif

                if (visited[i]) {
                    #ifdef DEBUG
                    printf("  visited\n");
                    #endif
                    continue;
                }

                #ifdef DEBUG
                printf("  enter %d\n", group_n);
                #endif

                stack_p++;
                stack_p->v = i;
                stack_p->i = 0;
                searching = 0;
                visited[i] = 1;
                groups[i] = group_n;
            }

            if (searching) {
                stack_p--;
            }
        }
        group_n++;
    }

    return group_n;
}

int main(void) {
    int n; size_t m;
    scanf("%d%ld", &n, &m);

    graph_t g = create_graph(n);
    graph_t g_t = create_graph(n);

    for(size_t i = 0; i < m; i++) {
        int a, b;
        scanf("%d%d", &a, &b);
        append_edge_graph(g, a - 1, b - 1);
        append_edge_graph(g_t, b - 1, a - 1);
    }

    #ifdef DEBUG
    printf("g:\n");
    for(int i = 0; i < n; i++) {
        printf("%2d:", i+1);
        for (int j = 0; j < g->edges[i]->size; j++) {
            printf(" %2d", g->edges[i]->data[j]+1);
        }
        printf("\n");
    }
    #endif

    int *sorted_vertices = (int *) malloc(sizeof(int) * n);

    topological_sort(sorted_vertices, g);

    #ifdef DEBUG
    printf("sorted_vertices:\n");
    for (int i = 0; i < n; i++) {
        if (i > 0) {
            printf(" ");
        }
        printf("%d", sorted_vertices[i]+1);
    }
    printf("\n");
    #endif

    int *groups = (int *) malloc(sizeof(int) * n);

    int groups_n = condensation(groups, sorted_vertices, g_t);
    #ifdef DEBUG
    printf("groups:\n");
    for (int i = 0; i < n; i++) {
        if (i > 0) {
            printf(" ");
        }
        printf("%d", groups[i]);
    }
    printf("\n");
    #endif

    int *reorder_groups = (int *) malloc(sizeof(int) * groups_n);
    memset(reorder_groups, 0, sizeof(int) * groups_n);
    printf("%d\n", groups_n);
    int groups_i = 1;

    for (int i = 0; i < n; i++) {
        if (reorder_groups[groups[i]] == 0) {
            reorder_groups[groups[i]] = groups_i;
            groups_i++;
        }
        if (i > 0) {
            printf(" ");
        }
        printf("%d", reorder_groups[groups[i]]);
    }
    printf("\n");

    free(reorder_groups);
    free(groups);
    free(sorted_vertices);
    free_graph(g);
    free_graph(g_t);

    return 0;
}
