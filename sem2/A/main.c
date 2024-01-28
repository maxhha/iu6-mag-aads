#include "stdio.h"
#include "stdlib.h"

/*

7 * 8 + ( 3 - 5 )

7   8    3   5
  *        -
      +

7 8 * 3 5 - +

7 * 8
7 8 *

7 + 8 * 1
7 8 1
+ *

1 + 2 / 5 + 10 - 8 + 12 - 17 + 45 - 123 * 2 + 15 - 94 / 7

1 2 5 / +

1 * (2 + 3)
1 2 3 + *
* ( +


*/

// #define DEBUG

#ifdef DEBUG
    #define LOG_DEBUG(...) printf(__VA_ARGS__)
#else
    #define LOG_DEBUG
#endif

int main(void) {
    #define STACK_SIZE 10000
    char stack[STACK_SIZE];
    int stack_i = 0;

    int c;
    #define S_EXPRESSION 0
    #define S_NUMBER     1
    #define S_OPERATOR   2
    char state_to_char[] = {'E', 'N', 'O'};
    int state = S_EXPRESSION;
    int has_space = 1;

    while ((c = getc(stdin)) != EOF) {
        LOG_DEBUG("\n%c > %c\n", state_to_char[state], c);

        if (state == S_EXPRESSION) {
            if (c >= '0' && c <= '9') {
                if (!has_space) {
                    printf(" ");
                }
                printf("%c", c);
                state = S_NUMBER;
                has_space = 0;
            }
            else if (c == '(') {
                stack[stack_i] = c;
                stack_i++;
            }
        }
        else if (state == S_NUMBER) {
            if (c >= '0' && c <= '9') {
                printf("%c", c);
            }
            else if (c == ' ') {
                if (!has_space) {
                    printf(" ");
                    has_space = 1;
                }
                state = S_OPERATOR;
            }
            else if (c == ')') {
                for (; stack_i > 0 && stack[stack_i - 1] != '('; stack_i--) {
                    if (!has_space) {
                        printf(" ");
                    }
                    LOG_DEBUG("\n  - %d %c \n", stack_i - 1, stack[stack_i - 1]);
                    printf("%c", stack[stack_i - 1]);
                    has_space = 0;
                }
                stack_i--;
            }
        } else if (state == S_OPERATOR) {
            if (c == '+' || c == '-') {
                for (; stack_i > 0 && (stack[stack_i - 1] == '+' || stack[stack_i - 1] == '-' || stack[stack_i - 1] == '*' || stack[stack_i - 1] == '/'); stack_i--) {
                     if (!has_space) {
                        printf(" ");
                    }
                    printf("%c", stack[stack_i - 1]);
                    has_space = 0;
                }

                stack[stack_i] = c;
                stack_i++;
                state = S_EXPRESSION;
            }
            else if (c == '*' || c == '/') {
                for (; stack_i > 0 && (stack[stack_i - 1] == '*' || stack[stack_i - 1] == '/'); stack_i--) {
                    if (!has_space) {
                        printf(" ");
                    }
                    printf("%c", stack[stack_i - 1]);
                    has_space = 0;
                }
                stack[stack_i] = c;
                stack_i++;
                state = S_EXPRESSION;
            }
            else if (c == ')') {
                for (; stack_i > 0 && stack[stack_i - 1] != '('; stack_i--) {
                    if (!has_space) {
                        printf(" ");
                    }
                    LOG_DEBUG("\n  - %d %c \n", stack_i - 1, stack[stack_i - 1]);
                    printf("%c", stack[stack_i - 1]);
                    has_space = 0;
                }
                stack_i--;
            }
        }
    }

    for (;stack_i > 0; stack_i--) {
        if (!has_space) {
            printf(" ");
        }
        printf("%c", stack[stack_i - 1]);
        has_space = 0;
    }

    printf("\n");

    return 0;
}
