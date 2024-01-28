#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #define DEBUG

#define min(a, b) ((a) > (b) ? (b) : (a))

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

int has_graph_edge(const graph_t g, int from_v, int to_v) {
    vec_t v = g->edges[from_v];
    for (int i = 0; i < v->size; i++) {
        if (v->data[i] == to_v) {
            return 1;
        }
    }
    return 0;
}

int append_edge_graph(graph_t g, int from_v, int to_v) {
    if (!has_graph_edge(g, from_v, to_v)) {
        return append_vec(g->edges[from_v], to_v);
    }
    return 0;
}

void free_graph(graph_t g) {
    for (int i = 0; i < g->size && g->edges[i]; i++) {
        free_vec(g->edges[i]);
    }
    free(g);
}

typedef struct find_bridges_context_s {
    graph_t r;
    char *visited;
    int *time_in;
    int *time_out;
    int time;
    const graph_t g;
}* find_bridges_context;

void find_bridges_body(find_bridges_context c, const int p_i, const int pp_i) {
    const vec_t p = c->g->edges[p_i];
    c->visited[p_i] = 1;
    c->time_in[p_i] = c->time;
    c->time_out[p_i] = c->time;
    c->time++;
    #ifdef DEBUG
    printf("%4d      in[%d]=%d out[%d]=%d\n", p_i+1, p_i+1, c->time_in[p_i], p_i+1, c->time_out[p_i]);
    #endif

    for (int i = 0; i < p->size; i++) {
        int to = p->data[i];
        if (pp_i == to) {
            continue;
        }
        if (c->visited[to]) {
            c->time_out[p_i] = min(c->time_out[p_i], c->time_in[to]);
            #ifdef DEBUG
            printf("%4d %4d in[%d]=%d out[%d]=%d\n", p_i+1, to+1, to+1, c->time_in[to], p_i+1, c->time_out[p_i]);
            #endif
        } else {
            find_bridges_body(c, to, p_i);
            c->time_out[p_i] = min(c->time_out[p_i], c->time_out[to]);
            #ifdef DEBUG
            printf("%4d %4d out[%d]=%d", p_i+1, to+1, p_i+1, c->time_out[p_i]);
            #endif
            if (c->time_out[to] > c->time_in[p_i]) {
                #ifdef DEBUG
                printf(" b\n");
                #endif
                append_edge_graph(c->r, to, p_i);
                append_edge_graph(c->r, p_i, to);
            }
            #ifdef DEBUG
            printf("\n");
            #endif
        }
    }
}

void find_bridges(graph_t r, const graph_t g) {
    char visited[g->size];
    memset(visited, 0, g->size * sizeof (char));
    int time_in[g->size];
    int time_out[g->size];
    struct find_bridges_context_s c = {
        .r = r,
        .visited = visited,
        .time_in = time_in,
        .time_out = time_out,
        .time = 0,
        .g = g,
    };
    for (int i = 0; i < g->size; i++) {
        if (!visited[i]) {
            find_bridges_body(&c, i, -1);
        }
    }
}

typedef struct count_bridges_context_s {
    const graph_t br;
    const graph_t g;
    char *visited;
    long int n;
} *count_bridges_context;

void count_bridges_body(count_bridges_context c, const int p_i, const int s_i, const long int n_i) {
    vec_t p = c->g->edges[p_i];
    c->visited[p_i] = 1;

    if (s_i < p_i) {
        c->n += n_i;
    }

    for (int i = 0; i < p->size; i++) {
        int to = p->data[i];
        if (c->visited[to]) {
            continue;
        }
        count_bridges_body(c, to, s_i, n_i + (has_graph_edge(c->br, p_i, to)));
    }
}

long int count_bridges(const graph_t br, const graph_t g) {
    char visited[g->size];
    struct count_bridges_context_s c = {
    .br = br,
    .g = g,
    .visited = visited,
    .n = 0,
    };
    for (int i = 0; i < g->size; i++) {
        memset(visited, 0, g->size * sizeof (char));
        count_bridges_body(&c, i, i, 0);
    }
    return c.n;
}

int main(void) {
    int n, m;
    scanf("%d%d", &n, &m);
    graph_t g = create_graph(n);
    for (int i = 0; i < m; i++) {
        int a, b;
        scanf("%d%d", &a, &b);
        append_edge_graph(g, a - 1, b - 1);
        append_edge_graph(g, b - 1, a - 1);
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

    graph_t bridges = create_graph(n);
    find_bridges(bridges, g);

    #ifdef DEBUG
    printf("bridges:\n");
    for(int i = 0; i < n; i++) {
        printf("%2d:", i+1);
        for (int j = 0; j < bridges->edges[i]->size; j++) {
            printf(" %2d", bridges->edges[i]->data[j]+1);
        }
        printf("\n");
    }
    #endif

    printf("%ld\n", count_bridges(bridges, g));

    return 0;
}
