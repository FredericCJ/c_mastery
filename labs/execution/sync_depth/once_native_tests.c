#define _POSIX_C_SOURCE 200809L
/* The serialized semantic preflight rejects flawed adapters before introducing
   real threads, so an intentional publication mutant is not executed as a race. */
#define main cdo_contract_main
#include "once_tests.c"
#undef main
#include <pthread.h>
#include <stdlib.h>
#undef CHECK
#define CHECK(c)                                                 \
    do                                                           \
    {                                                            \
        if (!(c))                                                \
        {                                                        \
            fprintf(stderr, "CDON_CHECK:%d:%s\n", __LINE__, #c); \
            return 1;                                            \
        }                                                        \
    } while (0)

struct cmx_native
{
    cmx_kernel kernel;
    struct cd_once once;
    struct cdo_handle handle[3];
    pthread_mutex_t lock;
    pthread_cond_t changed;
    unsigned waiting, fail, calls, drops, live, owner;
};

struct native_worker
{
    struct cmx_native *native;
    unsigned id;
    int failed;
    struct cdo_table observed;
};

static struct cdo_cell *heap_alloc(void *ctx)
{
    struct cmx_native *n = ctx;
    struct cdo_cell *p = malloc(sizeof *p);
    n->calls++;
    if (p)
    {
        n->live++;
    }
    return p;
}

static void heap_drop(void *ctx, struct cdo_cell *p)
{
    struct cmx_native *n = ctx;
    free(p);
    n->drops++;
    n->live--;
}

static void *native_worker(void *arg)
{
    struct native_worker *w = arg;
    struct cmx_native *n = w->native;
    struct cdo_ticket ticket;
    if (pthread_mutex_lock(&n->lock))
    {
        w->failed = 1;
        return 0;
    }
    int r = cdo_enter(n->handle[w->id], cmx_forever(), &ticket);
    if (r == CDO_INIT)
    {
        n->owner = w->id;
        /* Let both real contenders enroll before the terminal publication. */
        while (n->waiting < 2)
        {
            if (pthread_cond_wait(&n->changed, &n->lock))
            {
                w->failed = 2;
                return 0;
            }
        }
        if (cdo_allocate(n->handle[w->id]) != CMX_OK)
        {
            w->failed = 3;
            return 0;
        }
        for (unsigned i = 0; i < 8; i++)
        {
            if (cdo_write(n->handle[w->id], i, 17 * i + 1) != CMX_OK)
            {
                w->failed = 4;
                return 0;
            }
            if (n->fail && i == 3)
            {
                break;
            }
        }
        if (!n->fail && (cdo_seal(n->handle[w->id]) != CMX_OK ||
                         cdo_seal(n->handle[w->id]) != CMX_OK))
        {
            w->failed = 5;
            return 0;
        }
        if (cdo_finish(n->handle[w->id], !n->fail) != CMX_OK ||
            pthread_cond_broadcast(&n->changed))
        {
            w->failed = 6;
            return 0;
        }
    }
    else if (r == CMX_BLOCKED)
    {
        n->waiting++;
        if (pthread_cond_broadcast(&n->changed))
        {
            w->failed = 7;
            return 0;
        }
    }
    else
    {
        w->failed = 8;
        return 0;
    }
    while ((r = cdo_result(ticket, &w->observed)) == CMX_BLOCKED)
    {
        if (pthread_cond_wait(&n->changed, &n->lock))
        {
            w->failed = 9;
            return 0;
        }
    }
    if (r != (n->fail ? CDO_FAILED : CMX_OK))
    {
        w->failed = 10;
    }
    if (pthread_mutex_unlock(&n->lock))
    {
        w->failed = 11;
    }
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2 ||
        (!strcmp(argv[1], "success") ? 0 : strcmp(argv[1], "failure")))
    {
        return 2;
    }
    const char *cases[] = {"publication", "failure",  "retry",
                           "waiting",     "policies", "limits"};
    for (unsigned i = 0; i < 6; i++)
    {
        char *args[] = {"preflight", (char *)cases[i]};
        CHECK(cdo_contract_main(2, args) == 0);
    }
    unsigned rounds = 30;
    for (unsigned round = 0; round < rounds; round++)
    {
        struct cmx_native n = {.fail = !strcmp(argv[1], "failure"),
                               .owner = 99};
        struct native_worker workers[3];
        pthread_t threads[3];
        cmx_init(&n.kernel, CMX_FIXED);
        for (unsigned i = 0; i < 3; i++)
        {
            CHECK(cmx_task_add(&n.kernel, 5 + i, 0, 0x1000 + 256 * i, 256) ==
                  (int)i);
        }
        CHECK(cdo_init(&n.once, &n.kernel, heap_alloc, heap_drop, &n) ==
              CMX_OK);
        for (unsigned i = 0; i < 3; i++)
        {
            CHECK(cdo_register(&n.once, i, &n.handle[i]) == CMX_OK);
        }
        CHECK(!pthread_mutex_init(&n.lock, 0) &&
              !pthread_cond_init(&n.changed, 0));
        for (unsigned j = 0; j < 3; j++)
        {
            unsigned i = (j + round) % 3;
            workers[i] = (struct native_worker){
                .native = &n, .id = i, .observed = {.count = 999}};
            CHECK(!pthread_create(&threads[i], 0, native_worker, &workers[i]));
        }
        for (unsigned i = 0; i < 3; i++)
        {
            CHECK(!pthread_join(threads[i], 0) && !workers[i].failed);
            CHECK(workers[i].observed.count == (n.fail ? 999u : 8u));
            if (!n.fail)
            {
                CHECK(workers[i].observed.checksum == 484);
            }
        }
        CHECK(n.once.attempts == 1 && n.waiting == 2 && n.owner < 3 &&
              n.calls == 3 && n.live == (n.fail ? 0u : 3u));
        struct cdo_table later = {.count = 998};
        CHECK(cdo_read(n.handle[0], &later) == (n.fail ? CDO_FAILED : CMX_OK));
        CHECK(later.count == (n.fail ? 998u : 8u));
        CHECK(cdo_close(&n.once) == CMX_OK);
        for (unsigned i = 0; i < 3; i++)
        {
            CHECK(cdo_release(&n.handle[i]) == CMX_OK);
        }
        CHECK(cdo_destroy(&n.once) == CMX_OK && n.live == 0 && n.drops == 3);
        CHECK(!pthread_cond_destroy(&n.changed) &&
              !pthread_mutex_destroy(&n.lock));
    }
    printf(
        "CDON_NATIVE:mode=%s;rounds=%u;threads90;attempt1-per-round;two-enrolled-waiters;delayed-reader;heap-cleanup\n",
        argv[1], rounds);
    return 0;
}
