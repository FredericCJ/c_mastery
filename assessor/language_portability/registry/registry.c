#include "registry.h"
#include "generated/ids.h"
#ifndef LR_MUTANT
#define LR_MUTANT 0
#endif
int lr_handle_start(int x)
{
    return x + 1;
}

int lr_handle_stop(int x)
{
    return x + 4;
}

int lr_handle_status(int x)
{
    return x + 7;
}

int lr_handle_reset(int x)
{
    return x + 9;
}

struct lp_name
{
    uint32_t id;
    const char *name;
};

struct lp_dispatch
{
    uint32_t id;
    int (*handler)(int);
};

struct lp_alias
{
    const char *name;
    uint32_t id;
};
static const struct lp_name names[] = {
#include "generated/names.inc"
};
static const struct lp_dispatch dispatch[] = {
#include "generated/dispatch.inc"
};
static const struct lp_alias aliases[] = {
#include "generated/aliases.inc"
};
_Static_assert(sizeof names / sizeof *names == LR_COMMAND_COUNT, "name count");
_Static_assert(sizeof dispatch / sizeof *dispatch == LR_COMMAND_COUNT,
               "handler count");
_Static_assert(sizeof aliases / sizeof *aliases ==
                   (LR_ALIAS_COUNT ? LR_ALIAS_COUNT : 1u),
               "alias count");

static size_t find(uint32_t id, size_t *probes)
{
    size_t lo = 0, hi = LR_COMMAND_COUNT;
    *probes = 0;
    while (lo < hi)
    {
        size_t mid = lo + (hi - lo) / 2;
        ++*probes;
        if (names[mid].id < id)
        {
            lo = mid + 1;
        }
        else
        {
            hi = mid;
        }
    }
    return lo < LR_COMMAND_COUNT && names[lo].id == id ? lo : LR_COMMAND_COUNT;
}

static int equal(const char *a, const char *b)
{
    while (*a && *a == *b)
    {
        ++a;
        ++b;
    }
    return *a == *b;
}

size_t lr_count(void)
{
    return LR_COMMAND_COUNT;
}

size_t lr_probes(uint32_t id)
{
    size_t n;
    (void)find(id, &n);
    return n;
}

int lr_dispatch(uint32_t id, int x, int *out)
{
    size_t n, i;
    if (!out || x < 0 || x > 100)
    {
        return -1;
    }
    i = find(id, &n);
    if (i == LR_COMMAND_COUNT)
    {
        if (LR_MUTANT == 1)
        {
            *out = 0;
        }
        return -1;
    }
    *out = dispatch[i].handler(x);
    return 0;
}

int lr_name(uint32_t id, const char **out)
{
    size_t n, i;
    if (!out)
    {
        return -1;
    }
    i = find(id, &n);
    if (i == LR_COMMAND_COUNT)
    {
        return -1;
    }
    *out = names[LR_MUTANT == 2 ? 0 : i].name;
    return 0;
}

int lr_id(const char *name, uint32_t *out)
{
    if (!name || !out)
    {
        return -1;
    }
    for (size_t i = 0; i < LR_COMMAND_COUNT; ++i)
    {
        if (equal(name, names[i].name))
        {
            *out = names[i].id;
            return 0;
        }
    }
    for (size_t i = 0; i < sizeof aliases / sizeof *aliases; ++i)
    {
        if (aliases[i].name && equal(name, aliases[i].name))
        {
            *out = aliases[i].id;
            return 0;
        }
    }
    return -1;
}

int lr_at(size_t i, uint32_t *id, const char **name)
{
    if (!id || !name || i >= LR_COMMAND_COUNT)
    {
        return -1;
    }
    *id = names[i].id;
    *name = names[i].name;
    return 0;
}
