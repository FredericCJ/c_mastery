#include "references.h"
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define CHECK(x)                                                        \
    do                                                                  \
    {                                                                   \
        if (!(x))                                                       \
        {                                                               \
            fprintf(stderr, "OW_CHECK ORC line%d: %s\n", __LINE__, #x); \
            return 1;                                                   \
        }                                                               \
    } while (0)

struct ow_counts
{
    unsigned destroyed, released;
};

static void destroyed(void *p)
{
    ++((struct ow_counts *)p)->destroyed;
}

static void released(void *p)
{
    ++((struct ow_counts *)p)->released;
}

static int base(void)
{
    struct ow_counts c = {0};
    struct ow_shared s = {1, 1, destroyed, released, &c};
    CHECK(ow_shared_downgrade(&s) == OW_OK && ow_shared_retain(&s) == OW_OK);
    CHECK(ow_shared_drop(&s) == OW_OK && !c.destroyed);
    CHECK(ow_shared_drop(&s) == OW_OK && s.strong == 0 && s.weak == 1 &&
          c.destroyed == 1 && !c.released);
    CHECK(ow_shared_upgrade(&s) == OW_DEAD);
    CHECK(ow_shared_weak_drop(&s) == OW_OK && c.released == 1);
    puts(
        "ORC_BASE actual-nonatomic implicit-weak no-resurrection distinct-lifetimes");
    return 0;
}

static int overflow(void)
{
    struct ow_counts c = {0};
    struct ow_shared s = {UINT_MAX, 1, destroyed, released, &c};
    CHECK(ow_shared_retain(&s) == OW_FULL && s.strong == UINT_MAX);
    s.strong = 1;
    s.weak = UINT_MAX;
    CHECK(ow_shared_downgrade(&s) == OW_FULL && s.weak == UINT_MAX);
    struct ow_atomic_shared a;
    atomic_init(&a.strong, UINT_MAX);
    atomic_init(&a.weak, 1);
    a.destroy = destroyed;
    a.release = released;
    a.context = &c;
    CHECK(ow_atomic_retain(&a) == OW_FULL &&
          atomic_load(&a.strong) == UINT_MAX);
    atomic_store(&a.strong, 1);
    atomic_store(&a.weak, UINT_MAX);
    CHECK(ow_atomic_downgrade(&a) == OW_FULL &&
          atomic_load(&a.weak) == UINT_MAX);
    atomic_store(&a.weak, 2);
    CHECK(ow_atomic_drop(&a) == OW_OK && atomic_load(&a.strong) == 0 &&
          atomic_load(&a.weak) == 1 && c.destroyed == 1);
    CHECK(ow_atomic_upgrade(&a) == OW_DEAD &&
          ow_atomic_weak_drop(&a) == OW_OK && c.released == 1);
    printf(
        "ORC_OVERFLOW actual-maximum-refusal atomic-size%zu alignment%zu lock-free%d\n",
        sizeof(atomic_uint), _Alignof(atomic_uint),
        atomic_is_lock_free(&a.strong));
    return 0;
}

struct ow_raw
{
    unsigned calls, releases, live, fail;
    size_t bytes;
};

static void *raw_take(void *p, size_t n)
{
    struct ow_raw *r = p;
    ++r->calls;
    r->bytes = n;
    if (r->calls == r->fail)
        return NULL;
    void *v = malloc(n);
    if (v)
        ++r->live;
    return v;
}

static void raw_release(void *p, void *v)
{
    struct ow_raw *r = p;
    ++r->releases;
    --r->live;
    free(v);
}

static int cycle(void)
{
    for (int weak = 0; weak <= 1; ++weak)
    {
        struct ow_raw raw = {0};
        struct ow_allocator cells = {0};
        struct orc_cycle_report r = {0};
        CHECK(orc_cycle((struct orc_memory){&raw, raw_take, raw_release},
                        ow_allocator(&cells), weak, &r) == OW_OK);
        CHECK(r.retained_values == (weak ? 0u : 2u) &&
              r.retained_controls == 2 && r.destructions == 2 &&
              r.reclamations == 2 && r.repair_edges == (weak ? 0u : 1u) &&
              r.queued_releases == 2);
        CHECK(raw.calls == 2 && raw.releases == 2 && !raw.live &&
              cells.attempts == 2 && cells.releases == 2 && !cells.live);
    }
    puts(
        "ORC_CYCLE actual-allocated-two-nodes strong-retention weak-repair deferred-no-reentry");
    return 0;
}

static int failures(void)
{
    for (unsigned raw_fail = 0; raw_fail <= 2; ++raw_fail)
        for (unsigned cell_fail = 0; cell_fail <= 2; ++cell_fail)
        {
            if (!raw_fail && !cell_fail)
                continue;
            struct ow_raw raw = {0};
            raw.fail = raw_fail;
            struct ow_allocator cells = {0};
            cells.fail_at = cell_fail;
            struct orc_cycle_report out = {91, 92, 93, 94, 95, 96};
            CHECK(orc_cycle((struct orc_memory){&raw, raw_take, raw_release},
                            ow_allocator(&cells), 0, &out) == OW_NOMEM);
            CHECK(!raw.live && !cells.live && out.retained_values == 91 &&
                  out.queued_releases == 96);
        }
    puts("ORC_FAILURE raw-versus-typed-acquisition unwind unchanged-report");
    return 0;
}

struct atomic_counts
{
    atomic_uint destroyed, released;
};

static void atomic_destroyed(void *p)
{
    atomic_fetch_add(&((struct atomic_counts *)p)->destroyed, 1);
}

static void atomic_released(void *p)
{
    atomic_fetch_add(&((struct atomic_counts *)p)->released, 1);
}

struct ref_worker
{
    struct ow_atomic_shared *refs;
    unsigned failed;
};

static void *retainer(void *p)
{
    struct ref_worker *w = p;
    for (unsigned i = 0; i < 5000; ++i)
        if (ow_atomic_retain(w->refs) != OW_OK ||
            ow_atomic_drop(w->refs) != OW_OK)
            w->failed = 1;
    if (ow_atomic_drop(w->refs) != OW_OK)
        w->failed = 1;
    return NULL;
}

static int threads(void)
{
    struct atomic_counts counts;
    atomic_init(&counts.destroyed, 0);
    atomic_init(&counts.released, 0);
    struct ow_atomic_shared a;
    atomic_init(&a.strong, 1);
    atomic_init(&a.weak, 1);
    a.destroy = atomic_destroyed;
    a.release = atomic_released;
    a.context = &counts;
    CHECK(ow_atomic_downgrade(&a) == OW_OK);
    pthread_t threads[4];
    struct ref_worker work[4];
    for (unsigned i = 0; i < 4; ++i)
    {
        CHECK(ow_atomic_retain(&a) == OW_OK);
        work[i] = (struct ref_worker){&a, 0};
        CHECK(pthread_create(&threads[i], NULL, retainer, &work[i]) == 0);
    }
    CHECK(ow_atomic_drop(&a) == OW_OK);
    for (unsigned i = 0; i < 4; ++i)
        CHECK(pthread_join(threads[i], NULL) == 0 && !work[i].failed);
    CHECK(atomic_load(&a.strong) == 0 && atomic_load(&a.weak) == 1 &&
          atomic_load(&counts.destroyed) == 1 &&
          atomic_load(&counts.released) == 0);
    CHECK(ow_atomic_upgrade(&a) == OW_DEAD &&
          ow_atomic_weak_drop(&a) == OW_OK &&
          atomic_load(&counts.released) == 1);
    puts("ORC_THREADS actual-four-owned-workers 20000-retain-drop-pairs");
    return 0;
}

struct ow_gate
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    unsigned arrived, go;
};

static void start(struct ow_gate *g)
{
    pthread_mutex_lock(&g->mutex);
    ++g->arrived;
    pthread_cond_broadcast(&g->cond);
    while (!g->go)
        pthread_cond_wait(&g->cond, &g->mutex);
    pthread_mutex_unlock(&g->mutex);
}

struct race_node
{
    struct ow_atomic_shared refs;
    struct v4_allocator allocator;
    int32_t *payload;
    struct atomic_counts *counts;
};

static void race_destroy(void *p)
{
    struct race_node *n = p;
    n->allocator.release(n->allocator.context, n->payload);
    n->payload = NULL;
    atomic_fetch_add(&n->counts->destroyed, 1);
}

static void race_release(void *p)
{
    struct race_node *n = p;
    atomic_fetch_add(&n->counts->released, 1);
    free(n);
}

struct race_work
{
    struct race_node *node;
    struct ow_gate *gate;
    enum ow_status status;
    unsigned failed;
};

static void *dropper(void *p)
{
    struct race_work *w = p;
    start(w->gate);
    w->status = ow_atomic_drop(&w->node->refs);
    return NULL;
}

static void *upgrader(void *p)
{
    struct race_work *w = p;
    start(w->gate);
    w->status = ow_atomic_upgrade(&w->node->refs);
    if (w->status == OW_OK)
    {
        if (atomic_load(&w->node->counts->destroyed) != 0 ||
            *w->node->payload != 41)
            w->failed = 1;
        if (ow_atomic_drop(&w->node->refs) != OW_OK)
            w->failed = 1;
    }
    else if (w->status != OW_DEAD)
        w->failed = 1;
    if (ow_atomic_weak_drop(&w->node->refs) != OW_OK)
        w->failed = 1;
    return NULL;
}

static int race(void)
{
    unsigned wins = 0, dead = 0;
    for (unsigned trial = 0; trial < 256; ++trial)
    {
        struct ow_allocator allocator = {0};
        struct atomic_counts counts;
        atomic_init(&counts.destroyed, 0);
        atomic_init(&counts.released, 0);
        struct race_node *n = malloc(sizeof(*n));
        CHECK(n);
        n->allocator = ow_allocator(&allocator);
        n->payload = n->allocator.take(n->allocator.context, 1);
        CHECK(n->payload);
        *n->payload = 41;
        n->counts = &counts;
        atomic_init(&n->refs.strong, 1);
        atomic_init(&n->refs.weak, 1);
        n->refs.destroy = race_destroy;
        n->refs.release = race_release;
        n->refs.context = n;
        CHECK(ow_atomic_downgrade(&n->refs) == OW_OK &&
              ow_atomic_downgrade(&n->refs) == OW_OK);
        struct ow_gate gate = {.mutex = PTHREAD_MUTEX_INITIALIZER,
                               .cond = PTHREAD_COND_INITIALIZER};
        struct race_work d = {n, &gate, OW_INVALID, 0},
                         u = {n, &gate, OW_INVALID, 0};
        pthread_t dt, ut;
        CHECK(pthread_create(&dt, NULL, dropper, &d) == 0 &&
              pthread_create(&ut, NULL, upgrader, &u) == 0);
        pthread_mutex_lock(&gate.mutex);
        while (gate.arrived != 2)
            pthread_cond_wait(&gate.cond, &gate.mutex);
        gate.go = 1;
        pthread_cond_broadcast(&gate.cond);
        pthread_mutex_unlock(&gate.mutex);
        CHECK(pthread_join(dt, NULL) == 0 && pthread_join(ut, NULL) == 0 &&
              d.status == OW_OK && !u.failed);
        if (u.status == OW_OK)
            ++wins;
        else
            ++dead;
        CHECK(atomic_load(&n->refs.strong) == 0 &&
              atomic_load(&n->refs.weak) == 1 &&
              atomic_load(&counts.destroyed) == 1 &&
              !atomic_load(&counts.released) && !allocator.live &&
              allocator.releases == 1);
        CHECK(ow_atomic_weak_drop(&n->refs) == OW_OK &&
              atomic_load(&counts.released) == 1);
        CHECK(pthread_cond_destroy(&gate.cond) == 0 &&
              pthread_mutex_destroy(&gate.mutex) == 0);
    }
    printf(
        "ORC_RACE synchronized-start rounds256 success%u dead%u actual-payload-control-reclamation\n",
        wins, dead);
    return 0;
}

static int oracle(void)
{
    char ops[16];
    while (scanf("%15s", ops) == 1)
    {
        for (unsigned mode = 0; mode < 2; ++mode)
        {
            struct ow_counts c = {0};
            struct ow_shared s = {1, 1, destroyed, released, &c};
            struct ow_atomic_shared a;
            atomic_init(&a.strong, 1);
            atomic_init(&a.weak, 1);
            a.destroy = destroyed;
            a.release = released;
            a.context = &c;
            if (mode)
                putchar('|');
            for (size_t i = 0; ops[i] && ops[i] != '-'; ++i)
            {
                enum ow_status q;
                switch (ops[i])
                {
                    case 'r':
                        q = mode ? ow_atomic_retain(&a) : ow_shared_retain(&s);
                        break;
                    case 'w':
                        q = mode ? ow_atomic_downgrade(&a)
                                 : ow_shared_downgrade(&s);
                        break;
                    case 'd':
                        q = mode ? ow_atomic_drop(&a) : ow_shared_drop(&s);
                        break;
                    case 'u':
                        q = mode ? ow_atomic_upgrade(&a)
                                 : ow_shared_upgrade(&s);
                        break;
                    case 'x':
                        q = mode ? ow_atomic_weak_drop(&a)
                                 : ow_shared_weak_drop(&s);
                        break;
                    default:
                        return 2;
                }
                printf("%u,%u,%u,%u,%d;",
                       mode ? atomic_load(&a.strong) : s.strong,
                       mode ? atomic_load(&a.weak) : s.weak, c.destroyed,
                       c.released, q);
            }
            while (mode ? atomic_load(&a.strong) : s.strong)
            {
                enum ow_status q =
                    mode ? ow_atomic_drop(&a) : ow_shared_drop(&s);
                if (q != OW_OK)
                    return 2;
            }
            while (mode ? atomic_load(&a.weak) : s.weak)
            {
                enum ow_status q =
                    mode ? ow_atomic_weak_drop(&a) : ow_shared_weak_drop(&s);
                if (q != OW_OK)
                    return 2;
            }
        }
        puts("");
    }
    return ferror(stdin) ? 2 : 0;
}

static int metrics(void)
{
    struct atomic_counts c;
    atomic_init(&c.destroyed, 0);
    atomic_init(&c.released, 0);
    struct ow_atomic_shared a;
    atomic_init(&a.strong, 1);
    atomic_init(&a.weak, 1);
    a.destroy = atomic_destroyed;
    a.release = atomic_released;
    a.context = &c;
    double times[7];
    unsigned sum = 0;
    for (unsigned s = 0; s < 7; ++s)
    {
        clock_t t = clock();
        for (unsigned i = 0; i < 10000; ++i)
        {
            CHECK(ow_atomic_retain(&a) == OW_OK && ow_atomic_drop(&a) == OW_OK);
            ++sum;
        }
        times[s] = (double)(clock() - t) / CLOCKS_PER_SEC;
    }
    CHECK(ow_atomic_drop(&a) == OW_OK && atomic_load(&c.destroyed) == 1 &&
          atomic_load(&c.released) == 1);
    printf(
        "{\"iterations\":10000,\"checksum\":%u,\"atomic_size\":%zu,\"atomic_alignment\":%zu,\"lock_free\":%d,\"seconds\":[",
        sum, sizeof(atomic_uint), _Alignof(atomic_uint),
        atomic_is_lock_free(&a.strong));
    for (unsigned i = 0; i < 7; ++i)
        printf("%s%.9f", i ? "," : "", times[i]);
    puts("]}");
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    if (!strcmp(argv[1], "references_base"))
        return base();
    if (!strcmp(argv[1], "references_overflow"))
        return overflow();
    if (!strcmp(argv[1], "references_cycle"))
        return cycle();
    if (!strcmp(argv[1], "references_failures"))
        return failures();
    if (!strcmp(argv[1], "references_threads"))
        return threads();
    if (!strcmp(argv[1], "references_race"))
        return race();
    if (!strcmp(argv[1], "oracle"))
        return oracle();
    if (!strcmp(argv[1], "metrics"))
        return metrics();
    return 2;
}
