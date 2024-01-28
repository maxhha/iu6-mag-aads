#include <stdio.h>
#include <stdlib.h>

#define DEQUE_DATA_T int
#define DEQUE_DATA_NULL -1

struct deque_s
{
    int capacity;
    int size;
    int first_i;
    int last_i;
    DEQUE_DATA_T data[];
};

typedef struct deque_s *deque_t;

#define is_deque_full(d) ((d)->size == (d)->capacity)
#define is_deque_empty(d) ((d)->size == 0)
#define free_deque(d) free(d)

deque_t create_deque(int capacity)
{
    deque_t d = (deque_t)malloc((sizeof(struct deque_s)) + capacity * (sizeof(DEQUE_DATA_T)));
    if (d == NULL)
    {
        fprintf(stderr, "create_dec: malloc");
        return NULL;
    }
    d->capacity = capacity;
    d->size = 0;
    d->first_i = 0;
    d->last_i = -1;
    return d;
}

int append_deque(deque_t d, DEQUE_DATA_T item)
{
    if (is_deque_full(d))
    {
        return 1;
    }
    d->last_i = (d->last_i + 1) % d->capacity;
    d->data[d->last_i] = item;
    d->size++;
    return 0;
}

DEQUE_DATA_T pop_first_deque(deque_t d)
{
    if (d->size == 0)
    {
        return DEQUE_DATA_NULL;
    }

    DEQUE_DATA_T r = d->data[d->first_i];
    d->first_i = (d->first_i + 1) % d->capacity;
    d->size--;
    return r;
}

int main(void)
{
    deque_t a = create_deque(10), b = create_deque(10);
    if (a == NULL || b == NULL)
    {
        return 1;
    }

    int x;

    for (int i = 0; i < 5; i++)
    {
        scanf("%d", &x);
        append_deque(a, x);
    }

    for (int i = 0; i < 5; i++)
    {
        scanf("%d", &x);
        append_deque(b, x);
    }

    int has_winner = 0;

    for (int i = 0; i < 10; i++)
    {
        if (is_deque_empty(a))
        {
            has_winner = 1;
            printf("second %d\n", i);
            break;
        }

        if (is_deque_empty(b))
        {
            has_winner = 1;
            printf("first %d\n", i);
            break;
        }

        int ai = pop_first_deque(a);
        int bi = pop_first_deque(b);

        // printf("%d %d a->size %d b->size %d \n", ai, bi, a->size, b->size);

        deque_t w = (ai == 0 && bi == 9) ? a : (bi == 0 && ai == 9) ? b
                                           : (ai > bi)              ? a
                                                                    : b;

        append_deque(w, ai);
        append_deque(w, bi);
    }

    if (!has_winner)
    {
        printf("botva\n");
    }

    free_deque(a);
    free_deque(b);

    return 0;
}
