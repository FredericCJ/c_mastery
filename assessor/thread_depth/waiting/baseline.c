#define _POSIX_C_SOURCE 200809L
#include "hosted.h"
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x)                                     \
    do                                               \
    {                                                \
        if (!(x))                                    \
        {                                            \
            fprintf(stderr, "TW_BASELINE:%s\n", #x); \
            exit(41);                                \
        }                                            \
    } while (0)

int main(void)
{
    int old;
    CHECK(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &old) == 0);
    struct hs_queue q;
    CHECK(hs_queue_init(&q) == 0);
    for (int i = 1; i <= 4; i++)
        CHECK(hs_queue_push(&q, i) == 0);
    CHECK(hs_queue_close(&q) == 0);
    CHECK(hs_queue_push(&q, 9) == -1);
    for (int i = 1; i <= 4; i++)
    {
        int n;
        CHECK(hs_queue_pop(&q, &n) == 1 && n == i);
    }
    int n = 77;
    CHECK(hs_queue_pop(&q, &n) == 0 && n == 77);
    CHECK(hs_queue_destroy(&q) == 0);
    puts("OBS baseline capacity4 close-drain; cancellation disabled; no timed "
         "API");
    return 0;
}
