#include "posix_lifecycle.h"
#include <pthread.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x)                                     \
    do                                               \
    {                                                \
        if (!(x))                                    \
        {                                            \
            fprintf(stderr, "TL_BASELINE:%s\n", #x); \
            exit(41);                                \
        }                                            \
    } while (0)

int main(void)
{
    int previous;
    CHECK(!pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &previous));
    for (unsigned n = 0; n <= 4; n++)
    {
        for (unsigned fail = 0; fail <= n; fail++)
        {
            unsigned made = fail ? fail - 1 : n, sum = 0;
            for (unsigned i = 1; i <= made; i++)
            {
                sum += i * i;
            }
            struct pl_pool p = {99, 99, 99, 99, 99};
            CHECK(!pl_pool(n, fail, &p));
            CHECK(p.created == made && p.joined == made &&
                  p.completed == made && p.sum == sum &&
                  p.create_error == (fail ? EAGAIN : 0));
        }
    }
    struct pl_pool p = {99, 99, 99, 99, 99};
    CHECK(pl_pool(5, 0, &p) == EINVAL && p.sum == 99);
    CHECK(pl_pool(1, 2, &p) == EINVAL && p.created == 99);
    CHECK(pl_pool(1, 0, NULL) == EINVAL);
    CHECK(!pthread_setcancelstate(previous, NULL));
    puts("PASS posix_lifecycle_depth.pool");
    return 0;
}
