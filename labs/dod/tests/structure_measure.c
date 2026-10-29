#define _POSIX_C_SOURCE 200809L
#include "dod_entities.h"
#include "dod_persistent.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
static uint64_t copies;

void d_structure_copy(size_t n)
{
#ifdef D_STRUCTURE_OBSERVE
    copies += n;
#else
    (void)n;
#endif
}

#define CHECK(x)                                                              \
    do                                                                        \
    {                                                                         \
        if (!(x))                                                             \
        {                                                                     \
            fprintf(stderr, "structure measurement check %d: %s\n", __LINE__, \
                    #x);                                                      \
            exit(1);                                                          \
        }                                                                     \
    } while (0)

static uint64_t now(void)
{
    struct timespec t;
    CHECK(clock_gettime(CLOCK_MONOTONIC, &t) == 0);
    return (uint64_t)t.tv_sec * UINT64_C(1000000000) + (uint64_t)t.tv_nsec;
}

struct d_component
{
    unsigned ids[4], rank[4], count;
    int values[4];
};

struct d_measure_sparse
{
    struct d_sparse ids;
    struct d_component position, velocity;
    unsigned mask[4], version;
};

static void component_init(struct d_component *c)
{
    *c = (struct d_component){0};
    for (unsigned i = 0; i < 4; ++i)
    {
        c->rank[i] = 4;
    }
}

static void component_set(struct d_component *c, unsigned id, int present,
                          int value)
{
    unsigned at = c->rank[id];
    if (present)
    {
        if (at == 4)
        {
            at = c->count++;
            c->rank[id] = at;
            c->ids[at] = id;
            d_structure_copy(sizeof(id));
        }
        c->values[at] = value;
        d_structure_copy(sizeof(value));
    }
    else if (at != 4)
    {
        unsigned last = --c->count;
        if (at != last)
        {
            unsigned moved = c->ids[last];
            c->ids[at] = moved;
            c->values[at] = c->values[last];
            c->rank[moved] = at;
            d_structure_copy(sizeof(unsigned) + sizeof(int));
        }
        c->rank[id] = 4;
    }
}

static void sparse_init(struct d_measure_sparse *s, struct d_handle handles[2])
{
    *s = (struct d_measure_sparse){0};
    d_sparse_init(&s->ids);
    component_init(&s->position);
    component_init(&s->velocity);
    s->version = 1;
    for (unsigned id = 0; id < 2; ++id)
    {
        CHECK(d_sparse_add(&s->ids, 0, &handles[id]) == D_OK);
        s->mask[id] = id + 1;
        component_set(&s->position, id, (id + 1) & 1, (int)id + 1);
        component_set(&s->velocity, id, (id + 1) & 2, (int)id + 2);
        ++s->version;
    }
}

static void sparse_set(struct d_measure_sparse *s, struct d_handle h,
                       unsigned mask)
{
    int unused;
    CHECK(d_sparse_get(&s->ids, h, &unused) == D_OK);
    component_set(&s->position, h.slot, mask & 1, (int)h.slot + 1);
    component_set(&s->velocity, h.slot, mask & 2, (int)h.slot + 2);
    s->mask[h.slot] = mask;
    ++s->version;
}

static int sparse_query(const struct d_measure_sparse *s)
{
    int total = 0;
    for (unsigned row = 0; row < s->position.count; ++row)
    {
        unsigned id = s->position.ids[row], v = s->velocity.rank[id];
        if (v < 4)
        {
            total += s->position.values[row] + s->velocity.values[v];
        }
    }
    return total;
}

static void ecs_work(unsigned sparse, unsigned ratio, unsigned edits,
                     unsigned sample, int observe)
{
    struct d_entity_world w;
    struct d_handle handles[2];
    struct d_entity_query query = {0};
    struct d_measure_sparse s;
    if (sparse)
    {
        sparse_init(&s, handles);
    }
    else
    {
        d_entity_init(&w);
        for (unsigned id = 0; id < 2; ++id)
        {
            CHECK(d_entity_create(&w, id + 1, (int)id + 1, (int)id + 2,
                                  &handles[id]) == D_OK);
        }
    }
    copies = 0;
    uint64_t checksum = 0, misses = 0, start = now();
    for (unsigned edit = 0; edit < edits; ++edit)
    {
        unsigned id = edit % 2, mask = (edit / 2) % 3 + 1;
        if (sparse)
        {
            sparse_set(&s, handles[id], mask);
        }
        else
        {
            struct d_entity_command c = {handles[id], mask, (int)id + 1,
                                         (int)id + 2};
            CHECK(d_entity_apply(&w, &c, 1) == D_OK);
        }
        for (unsigned q = 0; q < ratio; ++q)
        {
            if (sparse)
            {
                checksum += (unsigned)sparse_query(&s);
            }
            else
            {
                misses += query.query.version != w.world.version;
                CHECK(d_entity_query(&w, 3, 0, &query) == D_OK);
                int total = 0;
                for (unsigned i = 0; i < query.query.count; ++i)
                {
                    unsigned entity = query.query.ids[i],
                             row = w.world.ecs.row[entity];
                    total += w.world.ecs.tables[2].position[row] +
                             w.world.ecs.tables[2].velocity[row];
                }
                checksum += (unsigned)total;
            }
        }
    }
    uint64_t elapsed = now() - start;
    printf("%c %u %u %u %u %llu %llu %llu %llu\n", observe ? 'O' : 'E', sparse,
           ratio, edits, sample, (unsigned long long)elapsed,
           (unsigned long long)checksum, (unsigned long long)copies,
           (unsigned long long)misses);
}

struct text_pool
{
    unsigned char bytes[16][32];
    unsigned used[16], clones, drops;
};

static int clone(void *context, const unsigned char *text, size_t n,
                 unsigned char **out)
{
    struct text_pool *p = context;
    for (unsigned i = 0; i < 16; ++i)
    {
        if (!p->used[i])
        {
            p->used[i] = 1;
            for (size_t j = 0; j < n; ++j)
            {
                p->bytes[i][j] = text[j];
            }
            ++p->clones;
            *out = p->bytes[i];
            return 1;
        }
    }
    return 0;
}

static void drop(void *context, unsigned char *text)
{
    struct text_pool *p = context;
    for (unsigned i = 0; i < 16; ++i)
    {
        if (text == p->bytes[i])
        {
            CHECK(p->used[i]);
            p->used[i] = 0;
            ++p->drops;
            return;
        }
    }
    CHECK(0);
}

static void tree_work(unsigned budget, unsigned sample)
{
    uint64_t elapsed = 0;
    unsigned total_calls = 0, total_reclaimed = 0, total_drops = 0,
             max_pending = 0;
    for (unsigned repeat = 0; repeat < 1000; ++repeat)
    {
        struct text_pool pool = {0};
        struct d_rc_tree tree;
        d_rc_init(&tree, (struct d_text_ops){&pool, clone, drop});
        unsigned root = 0;
        for (unsigned i = 0; i < 8; ++i)
        {
            unsigned next;
            unsigned char byte = (unsigned char)i;
            CHECK(d_rc_update(&tree, root, i, &byte, 1, &next) == D_OK);
            CHECK(d_rc_release(&tree, root) == D_OK);
            root = next;
        }
        CHECK(tree.live == 15 && pool.clones == 8);
        tree.reclaimed = 0;
        pool.drops = 0;
        struct d_rc_retire work = {0};
        uint64_t start = now();
        if (!budget)
        {
            CHECK(d_rc_release(&tree, root) == D_OK);
            ++total_calls;
        }
        else
        {
            CHECK(d_rc_retire(&tree, root, &work) == D_OK);
            unsigned done = 0;
            while (!done)
            {
                if (work.count > max_pending)
                {
                    max_pending = work.count;
                }
                CHECK(d_rc_drain(&work, budget, &done) == D_OK);
                ++total_calls;
            }
        }
        elapsed += now() - start;
        CHECK(tree.live == 0 && tree.reclaimed == 15 && pool.drops == 8);
        total_reclaimed += tree.reclaimed;
        total_drops += pool.drops;
    }
    printf("R %u %u %llu %u %u %u %u\n", budget, sample,
           (unsigned long long)elapsed, total_calls, total_reclaimed,
           total_drops, max_pending);
}

static void census(void)
{
    struct d_tree tree;
    d_tree_init(&tree);
    unsigned roots[8], root = 0;
    for (unsigned i = 0; i < 8; ++i)
    {
        CHECK(d_tree_update(&tree, root, i, (int)i, &root) == D_OK);
        roots[i] = root;
    }
    unsigned all, shared, latest, unused;
    CHECK(d_tree_census(&tree, roots, 8, &all, &shared) == D_OK &&
          d_tree_census(&tree, &root, 1, &latest, &unused) == D_OK);
    printf("C %u %u %u %u %zu %zu %zu\n", tree.used, all, latest, shared,
           sizeof(struct d_tree_node), sizeof(struct d_rc_node),
           sizeof(struct d_rc_retire));
}

int main(int argc, char **argv)
{
    CHECK(argc == 2);
    if (!strcmp(argv[1], "observe"))
    {
        for (unsigned mode = 0; mode < 2; ++mode)
        {
            for (unsigned ratio = 1; ratio <= 16; ratio *= 16)
            {
                ecs_work(mode, ratio, 12, 0, 1);
            }
        }
        census();
    }
    else if (!strcmp(argv[1], "time"))
    {
        for (unsigned sample = 0; sample < 7; ++sample)
        {
            for (unsigned ratio = 1; ratio <= 256; ratio *= 16)
            {
                for (unsigned j = 0; j < 2; ++j)
                {
                    ecs_work((j + sample) % 2, ratio, 6000, sample, 0);
                }
            }
            for (unsigned j = 0; j < 4; ++j)
            {
                unsigned index = (j + sample) % 4;
                tree_work(index ? 1u << (index - 1) : 0, sample);
            }
        }
    }
    else
    {
        CHECK(0);
    }
    return 0;
}
