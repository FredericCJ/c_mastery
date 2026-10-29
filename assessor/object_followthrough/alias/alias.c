#include "alias.h"

static int admits(struct os_allocator a, size_t size, size_t alignment)
{
    return a.allocator.allocate && a.allocator.deallocate &&
           a.capacity >= size && a.alignment >= alignment &&
           a.alignment % alignment == 0;
}

int os_acquire(struct fd_slot *s, struct os_allocator a)
{
    if (!s || !admits(a, sizeof(struct fd_value), _Alignof(struct fd_value)))
    {
        return -1;
    }
    return fd_slot_acquire(s, a.allocator) == FD_OK ? 0 : -1;
}

int os_wide_acquire(struct os_wide_slot *s, struct os_allocator a, int value,
                    os_drop_fn drop, void *context)
{
    struct os_wide *p;
    if (!s || s->state != OS_EMPTY || !drop ||
        !admits(a, sizeof(struct os_wide), _Alignof(struct os_wide)))
    {
        return -1;
    }
    p = a.allocator.allocate(a.allocator.context, sizeof(*p),
                             _Alignof(struct os_wide));
    if (!p)
    {
        return -1;
    }
    p->value = value;
    p->resource = NULL;
    s->value = p;
    s->allocation = a;
    s->drop = drop;
    s->context = context;
    s->state = OS_LIVE;
    return 0;
}

int os_wide_borrow(struct os_wide_slot *s, const struct os_wide **out)
{
    if (!s || !out || s->state != OS_LIVE)
    {
        return -1;
    }
    *out = s->value;
    s->state = OS_BORROWED;
    return 0;
}

int os_wide_return(struct os_wide_slot *s, const struct os_wide **p)
{
    if (!s || !p || s->state != OS_BORROWED || *p != s->value)
    {
        return -1;
    }
    *p = NULL;
    s->state = OS_LIVE;
    return 0;
}

int os_wide_reset(struct os_wide_slot *s)
{
    if (!s)
    {
        return -1;
    }
    if (s->state == OS_EMPTY)
    {
        return 0;
    }
    if (s->state == OS_BORROWED)
    {
        return -1;
    }
    s->state = OS_CLEANUP;
#ifdef OS_MUTANT_DESTRUCTOR
    (void)s->drop(s->context, s->value);
#else
    if (s->drop(s->context, s->value))
    {
        return -1;
    }
#endif
    s->allocation.allocator.deallocate(s->allocation.allocator.context,
                                       s->value);
    s->value = NULL;
    s->state = OS_EMPTY;
    s->drop = NULL;
    s->context = NULL;
    return 0;
}

int os_clone(void *allocated, const struct fd_value *source)
{
    unsigned char *d = allocated;
    const unsigned char *s = (const unsigned char *)source;
    if (!d || !s)
    {
        return -1;
    }
    for (size_t i = 0; i < sizeof(*source); ++i)
    {
        d[i] = s[i];
    }
    return 0;
}

uint32_t os_wire_integer(const unsigned char *bytes)
{
    uint32_t n = 0;
    for (size_t i = 0; i < 4; ++i)
    {
        n = (n << 8) | bytes[i];
    }
    return n;
}

int os_wire(const unsigned char *bytes, size_t count, struct fd_value *out)
{
    struct fd_value v;
    uint32_t number;
    if (!bytes || !out || count != 8)
    {
        return -1;
    }
    number = os_wire_integer(bytes);
    if (number > INT32_MAX)
    {
        return -1;
    }
    v.value = (int32_t)number;
    v.serial = os_wire_integer(bytes + 4);
#ifdef OS_MUTANT_WIRE
    v.value = bytes[0];
#endif
    *out = v;
    return 0;
}

uint32_t os_host_representation(uint32_t value)
{
    uint32_t out;
    unsigned char *d = (unsigned char *)&out;
    const unsigned char *s = (const unsigned char *)&value;
    for (size_t i = 0; i < sizeof value; ++i)
    {
        d[i] = s[i];
    }
    return out;
}

void os_transform(unsigned char *restrict d, const unsigned char *restrict s,
                  size_t n, unsigned char mask)
{
    for (size_t i = 0; i < n; ++i)
    {
        d[i] = (unsigned char)(s[i] ^ mask);
    }
}

void os_ordinary(unsigned char *d, const unsigned char *s, size_t n,
                 unsigned char mask)
{
    for (size_t i = 0; i < n; ++i)
    {
        d[i] = (unsigned char)(s[i] ^ mask);
    }
}

void os_nested(unsigned char *restrict d, const unsigned char *restrict s,
               size_t n, unsigned char mask)
{
    {
        unsigned char *restrict inner = d;
        const unsigned char *restrict input = s;
        for (size_t i = 0; i < n; ++i)
        {
            inner[i] = (unsigned char)(input[i] ^ mask);
        }
    }
}

void os_shared(unsigned char *restrict d, const unsigned char *restrict a,
               const unsigned char *restrict b, size_t n)
{
    for (size_t i = 0; i < n; ++i)
    {
        d[i] = (unsigned char)(a[i] ^ b[i]);
    }
}

int os_partition(unsigned char *base, size_t length, size_t dst, size_t src,
                 size_t n, unsigned char mask, int inplace)
{
    if ((!base && length) || dst > length || src > length || n > length - dst ||
        n > length - src)
    {
        return -1;
    }
    if (!n)
    {
        return 0;
    }
    if (dst == src)
    {
        if (!inplace)
        {
            return -1;
        }
        for (size_t i = 0; i < n; ++i)
        {
            base[dst + i] ^= mask;
        }
        return 0;
    }
    if (dst < src ? n > src - dst : n > dst - src)
    {
#ifdef OS_MUTANT_PARTITION
        return 0; /* Wrong success without touching a conflicting restricted range. */
#else
        return -1;
#endif
    }
    os_transform(base + dst, base + src, n, mask);
    return 0;
}

enum fd_status os_move(unsigned char *base, size_t length, size_t dst,
                       size_t src, size_t n)
{
    return fd_move(base, length, dst, src, n);
}

int os_member(struct os_container *owner, int *member, int *out)
{
    const unsigned char *whole;
    int *expected;
    if (!owner || !member || !out)
    {
        return -1;
    }
    whole = (const unsigned char *)owner;
    expected = (int *)(void *)(whole + offsetof(struct os_container, member));
    if (member != expected)
    {
        return -1;
    }
    *out = owner->member;
    return 0;
}
#ifdef UINTPTR_MAX
int os_tag_roundtrip(struct os_container *owner, unsigned tag,
                     uintptr_t *representation, int *out)
{
    uintptr_t raw, tagged, restored;
    void *round;
    if (!owner || !representation || !out || tag > 3 ||
        _Alignof(struct os_container) < 4)
    {
        return -1;
    }
    raw = (uintptr_t)(void *)owner;
    if (raw & 3u)
    {
        return -1;
    }
    tagged = raw | tag;
    restored = tagged & ~(uintptr_t)3u;
    if (restored != raw)
    {
        return -1;
    }
    round = (void *)restored;
    if (round != owner)
    {
        return -1;
    }
    *representation = tagged;
    *out = owner->member;
    return 0;
}
#endif
