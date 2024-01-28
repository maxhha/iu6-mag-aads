#include <stdio.h>
#include <termios.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

/* reads from keypress, echoes */
int getch(void)
{
    struct termios oldattr, newattr;
    int ch;
    tcgetattr( STDIN_FILENO, &oldattr );
    newattr = oldattr;
    newattr.c_lflag &= ~( ICANON | ECHO );
    tcsetattr( STDIN_FILENO, TCSANOW, &newattr );
    ch = getchar();
    tcsetattr( STDIN_FILENO, TCSANOW, &oldattr );
    return ch;
}

#define WIN_WIDTH 60
#define WIN_HEIGHT 20
#define DELAY 200

#define STATUS_EMPTY 0
#define STATUS_STARTING 1
#define STATUS_STARTED 2
#define STATUS_EXITING 3

struct bchar_s {
    int status;
    pthread_t tid;
};

#define MAX_BCHAR_COUNT 10
struct bchar_s bchars[MAX_BCHAR_COUNT];

void *char_thread(void *param) {
    int ci = (int) param;
    char c = ci + '0';
    bchars[ci].status = STATUS_STARTED;

    int x = rand() % (WIN_WIDTH - 1) + 1, y = rand() % (WIN_HEIGHT - 1) + 1;
    int d = rand() % 4;
    int dx = (d % 2) * 2 - 1;
    int dy = (d / 2) * 2 - 1;

    while(bchars[ci].status == STATUS_STARTED) {
        printf("\033[s\033[%d;%df%c\033[u", y + 1, x + 1, c);
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = DELAY * 1000000;
        nanosleep(&ts, NULL);
        printf("\033[s\033[%d;%df \033[u", y + 1, x + 1);
        x += dx;
        y += dy;
        if (x <= 1) {
            x = 1;
            dx = -dx;
        }
        if (x > WIN_WIDTH) {
            x = WIN_WIDTH;
            dx = -dx;
        }
        if (y <= 1) {
            y = 1;
            dy = -dy;
        }
        if (y > WIN_HEIGHT) {
            y = WIN_HEIGHT;
            dy = -dy;
        }
    }
    bchars[ci].status = STATUS_EMPTY;
}


int main(void) {
    setbuf(stdout, NULL);
    setbuf(stdin, NULL);
    printf("\033[2J");
    printf("\033[%d;%df", 0, 0);
    printf("+");
    for (int i = 0; i < WIN_WIDTH; i++) {
        printf("-");
    }
    printf("+\n");

    for (int i = 0; i < WIN_HEIGHT; i++) {
        printf("|\033[%d;%df|\n", i + 2, WIN_WIDTH + 2);
    }
    printf("+");
    for (int i = 0; i < WIN_WIDTH; i++) {
        printf("-");
    }
    printf("+\n");

    printf("\n");
    printf("\033[%d;%df", WIN_HEIGHT + 3, 0);

    int c;
    while ((c = getch()) != EOF && c != 'q') {
        printf("\033[%d;%df", WIN_HEIGHT + 3, 0);
        if (!(c >= '0' && c <= '9')) {
            continue;
        }

        c -= '0';

        if (bchars[c].status == STATUS_EMPTY) {
            bchars[c].status = STATUS_STARTING;

            pthread_attr_t attr;
            pthread_attr_init(&attr);
            pthread_create(&bchars[c].tid, &attr, char_thread, (void *) c);
            pthread_attr_destroy(&attr);
        } else if (bchars[c].status == STATUS_STARTED) {
            bchars[c].status = STATUS_EXITING;
        }
    }

    for (int i = 0; i < MAX_BCHAR_COUNT; i++) {
        if (bchars[i].status != STATUS_EMPTY) {
            bchars[i].status = STATUS_EXITING;
            pthread_join(bchars[i].tid, NULL);
        }
    }

    return 0;
}
