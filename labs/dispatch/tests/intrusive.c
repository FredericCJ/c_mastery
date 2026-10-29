#include "dispatch_intrusive.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>
static const char *test;
#define CHECK(x)                                                        \
    do                                                                  \
    {                                                                   \
        if (!(x))                                                       \
        {                                                               \
            fprintf(stderr, "DS_CHECK:%s:%d:%s\n", test, __LINE__, #x); \
            return 1;                                                   \
        }                                                               \
    } while (0)

static int add(int a, int b, int *out)
{
    if ((b > 0 && a > INT_MAX - b) || (b < 0 && a < INT_MIN - b))
    {
        return DS_RANGE;
    }
    *out = a + b;
    return DS_OK;
}

static int digits(int a, int b, int *out)
{
    *out = a * 5 + b;
    return DS_OK;
}

static int fail(int a, int b, int *out)
{
    (void)a;
    (void)b;
    *out = 123;
    return DS_FAILED;
}

struct ds_model
{
    unsigned values[4], n;
};

static int position(const struct ds_model *m, unsigned i)
{
    for (unsigned n = 0; n < m->n; ++n)
    {
        if (m->values[n] == i)
        {
            return (int)n;
        }
    }
    return -1;
}

static int model(struct ds_model *m, unsigned i, unsigned remove)
{
    int at = position(m, i);
    if (remove)
    {
        if (at < 0)
        {
            return DS_STALE;
        }
        for (unsigned n = (unsigned)at; n + 1 < m->n; ++n)
        {
            m->values[n] = m->values[n + 1];
        }
        --m->n;
        return DS_OK;
    }
    if (at >= 0)
    {
        return DS_BUSY;
    }
    m->values[m->n++] = i;
    return DS_OK;
}

static int dual_check(const struct ds_dual_list *l,
                      const struct ds_dual_item items[4],
                      const struct ds_model *m)
{
    CHECK(l->count == m->n);
    const struct ds_hook *h = &l->head;
    for (unsigned n = 0; n < m->n; ++n)
    {
        CHECK(h->next == &items[m->values[n]].hooks[l->kind]);
        CHECK(h->next->previous == h);
        h = h->next;
    }
    CHECK(h->next == &l->head && l->head.previous == h);
    for (unsigned n = m->n; n > 0; --n)
    {
        CHECK(h == &items[m->values[n - 1]].hooks[l->kind]);
        h = h->previous;
    }
    CHECK(h == &l->head);
    int out = 91, expected = 0;
    for (unsigned n = 0; n < m->n; ++n)
    {
        expected = expected * 5 + items[m->values[n]].value;
    }
    CHECK(ds_dual_fold(l, digits, 0, &out) == DS_OK && out == expected);
    return 0;
}

static int index_check(const struct ds_index_owner *o, unsigned k,
                       const struct ds_model *m)
{
    CHECK(o->count[k] == m->n);
    unsigned i = o->head[k], previous = 4;
    int expected = 0;
    for (unsigned n = 0; n < m->n; ++n)
    {
        CHECK(i == m->values[n]);
        CHECK(o->slots[i].previous[k] == previous);
        expected = expected * 5 + o->slots[i].value;
        previous = i;
        i = o->slots[i].next[k];
    }
    CHECK(i == 4 && previous == o->tail[k]);
    for (unsigned n = m->n; n > 0; --n)
    {
        CHECK(previous == m->values[n - 1]);
        previous = o->slots[previous].previous[k];
    }
    CHECK(previous == 4);
    int out = 91;
    CHECK(ds_index_fold(o, k, digits, 0, &out) == DS_OK && out == expected);
    return 0;
}

static int trace(unsigned code, unsigned length, int output)
{
    struct ds_dual_item items[4] = {0};
    struct ds_dual_list lists[2] = {0};
    struct ds_model m[2] = {0};
    struct ds_index_slot a[4] = {0}, b[4] = {0};
    struct ds_index_owner owner = {0};
    struct ds_index_handle handles[4] = {{0}};
    ds_index_init(&owner, a);
    for (unsigned k = 0; k < 2; ++k)
    {
        ds_dual_init(&lists[k], k);
        CHECK(lists[k].head.next == &lists[k].head);
    }
    for (unsigned i = 0; i < 4; ++i)
    {
        ds_dual_item_init(&items[i], (int)i + 1);
        CHECK(ds_index_new(&owner, (int)i + 1, &handles[i]) == DS_OK);
    }
    unsigned script = code;
    for (unsigned n = 0; n < length; ++n)
    {
        unsigned action = script % 8;
        script /= 8;
        unsigned item = action % 2, kind = (action / 2) % 2,
                 remove = action / 4;
        int want = model(&m[kind], item, remove);
        int rc = remove ? ds_dual_remove(&lists[kind], &items[item])
                        : ds_dual_push(&lists[kind], &items[item]);
        CHECK(rc == want);
        rc = remove ? ds_index_remove(&owner, kind, handles[item])
                    : ds_index_push(&owner, kind, handles[item]);
        CHECK(rc == want);
        CHECK(ds_index_relocate(&owner, owner.slots == a ? b : a) == DS_OK);
        CHECK(owner.slots == ((n % 2) ? a : b));
        for (unsigned k = 0; k < 2; ++k)
        {
            CHECK(dual_check(&lists[k], items, &m[k]) == 0);
            CHECK(index_check(&owner, k, &m[k]) == 0);
        }
        if (output)
        {
            int v0 = 0, v1 = 0;
            CHECK(ds_index_fold(&owner, 0, digits, 0, &v0) == DS_OK);
            CHECK(ds_index_fold(&owner, 1, digits, 0, &v1) == DS_OK);
            printf("I %u %u %d %d %d\n", code, n, rc, v0, v1);
        }
    }
    return 0;
}

static int baseline_histories(void)
{
    /* All prefixes are observed; length-seven words cover every shorter word. */
    for (unsigned code = 0; code < 2097152; ++code)
    {
        unsigned script = code;
        struct ds_list l;
        struct ds_item items[4];
        struct ds_model m = {0};
        ds_list_init(&l);
        for (unsigned i = 0; i < 4; ++i)
        {
            ds_item_init(&items[i], (int)i + 1);
        }
        for (unsigned n = 0; n < 7; ++n)
        {
            unsigned action = script % 8;
            script /= 8;
            unsigned i = action % 4, remove = action / 4;
            int at = position(&m, i);
            int want = remove ? (at < 0 ? DS_INVALID : DS_OK)
                              : (at >= 0 ? DS_INVALID : DS_OK);
            int rc = remove ? ds_list_remove(&l, &items[i])
                            : ds_list_push(&l, &items[i]);
            CHECK(rc == want);
            if (rc == DS_OK)
            {
                (void)model(&m, i, remove);
            }
            CHECK(l.count == m.n);
            const struct ds_link *h = &l.head;
            int sum = 0;
            for (unsigned j = 0; j < m.n; ++j)
            {
                CHECK(h->next == &items[m.values[j]].link);
                CHECK(h->next->previous == h);
                h = h->next;
                sum += items[m.values[j]].value;
            }
            CHECK(h->next == &l.head && l.head.previous == h);
            for (unsigned j = m.n; j > 0; --j)
            {
                CHECK(h == &items[m.values[j - 1]].link);
                h = h->previous;
            }
            CHECK(h == &l.head);
            int out = 91;
            CHECK(ds_list_sum(&l, &out) == DS_OK && out == sum);
        }
    }
    printf(
        "original-list: 2097152 length-seven scripts; 14680064 validated transitions\n");
    return 0;
}

static int dual(void)
{
    CHECK(trace(0, 1, 0) == 0);
    for (unsigned code = 0; code < 262144; ++code)
    {
        CHECK(trace(code, 6, 0) == 0);
    }
    struct ds_dual_list a = {0}, b = {0}, wrong = {0};
    struct ds_dual_item items[5] = {0};
    ds_dual_init(&a, 0);
    ds_dual_init(&b, 1);
    ds_dual_init(&wrong, 0);
    for (unsigned i = 0; i < 5; ++i)
    {
        ds_dual_item_init(&items[i], i == 0 ? INT_MAX : 1);
    }
    for (unsigned i = 0; i < 4; ++i)
    {
        CHECK(ds_dual_push(&a, &items[i]) == DS_OK);
        CHECK(ds_dual_push(&b, &items[3 - i]) == DS_OK);
    }
    CHECK(ds_dual_push(&a, &items[4]) == DS_FULL);
    CHECK(ds_dual_remove(&wrong, &items[0]) == DS_STALE);
    int out = 91;
    CHECK(ds_dual_fold(&a, add, 0, &out) == DS_RANGE && out == 91);
    CHECK(ds_dual_fold(&a, fail, 0, &out) == DS_FAILED && out == 91);
    CHECK(ds_dual_remove(&a, &items[0]) == DS_OK &&
          items[0].hooks[1].owner == &b);
    struct ds_dual_item *parent = items[1].hooks[0].parent;
    items[1].hooks[0].parent = &items[2];
    CHECK(ds_dual_fold(&a, add, 0, &out) == DS_INVALID && out == 91);
    items[1].hooks[0].parent = parent;
    CHECK(baseline_histories() == 0);
    return 0;
}

static int index_test(void)
{
    CHECK(trace(0, 1, 0) == 0);
    for (unsigned code = 0; code < 4096; ++code)
    {
        CHECK(trace(code, 4, 0) == 0);
    }
    struct ds_index_owner o = {0}, foreign = {0};
    struct ds_index_slot a[4] = {0}, b[4] = {0}, c[4] = {0};
    ds_index_init(&o, a);
    ds_index_init(&foreign, c);
    struct ds_index_handle all[12] = {{0}}, out = {0};
    int value = 91;
    for (unsigned i = 0; i < 12; ++i)
    {
        CHECK(ds_index_new(&o, (int)i, &all[i]) == DS_OK);
        for (unsigned j = 0; j < i; ++j)
        {
            CHECK(ds_index_get(&o, all[j], &value) == DS_STALE && value == 91);
        }
        CHECK(ds_index_get(&foreign, all[i], &value) == DS_STALE &&
              value == 91);
        CHECK(ds_index_push(&o, 0, all[i]) == DS_OK);
        CHECK(ds_index_push(&o, 1, all[i]) == DS_OK);
        CHECK(ds_index_release(&o, all[i]) == DS_BUSY);
        CHECK(ds_index_remove(&o, 0, all[i]) == DS_OK);
        CHECK(ds_index_release(&o, all[i]) == DS_BUSY);
        CHECK(ds_index_remove(&o, 1, all[i]) == DS_OK);
        CHECK(ds_index_release(&o, all[i]) == DS_OK);
        CHECK(ds_index_relocate(&o, (i % 2) ? a : b) == DS_OK);
        for (unsigned j = 0; j <= i; ++j)
        {
            CHECK(ds_index_get(&o, all[j], &value) == DS_STALE && value == 91);
        }
    }
    out = (struct ds_index_handle){&foreign, 3, 2};
    CHECK(ds_index_new(&o, 7, &out) == DS_FULL && out.owner == &foreign &&
          out.index == 3 && out.generation == 2);
    for (unsigned i = 0; i < 4; ++i)
    {
        CHECK(o.generation[i] == 4 && !o.live[i]);
    }
    CHECK(ds_index_relocate(&o, NULL) == DS_INVALID);
    CHECK(ds_index_get(NULL, out, &value) == DS_INVALID);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        return 2;
    }
    test = argv[1];
    if (!strcmp(test, "dual_hook"))
    {
        return dual();
    }
    if (!strcmp(test, "index_move"))
    {
        return index_test();
    }
    if (!strcmp(test, "oracle"))
    {
        for (unsigned code = 0; code < 4096; ++code)
        {
            if (trace(code, 4, 1))
            {
                return 1;
            }
        }
        return 0;
    }
    return 2;
}
