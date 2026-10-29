#define _POSIX_C_SOURCE 200809L
#include "barrier_native.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#define CHECK(c)                                                 \
    do                                                           \
    {                                                            \
        if (!(c))                                                \
        {                                                        \
            fprintf(stderr, "CDBN_CHECK:%d:%s\n", __LINE__, #c); \
            return 1;                                            \
        }                                                        \
    } while (0)

enum
{
    ROUNDS = 32
};

struct cmx_world
{
    struct cdb_native central;
    struct cdb_tree tree;
    unsigned tree_mode, values[4];
};

struct cmx_worker
{
    struct cmx_world *world;
    unsigned id, reads;
    int failed;
};

static int wait_round(struct cmx_world *w, unsigned id)
{
    return w->tree_mode ? cdbt_wait(&w->tree, id) : cdbn_wait(&w->central, id);
}

static void *worker(void *arg)
{
    struct cmx_worker *p = arg;
    struct cmx_world *w = p->world;
    for (unsigned r = 0; r < ROUNDS; r++)
    {
        /* Rotate the delayed participant, with no publication in this delay. */
        if (p->id == r % 4)
        {
            struct timespec pause = {0, 1000000};
            while (nanosleep(&pause, &pause))
            {
            }
        }
        w->values[p->id] = 100 * r + p->id + 1;
        if (wait_round(w, p->id) != CMX_OK)
        {
            p->failed = 1;
            return 0;
        }
        for (unsigned j = 0; j < 4; j++)
            if (w->values[j] != 100 * r + j + 1)
            {
                p->failed = 2;
                return 0;
            }
            else
                p->reads++;
        /* Prevent next-round writes from racing with the preceding reads. */
        if (wait_round(w, p->id) != CMX_OK)
        {
            p->failed = 3;
            return 0;
        }
    }
    return 0;
}

static int preflight(void)
{
    struct cdb_native n;
    struct cdb_ticket t[2];
    unsigned out = 999;
    CHECK(cdbn_init(&n, 2) == CMX_OK);
    CHECK(cdb_arrive(n.member[0], cmx_forever(), &t[0]) == CMX_BLOCKED);
    CHECK(cdb_result(t[0], &out) == CMX_BLOCKED && out == 999);
    CHECK(cdb_arrive(n.member[1], cmx_forever(), &t[1]) == CMX_OK);
    CHECK(cdb_result(t[0], &out) == CMX_OK &&
          cdb_result(t[1], &out) == CMX_OK && out == 0);
    CHECK(cdbn_destroy(&n) == CMX_OK);
    return 0;
}

int main(int argc, char **argv)
{
    CHECK(argc == 2 &&
          (!strcmp(argv[1], "central") || !strcmp(argv[1], "tree")));
    CHECK(preflight() == 0);
    struct cmx_world w = {.tree_mode = !strcmp(argv[1], "tree")};
    CHECK((w.tree_mode ? cdbt_init(&w.tree) : cdbn_init(&w.central, 4)) ==
          CMX_OK);
    pthread_t threads[4];
    struct cmx_worker workers[4];
    for (unsigned i = 0; i < 4; i++)
    {
        workers[i] = (struct cmx_worker){.world = &w, .id = i};
        CHECK(!pthread_create(&threads[i], 0, worker, &workers[i]));
    }
    for (unsigned i = 0; i < 4; i++)
        CHECK(!pthread_join(threads[i], 0) && !workers[i].failed &&
              workers[i].reads == 128);
    if (w.tree_mode)
    {
        CHECK(w.tree.leaf[0].arrivals == 256 &&
              w.tree.leaf[1].arrivals == 256 && w.tree.root.arrivals == 128);
        CHECK(w.tree.leaf[0].barrier.raw.generation == 128 &&
              w.tree.leaf[1].barrier.raw.generation == 128 &&
              w.tree.root.barrier.raw.generation == 64);
        CHECK(cdbt_destroy(&w.tree) == CMX_OK);
        puts(
            "CDBN_TREE:rounds32;plain-reads512;leaf-arrivals256+256;root-arrivals128;gather-root-release;reuse-fence");
    }
    else
    {
        CHECK(w.central.arrivals == 256 &&
              w.central.barrier.raw.generation == 64 &&
              cdbn_destroy(&w.central) == CMX_OK);
        puts(
            "CDBN_CENTRAL:rounds32;plain-reads512;arrivals256;generations64;rotating-delayed-participant;reuse-fence");
    }
    return 0;
}
