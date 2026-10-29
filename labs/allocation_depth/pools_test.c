#include "allocation.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                         \
    do                                                                   \
    {                                                                    \
        if (!(x))                                                        \
        {                                                                \
            fprintf(stderr, "ASSERT pools line %d: %s\n", __LINE__, #x); \
            return 1;                                                    \
        }                                                                \
    } while (0)

struct al_resource
{
    unsigned acquired, destroyed;
    int fail;
};

static int init(void *v, unsigned char *p, size_t n)
{
    struct al_resource *r = v;
    ++r->acquired;
    if (r->fail)
    {
        ++r->destroyed;
        return 1;
    }
    for (size_t i = 0; i < n; ++i)
    {
        p[i] = (unsigned char)(i + 1);
    }
    return 0;
}

static int destroy(void *v)
{
    struct al_resource *r = v;
    ++r->destroyed;
    return 0;
}

int main(void)
{
    struct al_pool p, q;
    struct al_pool_handle h[3], foreign, reused;
    int32_t value = 0;
    al_pool_init(&p);
    al_pool_init(&q);
    CHECK(al_pool_take(&q, 88, &foreign) == AL_OK);
    for (unsigned i = 0; i < 3; ++i)
    {
        CHECK(al_pool_take(&p, (int32_t)i, &h[i]) == AL_OK);
    }
    CHECK(al_pool_take(&p, 9, &reused) == AL_FULL);
    {
        unsigned live = 0, free = 0;
        for (unsigned i = 0; i < 3; ++i)
        {
            live += p.producer.state[i] == 1;
            free += p.producer.state[i] == 0;
        }
        CHECK(live == 3 && free == 0);
    }
    CHECK(al_pool_get(&p, foreign, &value) == AL_STALE);
    CHECK(algo_pool_get(&p.producer, foreign.raw, &value) == ALGO_OK &&
          value == 0);
    CHECK(al_pool_release(&p, h[1]) == AL_OK);
    {
        unsigned live = 0, free = 0;
        for (unsigned i = 0; i < 3; ++i)
        {
            live += p.producer.state[i] == 1;
            free += p.producer.state[i] == 0;
        }
        CHECK(live == 2 && free == 1);
    }
    CHECK(al_pool_take(&p, 91, &reused) == AL_OK);
    CHECK(al_pool_get(&p, h[1], &value) == AL_STALE);
    CHECK(al_pool_get(&p, reused, &value) == AL_OK && value == 91);
    CHECK(al_pool_release(&p, reused) == AL_OK);
    CHECK(al_pool_take(&p, 92, &reused) == AL_OK);
    CHECK(al_pool_release(&p, reused) == AL_OK);
    CHECK(al_pool_take(&p, 93, &reused) == AL_FULL);
    CHECK(al_pool_release(&p, h[0]) == AL_OK);
    CHECK(al_pool_release(&p, h[0]) == AL_STALE);
    CHECK(al_pool_release(&p, h[2]) == AL_OK);
    CHECK(al_pool_release(&q, foreign) == AL_OK);
    struct al_classes classes, saved;
    struct al_handle handles[4], out;
    struct al_resource r = {0};
    size_t payload, charged, retained;
    CHECK(al_classes_init(&classes) == AL_OK);
    unsigned sizes[] = {1, 8, 9, 17};
    for (unsigned i = 0; i < 4; ++i)
    {
        CHECK(al_classes_take(&classes, sizes[i], init, destroy, &r,
                              &handles[i]) == AL_OK);
    }
    CHECK(al_classes_stats(&classes, &payload, &charged, &retained) == AL_OK &&
          payload == 35 && charged == 64 && retained == 224);
    for (unsigned i = 0; i < 4; ++i)
    {
        CHECK(al_classes_release(&classes, handles[i]) == AL_OK);
    }
    CHECK(r.acquired == 4 && r.destroyed == 4);
    CHECK(al_classes_stats(&classes, &payload, &charged, &retained) == AL_OK &&
          !payload && !charged && retained == 224);
    r.fail = 1;
    saved = classes;
    CHECK(al_classes_take(&classes, 8, init, destroy, &r, &out) == AL_CLEANUP &&
          memcmp(&saved, &classes, sizeof classes) == 0 && r.acquired == 5 &&
          r.destroyed == 5);
    r.fail = 0;
    CHECK(al_classes_take(&classes, 8, init, destroy, &r, &out) == AL_OK);
    CHECK(al_classes_release(&classes, out) == AL_OK &&
          r.acquired == r.destroyed);
    printf("PASS pools actual-producer owner/3-generation/cleanup/classes\n");
    return 0;
}
