#include "runtime.h"

static uint16_t u16(const unsigned char *p)
{
    return (uint16_t)(p[0] | (uint16_t)p[1] << 8);
}

static uint32_t u32(const unsigned char *p)
{
    uint32_t v = 0;
    unsigned i;
    for (i = 0; i < 4; i++)
        v |= (uint32_t)p[i] << (8 * i);
    return v;
}

static uint64_t u64(const unsigned char *p)
{
    uint64_t v = 0;
    unsigned i;
    for (i = 0; i < 8; i++)
        v |= (uint64_t)p[i] << (8 * i);
    return v;
}

static uintptr_t location(const unsigned char *p, uintptr_t sp, uint64_t stack)
{
    uint32_t off = u32(p + 8);
    CHECK(p[0] == 3 && p[1] == 0 && u16(p + 2) == 8 && u16(p + 4) == 7 &&
          u16(p + 6) == 0);
    CHECK(stack <= 512 && off <= stack && stack - off >= 8);
    return sp + off;
}

void relocate_roots(uintptr_t sp, uintptr_t ret)
{
    const unsigned char *m = __sp_maps, *r, *loc;
    uint64_t address, stack;
    unsigned count, i, n = 0;
    uintptr_t dst[32], value[32];
    CHECK((uintptr_t)__sp_maps_end - (uintptr_t)m >= 56);
    CHECK(m[0] == 3 && m[1] == 0 && u16(m + 2) == 0 && u32(m + 4) == 1 &&
          u32(m + 8) == 0 && u32(m + 12) == 1);
    address = u64(m + 16);
    stack = u64(m + 24);
    CHECK(u64(m + 32) == 1);
    r = m + 40;
    CHECK(ret == address + u32(r + 8) && u16(r + 12) == 0);
    count = u16(r + 14);
    CHECK(count >= 5 && count <= 35 && (count - 3) % 2 == 0);
    CHECK((uintptr_t)(r + 16) + (uintptr_t)count * 12 <=
          (uintptr_t)__sp_maps_end);
    loc = r + 16;
    for (i = 0; i < 3; i++)
        CHECK(loc[12 * i] == 4 && u32(loc + 12 * i + 8) == 0);
    for (i = 3; i < count; i += 2)
    {
        uintptr_t base, derived, a = location(loc + 12 * i, sp, stack),
                                 b = location(loc + 12 * (i + 1), sp, stack);
        unsigned j;
        memcpy(&base, (void *)a, 8);
        memcpy(&derived, (void *)b, 8);
        CHECK(base == (uintptr_t)old_space && derived >= base &&
              derived - base < 8);
        for (j = 0; j < n; j++)
            if (dst[j] == b)
                break;
        if (j == n)
        {
            dst[n] = b;
            value[n] = (uintptr_t)new_space + (derived - base);
            n++;
        }
    }
    memcpy(new_space, old_space, 8);
    memset(old_space, 0, 8);
    new_space[8] = 99;
    if (mutant != 1)
        for (i = 0; i < n; i++)
        {
            uintptr_t updated = value[i] + (mutant == 2 ? 1u : 0u);
            memcpy((void *)dst[i], &updated, 8);
        }
    polls++;
    slots = n;
}
