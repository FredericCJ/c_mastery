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

/* Each phase returns 0 on success, 1 after printing its failed CHECK. */
#define RUN_PHASE(phase) \
    do                   \
    {                    \
        if (phase)       \
        {                \
            return 1;    \
        }                \
    } while (0)

enum
{
    POOL_SLOTS = 3,
    SLOT_LIVE = 1, /* values of algo_pool state[] */
    SLOT_FREE = 0
};

struct al_resource
{
    unsigned acquired;
    unsigned destroyed;
    int fail;
};

/* Initialiser for al_classes_take: counts itself, and cleans up if told to fail. */
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

/* Slots 0..2 of the producer hold exactly `live` live and `free` free ones. */
static int slot_counts_are(const struct al_pool *p, unsigned live,
                           unsigned free_slots)
{
    unsigned live_seen = 0;
    unsigned free_seen = 0;
    for (unsigned i = 0; i < POOL_SLOTS; ++i)
    {
        live_seen += p->producer.state[i] == SLOT_LIVE;
        free_seen += p->producer.state[i] == SLOT_FREE;
    }
    return live_seen == live && free_seen == free_slots;
}

/* Owner binding, reuse with a new generation, and generation retirement. */
static int check_pool(void)
{
    struct al_pool p;
    struct al_pool q;
    struct al_pool_handle h[POOL_SLOTS];
    struct al_pool_handle foreign;
    struct al_pool_handle reused;
    int32_t value = 0;
    al_pool_init(&p);
    al_pool_init(&q);
    CHECK(al_pool_take(&q, 88, &foreign) == AL_OK);
    for (unsigned i = 0; i < POOL_SLOTS; ++i)
    {
        CHECK(al_pool_take(&p, (int32_t)i, &h[i]) == AL_OK);
    }
    CHECK(al_pool_take(&p, 9, &reused) == AL_FULL);
    CHECK(slot_counts_are(&p, 3, 0));
    /* The wrapper rejects a foreign handle that the raw producer would accept. */
    CHECK(al_pool_get(&p, foreign, &value) == AL_STALE);
    CHECK(algo_pool_get(&p.producer, foreign.raw, &value) == ALGO_OK &&
          value == 0);
    CHECK(al_pool_release(&p, h[1]) == AL_OK);
    CHECK(slot_counts_are(&p, 2, 1));
    CHECK(al_pool_take(&p, 91, &reused) == AL_OK);
    CHECK(al_pool_get(&p, h[1], &value) == AL_STALE);
    CHECK(al_pool_get(&p, reused, &value) == AL_OK && value == 91);
    CHECK(al_pool_release(&p, reused) == AL_OK);
    CHECK(al_pool_take(&p, 92, &reused) == AL_OK);
    CHECK(al_pool_release(&p, reused) == AL_OK);
    /* Third generation: the slot is retired, so the pool stays full. */
    CHECK(al_pool_take(&p, 93, &reused) == AL_FULL);
    CHECK(al_pool_release(&p, h[0]) == AL_OK);
    CHECK(al_pool_release(&p, h[0]) == AL_STALE);
    CHECK(al_pool_release(&p, h[2]) == AL_OK);
    CHECK(al_pool_release(&q, foreign) == AL_OK);
    return 0;
}

/* Size classes: charge, retained regions, cleanup counts and failed init. */
static int check_classes(void)
{
    struct al_classes classes;
    struct al_classes saved;
    struct al_handle handles[4];
    struct al_handle out;
    struct al_resource r = {0};
    size_t payload;
    size_t charged;
    size_t retained;
    static const unsigned sizes[] = {1, 8, 9, 17};
    CHECK(al_classes_init(&classes) == AL_OK);
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
    return 0;
}

int main(void)
{
    RUN_PHASE(check_pool());
    RUN_PHASE(check_classes());
    printf("PASS pools actual-producer owner/3-generation/cleanup/classes\n");
    return 0;
}
