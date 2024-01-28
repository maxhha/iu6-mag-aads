#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

struct citizen_s {
    int city;
    int cost;
};

int main(void)
{
    int n;
    scanf("%d", &n);
    struct citizen_s *stack = (struct citizen_s *) malloc(n * sizeof(struct citizen_s));
    int stack_i = 0;
    int *result = (int *) malloc(n * sizeof(int));

    for (int i = 0; i < n; i++) {
        int c;
        scanf("%d", &c);
        for(; stack_i > 0 && stack[stack_i - 1].cost > c; stack_i--) {
            result[stack[stack_i - 1].city] = i;
        }

        stack[stack_i].city = i;
        stack[stack_i].cost = c;
        result[i] = -1;
        stack_i++;
    }

    for (int i = 0; i < n; i++) {
        if (i > 0) {
            printf(" ");
        }
        printf("%d", result[i]);
    }
    printf("\n");

    free(stack);
    free(result);
    return 0;
}
