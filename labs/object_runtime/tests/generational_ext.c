#include "object_runtime_generational.h"
#include <stdio.h>
#include <string.h>
#define CHECK(c)                                               \
    do                                                         \
    {                                                          \
        if (!(c))                                              \
        {                                                      \
            fprintf(stderr, "OR_CHECK:%d:%s\n", __LINE__, #c); \
            return 1;                                          \
        }                                                      \
    } while (0)

static const struct or_cell *cell(const struct or_nursery *h, unsigned r)
{
    return r <= 8 ? &h->young[h->active][r - 1] : &h->old[r - 9];
}

static int equivalent(const struct or_nursery *a, const struct or_nursery *b)
{
    unsigned i, j;
    if (a->used != b->used || a->old_used != b->old_used ||
        memcmp(a->roots, b->roots, 4))
        return 0;
    for (i = 0; i < a->used + a->old_used; i++)
    {
        unsigned r = i < a->used ? i + 1 : i - a->used + 9;
        const struct or_cell *x = cell(a, r), *y = cell(b, r);
        if (x->value != y->value || x->age != y->age)
            return 0;
        for (j = 0; j < 2; j++)
            if (x->edge[j] != y->edge[j])
                return 0;
    }
    return 1;
}

static struct or_nursery example(void)
{
    struct or_nursery h = {0};
    unsigned i;
    h.used = 3;
    h.old_used = 1;
    h.roots[0] = 3;
    for (i = 0; i < 3; i++)
        h.young[0][i].value = (int32_t)(101 + i);
    h.young[0][2].edge[0] = 2;
    h.old[0].value = 201;
    h.old[0].edge[0] = 2;
    return h;
}

static int preflight(void)
{
    struct or_nursery h = example(), reference = h, before;
    struct or_cards cards, card_before;
    unsigned copied, scanned, wanted;
    CHECK(or_nursery_cards_init(&h, &cards) == OR_OK);
    CHECK(or_minor(&reference, 8, 8, &wanted) == OR_OK);
    CHECK(or_minor_preflight(&h, &cards, 8, 8, &copied, &scanned) == OR_OK &&
          copied == 2 && scanned == 4);
    CHECK(equivalent(&h, &reference) && h.roots[0] == 1 &&
          h.old[0].edge[0] == 2);
    memcpy(&before, &h, sizeof h);
    memcpy(&card_before, &cards, sizeof cards);
    copied = 777;
    scanned = 888;
    CHECK(or_minor_preflight(&h, &cards, 8, 2, &copied, &scanned) == OR_FULL);
    CHECK(!memcmp(&before, &h, sizeof h) &&
          !memcmp(&card_before, &cards, sizeof cards) && copied == 777 &&
          scanned == 888);
    CHECK(or_minor(&reference, 8, 8, &wanted) == OR_OK);
    CHECK(or_minor_preflight(&h, &cards, 8, 8, &copied, &scanned) == OR_OK &&
          copied == 2 && equivalent(&h, &reference));
    CHECK(h.used == 0 && h.old_used == 3 && h.roots[0] == 10 &&
          h.old[0].edge[0] == 11);
    h = example();
    h.roots[0] = 0;
    reference = h;
    CHECK(or_minor(&reference, 8, 8, &wanted) == OR_OK);
    CHECK(or_minor_preflight(&h, NULL, 8, 8, &copied, &scanned) == OR_OK &&
          copied == 1 && scanned == 2 && equivalent(&h, &reference));
    h = example();
    memcpy(&before, &h, sizeof h);
    copied = 777;
    scanned = 888;
    CHECK(or_minor_preflight(&h, NULL, 0, 8, &copied, &scanned) == OR_FULL &&
          !memcmp(&before, &h, sizeof h) && copied == 777 && scanned == 888);
    return 0;
}

static int composition(void)
{
    struct or_nursery h = {0}, reference, before;
    struct or_cards cards, cb;
    unsigned copied, scanned, n, i;
    uint8_t forward[16];
    h.used = 7;
    h.old_used = 4;
    for (i = 0; i < 7; i++)
        h.young[0][i].value = (int32_t)(i + 1);
    for (i = 0; i < 4; i++)
        h.old[i].value = (int32_t)(101 + i);
    CHECK(or_nursery_cards_init(&h, &cards) == OR_OK && cards.dirty == 0);
    CHECK(or_nursery_old_store(&h, &cards, 3, 1, 5) == OR_OK);
    CHECK(or_nursery_old_store(&h, &cards, 2, 0, 7) == OR_OK &&
          cards.dirty == 2);
    reference = h;
    CHECK(or_minor(&reference, 8, 8, &n) == OR_OK);
    CHECK(or_minor_preflight(&h, &cards, 8, 8, &copied, &scanned) == OR_OK &&
          copied == 2 && scanned == 4 && equivalent(&h, &reference));
    CHECK(h.old[2].edge[0] == 1 && h.old[3].edge[1] == 2 && cards.dirty == 2);
    CHECK(or_nursery_old_store(&h, &cards, 3, 1, 0) == OR_OK &&
          cards.dirty == 2);
    CHECK(or_minor_preflight(&h, &cards, 8, 8, &copied, &scanned) == OR_OK &&
          copied == 1 && h.used == 0 && h.old_used == 5 && cards.dirty == 0);
    CHECK(h.old[2].edge[0] == 13);
    h.roots[0] = 11;
    CHECK(or_major_preflight(&h, &cards, &n, forward) == OR_OK && n == 3 &&
          h.old_used == 2 && h.roots[0] == 9 && h.old[0].value == 103 &&
          h.old[0].edge[0] == 10 && h.old[1].value == 7);
    CHECK(forward[10] == 9 && forward[12] == 10 && cards.dirty == 0);
    h.roots[0] = 0;
    CHECK(or_major_preflight(&h, &cards, &n, forward) == OR_OK && n == 2 &&
          h.old_used == 0);
    h = example();
    CHECK(or_nursery_cards_init(&h, &cards) == OR_OK);
    memcpy(&before, &h, sizeof h);
    memcpy(&cb, &cards, sizeof cards);
    CHECK(or_nursery_old_store(&h, &cards, 0, 2, 0) == OR_INVALID &&
          !memcmp(&before, &h, sizeof h) && !memcmp(&cb, &cards, sizeof cards));
    h = (struct or_nursery){0};
    h.used = 1;
    h.young[0][0] = (struct or_cell){.value = 77, .age = 1};
    h.old_used = 8;
    h.roots[0] = 9;
    h.roots[1] = 1;
    for (i = 0; i < 8; i++)
    {
        h.old[i].value = (int32_t)(100 + i);
        h.old[i].age = 1;
        if (i < 7)
            h.old[i].edge[0] = (uint8_t)(10 + i);
    }
    CHECK(or_nursery_cards_init(&h, &cards) == OR_OK);
    memcpy(&before, &h, sizeof h);
    CHECK(or_minor_preflight(&h, &cards, 8, 8, &copied, &scanned) == OR_FULL &&
          !memcmp(&before, &h, sizeof h));
    h.roots[0] = 0;
    CHECK(or_major_preflight(&h, &cards, &n, forward) == OR_OK && n == 8 &&
          h.old_used == 0 && h.used == 1);
    CHECK(or_minor_preflight(&h, &cards, 8, 8, &copied, &scanned) == OR_OK &&
          copied == 1 && h.used == 0 && h.old_used == 1 &&
          h.old[0].value == 77);
    return 0;
}

static int large(void)
{
    struct or_large_heap h, before;
    struct or_large_handle objects[4], out;
    struct or_large_lease lease, copy;
    const uint32_t *data;
    unsigned n, i;
    or_large_init(&h);
    for (i = 0; i < 4; i++)
        CHECK(or_large_allocate(&h, 2, 10 + i, &objects[i]) == OR_OK);
    CHECK(or_large_pin(&h, objects[0], &lease, &data, &n) == OR_OK && n == 2 &&
          data[0] == 10 && data[1] == 10);
    copy = lease;
    CHECK(or_large_release(&h, objects[0]) == OR_OK &&
          or_large_release(&h, objects[2]) == OR_OK);
    CHECK(or_large_collect(&h, &n) == OR_OK && n == 1 && h.block[0].active &&
          data[0] == 10 && data[1] == 10);
    CHECK(or_large_unpin(lease) == OR_OK && or_large_unpin(copy) == OR_STATE);
    CHECK(or_large_collect(&h, &n) == OR_OK && n == 1 && !h.block[0].active);
    memcpy(&before, &h, sizeof h);
    out = (struct or_large_handle){99, 99};
    CHECK(or_large_allocate(&h, 3, 99, &out) == OR_FULL && out.slot == 99 &&
          !memcmp(&before, &h, sizeof h));
    CHECK(or_large_release(&h, objects[1]) == OR_OK &&
          or_large_collect(&h, &n) == OR_OK && n == 1);
    CHECK(or_large_allocate(&h, 4, 99, &out) == OR_OK &&
          h.block[out.slot - 1].start == 0 && out.generation == 2);
    CHECK(or_large_release(&h, objects[0]) == OR_INVALID);
    CHECK(or_large_release(&h, objects[3]) == OR_OK &&
          or_large_release(&h, out) == OR_OK);
    CHECK(or_large_collect(&h, &n) == OR_OK && n == 2 && h.heap.old_used == 0);
    or_large_init(&h);
    for (i = 0; i < 12; i++)
    {
        CHECK(or_large_allocate(&h, 1, 55, &out) == OR_OK);
        CHECK(out.slot == i / 3 + 1 && out.generation == i % 3 + 1);
        CHECK(or_large_pin(&h, out, &lease, &data, &n) == OR_OK);
        CHECK(or_large_unpin(lease) == OR_OK &&
              or_large_release(&h, out) == OR_OK);
        CHECK(or_large_collect(&h, &n) == OR_OK && n == 1);
    }
    memcpy(&before, &h, sizeof h);
    out = (struct or_large_handle){99, 99};
    CHECK(or_large_allocate(&h, 1, 55, &out) == OR_FULL && out.slot == 99 &&
          !memcmp(&before, &h, sizeof h));
    /* The descriptor moves left while the saved native payload span stays fixed. */
    or_large_init(&h);
    CHECK(or_large_allocate(&h, 2, 71, &objects[0]) == OR_OK);
    CHECK(or_large_allocate(&h, 2, 92, &objects[1]) == OR_OK);
    CHECK(h.heap.old[1].value == 1001);
    CHECK(or_large_pin(&h, objects[1], &lease, &data, &n) == OR_OK &&
          data == &h.words[2] && n == 2);
    CHECK(or_large_release(&h, objects[0]) == OR_OK &&
          or_large_release(&h, objects[1]) == OR_OK);
    CHECK(or_large_collect(&h, &n) == OR_OK && n == 1 && h.heap.old_used == 1 &&
          h.heap.old[0].value == 1001 && data == &h.words[2] && data[0] == 92 &&
          data[1] == 92);
    CHECK(or_large_unpin(lease) == OR_OK && or_large_collect(&h, &n) == OR_OK &&
          n == 1 && h.heap.old_used == 0 && h.words[2] == 0 && h.words[3] == 0);
    return 0;
}

static void normalized(const struct or_nursery *h, unsigned *live,
                       unsigned *young, unsigned roots[4], unsigned edges[4])
{
    unsigned i;
    *live = *young = 0;
    for (i = 0; i < 4; i++)
    {
        roots[i] = h->roots[i] ? (unsigned)cell(h, h->roots[i])->value : 0;
        edges[i] = 0;
    }
    for (i = 0; i < h->used + h->old_used; i++)
    {
        unsigned r = i < h->used ? i + 1 : i - h->used + 9;
        const struct or_cell *p = cell(h, r);
        *live |= 1u << ((unsigned)p->value - 1);
        if (r <= 8)
            *young |= 1u << ((unsigned)p->value - 1);
        edges[p->value - 1] =
            p->edge[0] ? (unsigned)cell(h, p->edge[0])->value : 0;
    }
}

static int oracle(void)
{
    unsigned graph, mask, target, ages, i, j;
    for (graph = 0; graph < 64; graph++)
        for (mask = 0; mask < 16; mask++)
            for (target = 0; target < 4; target++)
                for (ages = 0; ages < 8; ages++)
                {
                    struct or_nursery h = {0}, reference;
                    struct or_cards cards;
                    unsigned code = graph, copied, scanned, wanted, n, live,
                             young, roots[4], edges[4];
                    uint8_t f[16];
                    h.used = 3;
                    h.old_used = 1;
                    h.old[0].value = 4;
                    h.old[0].age = 1;
                    h.old[0].edge[0] = (uint8_t)target;
                    for (i = 0; i < 3; i++)
                    {
                        h.young[0][i].value = (int32_t)(i + 1);
                        h.young[0][i].age = (uint8_t)((ages >> i) & 1);
                        h.young[0][i].edge[0] = (uint8_t)(code % 4);
                        code /= 4;
                    }
                    for (i = 0, j = 0; i < 4; i++)
                        if (mask & (1u << i))
                            h.roots[j++] = (uint8_t)(i == 3 ? 9 : i + 1);
                    reference = h;
                    CHECK(or_nursery_cards_init(&h, &cards) == OR_OK);
                    CHECK(or_minor(&reference, 8, 8, &wanted) == OR_OK);
                    CHECK(or_minor_preflight(&h, &cards, 8, 8, &copied,
                                             &scanned) == OR_OK &&
                          copied == wanted && equivalent(&h, &reference));
                    normalized(&h, &live, &young, roots, edges);
                    printf("N %u %u %u %u %u %u %u", graph, mask, target, ages,
                           live, young, copied);
                    for (i = 0; i < 4; i++)
                        printf(" %u", roots[i]);
                    for (i = 0; i < 4; i++)
                        printf(" %u", edges[i]);
                    putchar('\n');
                    CHECK(or_major_preflight(&h, &cards, &n, f) == OR_OK);
                    normalized(&h, &live, &young, roots, edges);
                    printf("J %u %u %u %u %u %u %u", graph, mask, target, ages,
                           live, young, n);
                    for (i = 0; i < 4; i++)
                        printf(" %u", roots[i]);
                    for (i = 0; i < 4; i++)
                        printf(" %u", edges[i]);
                    putchar('\n');
                }
    return 0;
}

static int large_oracle(void)
{
    unsigned code, release, pins, i;
    for (code = 0; code < 81; code++)
        for (release = 0; release < 16; release++)
            for (pins = 0; pins < 16; pins++)
            {
                struct or_large_heap h;
                struct or_large_handle objects[4];
                struct or_large_lease leases[4];
                unsigned word = code, success = 0, leased = 0, live = 0,
                         used = 0, reclaimed, n;
                int status[4];
                or_large_init(&h);
                for (i = 0; i < 4; i++)
                {
                    unsigned len = 1 + word % 3;
                    word /= 3;
                    status[i] = or_large_allocate(&h, len, 10 + i, &objects[i]);
                    CHECK(status[i] == OR_OK || status[i] == OR_FULL);
                    if (status[i] == OR_OK)
                        success |= 1u << i;
                }
                for (i = 0; i < 4; i++)
                    if (success & (1u << i))
                    {
                        if (pins & (1u << i))
                        {
                            const uint32_t *data;
                            CHECK(or_large_pin(&h, objects[i], &leases[i],
                                               &data, &n) == OR_OK &&
                                  data[0] == 10 + i);
                            leased |= 1u << i;
                        }
                        if (release & (1u << i))
                            CHECK(or_large_release(&h, objects[i]) == OR_OK);
                    }
                CHECK(or_large_collect(&h, &reclaimed) == OR_OK);
                for (i = 0; i < 4; i++)
                    if (success & (1u << i))
                    {
                        struct or_large_block *b =
                            &h.block[objects[i].slot - 1];
                        unsigned j;
                        if (b->active)
                        {
                            live |= 1u << i;
                            for (j = 0; j < b->length; j++)
                                used |= 1u << (b->start + j);
                        }
                    }
                printf("L %u %u %u %d %d %d %d %u %u %u\n", code, release, pins,
                       status[0], status[1], status[2], status[3], live, used,
                       reclaimed);
                for (i = 0; i < 4; i++)
                {
                    if (leased & (1u << i))
                        CHECK(or_large_unpin(leases[i]) == OR_OK);
                    if ((success & (1u << i)) && !(release & (1u << i)))
                        CHECK(or_large_release(&h, objects[i]) == OR_OK);
                }
                CHECK(or_large_collect(&h, &n) == OR_OK &&
                      h.heap.old_used == 0);
            }
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    if (!strcmp(argv[1], "preflight"))
        return preflight();
    if (!strcmp(argv[1], "composition"))
        return composition();
    if (!strcmp(argv[1], "large"))
        return large();
    if (!strcmp(argv[1], "oracle"))
        return oracle();
    if (!strcmp(argv[1], "large-oracle"))
        return large_oracle();
    return 2;
}
