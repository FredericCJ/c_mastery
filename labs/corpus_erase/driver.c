#include "erase.h"
#include <stdio.h>

struct er_context
{
    unsigned mask, fail, calls, deleted;
};

static int predicate(void *p, const struct er_item *i)
{
    struct er_context *c = p;
    if (++c->calls == c->fail)
        return -1;
    return (int)((c->mask >> (i->id - 1)) & 1);
}

static void drop(void *p, unsigned id)
{
    ((struct er_context *)p)->deleted |= 1u << (id - 1);
}

int main(void)
{
    unsigned n, mask, fail;
    while (scanf("%u %u %u", &n, &mask, &fail) == 3)
    {
        if (n > 8)
            return 2;
        struct er_item items[8];
        for (unsigned i = 0; i < n; ++i)
            items[i] = (struct er_item){i + 1, (int)i};
        struct er_owner o;
        if (er_init(&o, items, n))
            return 3;
        struct er_context c = {mask, fail, 0, 0};
        struct er_tail tail = {0};
        int status = er_remove_if(&o, predicate, &c, &tail);
        printf("%d %u %zu %zu", status, c.calls, o.count,
               o.pending ? o.keep : o.count);
        for (size_t i = 0; i < o.count; ++i)
            printf(" %u", o.items[i].id);
        if (!status && er_erase(&o, tail, drop, &c))
            return 4;
        printf(" | %zu %u", o.count, c.deleted);
        if (er_clear(&o, drop, &c))
            return 5;
        printf(" %u\n", c.deleted);
    }
    return 0;
}
