#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                               \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "DX_ALLOC:%d:%s\n", __LINE__, #x); \
            exit(1);                                           \
        }                                                      \
    } while (0)
static unsigned fail_at, attempt, live, cleaned;

static void *take(size_t n)
{
    if (++attempt == fail_at)
    {
        return NULL;
    }
    void *p = malloc(n);
    if (p)
    {
        live++;
    }
    return p;
}

static void drop(void *p)
{
    if (p)
    {
        CHECK(live > 0);
        live--;
        cleaned++;
        free(p);
    }
}

static int transaction(unsigned *published)
{
    void *p[3] = {0};
    unsigned old = *published;
    for (unsigned i = 0; i < 3; i++)
    {
        p[i] = take((i + 1) * 7);
        if (!p[i])
        {
            for (unsigned j = i; j > 0; j--)
            {
                drop(p[j - 1]);
            }
            CHECK(*published == old);
            return -1;
        }
    }
    *published = old + 1;
    for (unsigned i = 3; i > 0; i--)
    {
        drop(p[i - 1]);
    }
    return 0;
}

int main(void)
{
    for (unsigned failure = 1; failure <= 4; failure++)
    {
        fail_at = failure;
        attempt = live = cleaned = 0;
        unsigned published = 4;
        int r = transaction(&published);
        CHECK(r == (failure < 4 ? -1 : 0) &&
              published == (failure < 4 ? 4 : 5) && !live &&
              cleaned == (failure < 4 ? failure - 1 : 3));
    }
    for (size_t n = 3; n <= 5; n++)
    {
        fail_at = 0;
        attempt = live = cleaned = 0;
        unsigned char *p = take(n);
        CHECK(p);
        memset(p, 7, n);
        unsigned generation = 1;
        fail_at = attempt + 1;
        unsigned char *q = take(n + 1);
        CHECK(!q && generation == 1 && p[n - 1] == 7);
        fail_at = 0;
        q = take(n + 1);
        CHECK(q && q != p);
        memcpy(q, p, n);
        q[n] = 9;
        drop(p);
        generation++;
        p = q;
        CHECK(generation == 2 && p[n - 1] == 7 && p[n] == 9);
        drop(p);
        CHECK(!live);
    }
    puts(
        "DX:allocation:fail-prefixes1,2,3:publication-unchanged:neighbors3,4,5:failed-resize:forced-relocation:no-stale-pointer-read");
    return 0;
}
