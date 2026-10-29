#include "transfer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                        \
    do                                                                  \
    {                                                                   \
        if (!(x))                                                       \
        {                                                               \
            fprintf(stderr, "FD_TRANSFER_CHECK:%d:%s\n", __LINE__, #x); \
            return 1;                                                   \
        }                                                               \
    } while (0)
static unsigned long observations;

static int semantic(void)
{
    const int values[] = {INT_MIN, -1, 0, 1, INT_MAX - 1, INT_MAX};
    for (unsigned i = 0; i < sizeof values / sizeof values[0]; ++i)
    {
        struct fd_step s = fd_step_value(values[i]);
        CHECK(s.old_value == values[i]);
        CHECK(s.status == (values[i] == INT_MAX ? FD_RANGE : FD_OK));
        CHECK(s.new_value ==
              (values[i] == INT_MAX ? values[i] : values[i] + 1));
        ++observations;
    }
    /* Promotion models are mathematical premises, not claims about this host. */
    CHECK(65535L + 1L == 65536L);
    CHECK(((65535UL + 1UL) % 65536UL) == 0);
    return 0;
}

static int layout(void)
{
    size_t sentinel = 917;
    CHECK(fd_extent(1, 0, 1, 0, &sentinel) == FD_RANGE && sentinel == 917);
    CHECK(fd_extent(0, SIZE_MAX, 2, SIZE_MAX, &sentinel) == FD_RANGE &&
          sentinel == 917);
    CHECK(fd_extent(0, 1, 1, 0, &sentinel) == FD_RANGE && sentinel == 917);
    struct fd_layout out = {91, 92, 93};
    CHECK(fd_layout(12, 7, 9, 10, 11, 180, &out) == FD_RANGE);
    CHECK(out.first == 91 && out.second == 92 && out.end == 93);
    for (size_t h = 0; h < 5; ++h)
    {
        for (int a = -1; a < 6; ++a)
        {
            for (int b = -1; b < 6; ++b)
            {
                for (size_t aw = 0; aw < 5; ++aw)
                {
                    for (size_t bw = 0; bw < 5; ++bw)
                    {
                        for (size_t limit = 0; limit < 20; ++limit)
                        {
                            long exact = (long)h + (long)a * (long)aw +
                                         (long)b * (long)bw;
                            int valid =
                                a >= 0 && b >= 0 && exact <= (long)limit;
                            out = (struct fd_layout){91, 92, 93};
                            CHECK(fd_layout(h, a, aw, b, bw, limit, &out) ==
                                  (valid ? FD_OK : FD_RANGE));
                            if (valid)
                            {
                                CHECK(out.first == h &&
                                      out.second == h + (size_t)a * aw &&
                                      out.end == (size_t)exact);
                            }
                            else
                            {
                                CHECK(out.first == 91 && out.second == 92 &&
                                      out.end == 93);
                            }
                            ++observations;
                        }
                    }
                }
            }
        }
    }
    CHECK(fd_layout(SIZE_MAX, 1, 1, 0, 0, SIZE_MAX, &out) == FD_RANGE);
    CHECK(fd_layout(0, INTMAX_MIN, 0, 0, 0, SIZE_MAX, &out) == FD_RANGE);
    return 0;
}

static int cursor(void)
{
    const unsigned char a[] = {11, 22}, b[] = {33, 44, 55},
                        flat[] = {11, 22, 33, 44, 55};
    for (size_t pos = 0; pos <= 5; ++pos)
    {
        for (size_t n = 0; n <= 6; ++n)
        {
            for (size_t cap = 0; cap <= 6; ++cap)
            {
                struct fd_segmented c = {{{a, 2}, {b, 3}}, pos};
                unsigned char out[7];
                memset(out, 0xa7, sizeof out);
                int valid = n <= 5 - pos && n <= cap;
                CHECK(fd_copy_segments(&c, n, out, cap) ==
                      (valid ? FD_OK : FD_RANGE));
                CHECK(c.position == (valid ? pos + n : pos));
                for (size_t i = 0; i < 7; ++i)
                {
                    CHECK(out[i] == (valid && i < n ? flat[pos + i] : 0xa7));
                }
                ++observations;
            }
        }
    }
    struct fd_segmented empty = {{{0, 0}, {0, 0}}, 0};
    CHECK(fd_copy_segments(&empty, 0, 0, 0) == FD_OK);
    const unsigned char data[] = {1, 2, 3, 4, 5, 6};
    for (size_t begin = 0; begin <= 6; ++begin)
    {
        for (size_t end = begin; end <= 6; ++end)
        {
            for (size_t pos = 0; pos <= end - begin; ++pos)
            {
                for (size_t off = 0; off <= 7; ++off)
                {
                    for (size_t n = 0; n <= 7; ++n)
                    {
                        struct fd_owner_view source = {data, 6, begin, end,
                                                       pos},
                                             out = source;
                        int valid = off <= end - begin - pos &&
                                    n <= end - begin - pos - off;
                        CHECK(fd_owner_subview(&out, off, n, &out) ==
                              (valid ? FD_OK : FD_RANGE));
                        CHECK(out.owner == data && out.owner_length == 6);
                        CHECK(out.begin == (valid ? begin + pos + off : begin));
                        CHECK(out.end ==
                                  (valid ? begin + pos + off + n : end) &&
                              out.position == (valid ? 0 : pos));
                        ++observations;
                    }
                }
            }
        }
    }
    unsigned char tagged[] = {0xa5, 2, 0x34, 0x12};
    for (size_t length = 0; length <= 4; ++length)
    {
        struct fd_cursor source = {tagged, length, 0};
        uint16_t out = 71;
        CHECK(fd_read_tagged(&source, &out) ==
              (length == 4 ? FD_OK : FD_RANGE));
        CHECK(source.position == (length == 4 ? 4u : 0u) &&
              out == (length == 4 ? 0x1234 : 71));
    }
    tagged[1] = 3;
    struct fd_cursor malformed = {tagged, 4, 0};
    uint16_t saved = 71;
    CHECK(fd_read_tagged(&malformed, &saved) == FD_INVALID &&
          !malformed.position && saved == 71);
    /* Integer intervals oracle, independently derived before C pointer formation. */
    for (size_t p = 0; p <= 6; ++p)
    {
        for (size_t off = 0; off <= 7; ++off)
        {
            for (size_t n = 0; n <= 7; ++n)
            {
                struct fd_cursor parent = {data, 6, p}, child = {data, 1, 1},
                                 sentinel = child;
                int valid = off <= 6 - p && n <= 6 - p - off;
                CHECK(fd_subview(&parent, off, n, &child) ==
                      (valid ? FD_OK : FD_RANGE));
                CHECK(parent.base == data && parent.length == 6 &&
                      parent.position == p);
                if (!valid)
                {
                    CHECK(child.base == sentinel.base && child.length == 1 &&
                          child.position == 1);
                    continue;
                }
                CHECK(child.base == data + p + off && child.length == n &&
                      child.position == 0);
                struct fd_cursor alias = parent;
                CHECK(fd_subview(&alias, off, n, &alias) == FD_OK);
                CHECK(alias.base == child.base && alias.length == n &&
                      alias.position == 0);
                for (size_t q = 0; q <= n; ++q)
                {
                    for (size_t next = 0; next <= n + 1; ++next)
                    {
                        struct fd_cursor nested = child;
                        nested.position = q;
                        const unsigned char *out = data;
                        CHECK(fd_take(&nested, next, &out) ==
                              (next <= n - q ? FD_OK : FD_RANGE));
                        CHECK(nested.position ==
                              (next <= n - q ? q + next : q));
                        CHECK(out ==
                              (next <= n - q ? data + p + off + q : data));
                        ++observations;
                    }
                }
            }
        }
    }
    return 0;
}

static int codec(void)
{
    const unsigned char gold[] = {2, 0, 2, 1, 254, 255, 255, 255};
    unsigned char out[8];
    uintmax_t count = 0;
    intmax_t value = 0;
    CHECK(fd_encode_v2(258, -2, out, 8) == FD_OK && !memcmp(out, gold, 8));
    CHECK(fd_decode_v2(gold, 8, &count, &value) == FD_OK && count == 258 &&
          value == -2);
    struct fd_least_record least = {258, -2}, decoded = {0, 0};
    CHECK(fd_least_encode(least, out, 8) == FD_OK && !memcmp(out, gold, 8));
    CHECK(fd_least_decode(out, 8, &decoded) == FD_OK && decoded.count == 258 &&
          decoded.value == -2);
    out[1] = 1;
    CHECK(fd_least_decode(out, 8, &decoded) == FD_INVALID &&
          decoded.count == 258 && decoded.value == -2);
    const intmax_t values[] = {INT32_MIN, -1, 0, 1, INT32_MAX};
    const uintmax_t counts[] = {0, 1, 255, 256, 65535, 65536};
    for (unsigned i = 0; i < 6; ++i)
    {
        for (unsigned j = 0; j < 5; ++j)
        {
            memset(out, 0xa7, 8);
            enum fd_status s = fd_encode_v2(counts[i], values[j], out, 8);
            if (counts[i] > 65535)
            {
                CHECK(s == FD_RANGE);
                for (unsigned k = 0; k < 8; ++k)
                {
                    CHECK(out[k] == 0xa7);
                }
            }
            else
            {
                CHECK(s == FD_OK);
                CHECK(fd_decode_v2(out, 8, &count, &value) == FD_OK);
                CHECK(count == counts[i] && value == values[j]);
            }
            ++observations;
        }
    }
    memcpy(out, gold, 8);
    out[1] = 1;
    count = 99;
    value = 42;
    CHECK(fd_decode_v2(out, 8, &count, &value) == FD_INVALID && count == 99 &&
          value == 42);
    for (size_t n = 0; n <= 9; ++n)
    {
        if (n != 8)
        {
            CHECK(fd_decode_v2(gold, n, &count, &value) == FD_RANGE &&
                  count == 99 && value == 42);
            ++observations;
        }
    }
    return 0;
}

struct fd_tokens
{
    char events[32];
    size_t used;
    unsigned fail, a, b;
};

static void event(struct fd_tokens *t, char ch)
{
    if (t->used < sizeof t->events)
    {
        t->events[t->used++] = ch;
    }
}

static int initialize(void *context, struct fd_value *v)
{
    struct fd_tokens *t = context;
    event(t, 'A');
    t->a = 1;
    if (t->fail == 1)
    {
        event(t, 'a');
        t->a = 0;
        return 1;
    }
    event(t, 'B');
    t->b = 1;
    if (t->fail == 2)
    {
        event(t, 'b');
        t->b = 0;
        event(t, 'a');
        t->a = 0;
        return 1;
    }
    *v = (struct fd_value){37, 123};
    return 0;
}

static void destroy(void *context, struct fd_value *v)
{
    struct fd_tokens *t = context;
    (void)v;
    event(t, 'b');
    t->b = 0;
    event(t, 'a');
    t->a = 0;
}

static int lifecycle(void)
{
    for (unsigned fail = 0; fail < 3; ++fail)
    {
        struct fd_typed_storage store = {{0, 0}, 0};
        struct fd_tokens t = {{0}, 0, fail, 0, 0};
        struct fd_slot s = FD_SLOT_INITIALIZER,
                       destination = FD_SLOT_INITIALIZER;
        CHECK(fd_slot_acquire(
                  &s, (struct fd_allocator){&store, fd_typed_allocate,
                                            fd_typed_release}) == FD_OK);
        CHECK(fd_slot_initialize(&s, initialize, destroy, &t) ==
              (fail ? FD_INIT_FAILED : FD_OK));
        if (fail)
        {
            CHECK(!store.reserved && !t.a && !t.b &&
                  t.used == (fail == 1 ? 2u : 4u));
            CHECK(!memcmp(t.events, fail == 1 ? "Aa" : "ABba", t.used));
        }
        else
        {
            CHECK(fd_slot_transfer(&destination, &s) == FD_OK &&
                  s.state == FD_EMPTY);
            fd_slot_reset(&s);
            CHECK(store.reserved && t.a && t.b);
            const struct fd_value *published = 0;
            CHECK(fd_slot_publish(&destination, &published) == FD_OK &&
                  published->value == 37);
            fd_slot_reset(&destination);
            CHECK(!store.reserved && !t.a && !t.b && t.used == 4 &&
                  !memcmp(t.events, "ABba", 4));
        }
        fd_slot_reset(&s);
        fd_slot_reset(&destination);
        ++observations;
    }
    /* All operation strings of length five: acquire, init-success, init-fail,
       publish, reset. Each successful initialization owns two synthetic tokens. */
    for (unsigned sequence = 0; sequence < 3125; ++sequence)
    {
        unsigned digits = sequence;
        struct fd_slot s = FD_SLOT_INITIALIZER;
        struct fd_typed_storage store = {{0, 0}, 0};
        struct fd_tokens t = {{0}, 0, 0, 0, 0};
        enum fd_slot_state model = FD_EMPTY;
        for (unsigned step = 0; step < 5; ++step)
        {
            unsigned op = digits % 5;
            digits /= 5;
            enum fd_status want = FD_OK, got = FD_OK;
            if (op == 0)
            {
                want = model == FD_EMPTY ? FD_OK : FD_STATE;
                got = fd_slot_acquire(
                    &s, (struct fd_allocator){&store, fd_typed_allocate,
                                              fd_typed_release});
                if (want == FD_OK)
                {
                    model = FD_ACQUIRED;
                }
            }
            else if (op < 3)
            {
                t.fail = op == 2;
                want = model != FD_ACQUIRED ? FD_STATE
                       : t.fail             ? FD_INIT_FAILED
                                            : FD_OK;
                got = fd_slot_initialize(&s, initialize, destroy, &t);
                if (model == FD_ACQUIRED)
                {
                    model = t.fail ? FD_EMPTY : FD_INITIALIZED;
                }
            }
            else if (op == 3)
            {
                const struct fd_value sentinel = {0, 0}, *p = &sentinel;
                want = model == FD_INITIALIZED || model == FD_PUBLISHED
                           ? FD_OK
                           : FD_STATE;
                got = fd_slot_publish(&s, &p);
                if (want == FD_OK)
                {
                    model = FD_PUBLISHED;
                    CHECK(p->value == 37);
                }
                else
                {
                    CHECK(p == &sentinel);
                }
            }
            else
            {
                fd_slot_reset(&s);
                model = FD_EMPTY;
            }
            CHECK(got == want && s.state == model);
            CHECK(store.reserved == (model != FD_EMPTY));
            CHECK(t.a == (model == FD_INITIALIZED || model == FD_PUBLISHED) &&
                  t.b == t.a);
            ++observations;
        }
        fd_slot_reset(&s);
        CHECK(!store.reserved && !t.a && !t.b);
    }
    return 0;
}

static int movement(void)
{
    unsigned char inplace[256];
    for (unsigned i = 0; i < 256; ++i)
    {
        inplace[i] = (unsigned char)i;
    }
    for (unsigned mask = 0; mask < 256; ++mask)
    {
        CHECK(fd_inplace_xor(inplace, 256, (unsigned char)mask) == FD_OK);
        for (unsigned i = 0; i < 256; ++i)
        {
            CHECK(inplace[i] == (unsigned char)(i ^ mask));
        }
        CHECK(fd_inplace_xor(inplace, 256, (unsigned char)mask) == FD_OK);
        ++observations;
    }
    CHECK(fd_inplace_xor(0, 0, 3) == FD_OK &&
          fd_inplace_xor(0, 1, 3) == FD_INVALID);
    for (size_t stride = 1; stride <= 3; ++stride)
    {
        for (size_t dst = 0; dst <= 4; ++dst)
        {
            for (size_t src = 0; src <= 4; ++src)
            {
                for (size_t n = 0; n <= 5; ++n)
                {
                    unsigned char data[12], snapshot[12], want[12];
                    for (unsigned i = 0; i < 12; ++i)
                    {
                        data[i] = snapshot[i] = want[i] =
                            (unsigned char)(i + 10);
                    }
                    int valid = n <= 4 - dst && n <= 4 - src;
                    if (valid)
                    {
                        for (size_t i = 0; i < n; ++i)
                        {
                            want[(dst + i) * stride] =
                                snapshot[(src + i) * stride];
                        }
                    }
                    CHECK(fd_stride_move(data, 12, 4, stride, dst, src, n) ==
                          (valid ? FD_OK : FD_RANGE));
                    CHECK(!memcmp(data, want, 12));
                    ++observations;
                }
            }
        }
    }
    unsigned char data[] = {10, 20, 30, 40, 50, 60, 70, 80};
    struct fd_stream_view stream = {data, 8, 3, 1};
    CHECK(fd_compact(&stream) == FD_OK && stream.length == 5 &&
          stream.position == 0 && stream.epoch == 2);
    const unsigned char want[] = {40, 50, 60, 70, 80};
    CHECK(!memcmp(data, want, 5));
    stream.epoch = SIZE_MAX;
    CHECK(fd_compact(&stream) == FD_FULL);
    return 0;
}

static int registry(void)
{
    struct fd_pool direct;
    fd_pool_init(&direct);
    struct fd_handle local;
    CHECK(fd_pool_acquire(&direct, (struct fd_value){17, 23}, &local) == FD_OK);
    const struct fd_value *borrow = 0;
    CHECK(fd_pool_borrow(&direct, local, &borrow) == FD_OK &&
          borrow->value == 17);
    CHECK(fd_pool_release(&direct, local) == FD_OK);
    /* Discard expired borrow without using it; stale lookup preserves a fresh sentinel. */
    struct fd_value sentinel = {91, 92};
    borrow = &sentinel;
    CHECK(fd_pool_borrow(&direct, local, &borrow) == FD_STALE &&
          borrow == &sentinel);
    struct fd_registry moving;
    fd_registry_init(&moving);
    struct fd_pool source, destination;
    fd_pool_init(&source);
    CHECK(fd_registry_bind(&moving, 1, &source) == FD_OK);
    struct fd_registry_handle stable;
    CHECK(fd_registry_acquire(&moving, 1, (struct fd_value){31, 37}, &stable) ==
          FD_OK);
    CHECK(fd_registry_relocate(&moving, 1, &destination) == FD_OK);
    struct fd_value moved = {0, 0};
    CHECK(fd_registry_get(&moving, stable, &moved) == FD_OK &&
          moved.value == 31);
    CHECK(!source.entries[stable.local_slot].live &&
          source.entries[stable.local_slot].retired);
    CHECK(fd_registry_relocate(&moving, 1, &destination) == FD_STATE);
    struct fd_registry registry;
    fd_registry_init(&registry);
    struct fd_pool owners[3];
    struct fd_registry_handle old = {0, 0, 0, 0};
    for (unsigned epoch = 1; epoch <= 3; ++epoch)
    {
        fd_pool_init(&owners[epoch - 1]);
        CHECK(fd_registry_bind(&registry, 0, &owners[epoch - 1]) == FD_OK);
        struct fd_registry_handle h;
        CHECK(fd_registry_acquire(&registry, 0,
                                  (struct fd_value){(int32_t)epoch, 7},
                                  &h) == FD_OK);
        struct fd_value value = {99, 99};
        if (epoch > 1)
        {
            CHECK(fd_registry_get(&registry, old, &value) == FD_STALE &&
                  value.value == 99);
        }
        CHECK(fd_registry_get(&registry, h, &value) == FD_OK &&
              value.value == (int32_t)epoch);
        if (epoch == 1)
        {
            old = h;
        }
        CHECK(fd_registry_unbind(&registry, 0) == FD_OK);
        value.value = 99;
        CHECK(fd_registry_get(&registry, h, &value) == FD_STALE &&
              value.value == 99);
        ++observations;
    }
    CHECK(fd_registry_bind(&registry, 0, &owners[0]) == FD_FULL);
    CHECK(registry.entries[0].retired);
    return 0;
}

static int frame(void)
{
    for (unsigned flag = 0; flag < 4; ++flag)
    {
        for (int auth = 0; auth < 2; ++auth)
        {
            unsigned char request[] = {1, (unsigned char)flag, 0, 2, 0, 0, 0,
                                       3};
            int32_t state = 10;
            CHECK(fd_apply(request, 8, auth, &state) ==
                  (auth ? FD_OK : FD_DENIED));
            CHECK(state == (auth ? 16 : 10));
            ++observations;
            int32_t observed = 71;
            state = INT32_MAX;
            CHECK(fd_effect(FD_QUERY, request, 8, auth, &state, &observed) ==
                  FD_OK);
            CHECK(state == INT32_MAX && observed == INT32_MAX);
            observed = 71;
            CHECK(fd_effect(FD_UPDATE, request, 8, auth, &state, &observed) ==
                  FD_RANGE);
            CHECK(state == INT32_MAX && observed == 71);
            state = 10;
            CHECK(fd_effect(FD_UPDATE, request, 8, auth, &state, &observed) ==
                  (auth ? FD_OK : FD_DENIED));
            CHECK(state == (auth ? 16 : 10) && observed == (auth ? 16 : 71));
            observed = 71;
            CHECK(fd_effect(FD_QUERY, request, 7, auth, &state, &observed) ==
                      FD_RANGE &&
                  observed == 71);
        }
    }
    const unsigned char wire[] = {1, 1, 0, 2, 0, 0, 0, 3};
    for (size_t cut = 0; cut < 8; ++cut)
    {
        for (int first = 0; first < 2; ++first)
        {
            for (int next = 0; next < 2; ++next)
            {
                struct fd_frame cancelled = {{0}, 0, 0};
                int32_t state = 10;
                int done = 77;
                CHECK(fd_frame_feed(&cancelled, wire, cut, first, &state,
                                    &done) == FD_OK);
                fd_frame_cancel(&cancelled);
                CHECK(fd_frame_feed(&cancelled, wire, 8, next, &state, &done) ==
                      (next ? FD_OK : FD_DENIED));
                CHECK(state == (next ? 16 : 10) && done == next);
                ++observations;
            }
        }
    }
    /* All 128 nonempty chunkings and both initial authority values. */
    for (unsigned cuts = 0; cuts < 128; ++cuts)
    {
        for (int authority = 0; authority < 2; ++authority)
        {
            struct fd_frame f = {{0}, 0, 0};
            int32_t counter = 10;
            int committed = 77;
            size_t start = 0;
            for (size_t end = 1; end <= 8; ++end)
            {
                if (end == 8 || (cuts & (1u << (end - 1))))
                {
                    int supplied = start == 0 ? authority : !authority;
                    enum fd_status s =
                        fd_frame_feed(&f, wire + start, end - start, supplied,
                                      &counter, &committed);
                    CHECK(s == (end == 8 && !authority ? FD_DENIED : FD_OK));
                    CHECK(committed == (end == 8 && authority));
                    CHECK(counter == (end == 8 && authority ? 16 : 10));
                    start = end;
                    ++observations;
                }
            }
            CHECK(f.used == 0);
        }
    }
    struct fd_frame f = {{0}, 0, 0};
    int32_t counter = 10;
    int committed = 77;
    CHECK(fd_frame_feed(&f, wire, 3, 1, &counter, &committed) == FD_OK);
    fd_frame_cancel(&f);
    CHECK(fd_frame_feed(&f, wire, 8, 0, &counter, &committed) == FD_DENIED);
    CHECK(counter == 10 && !committed);
    CHECK(fd_frame_feed(&f, wire, 3, 0, &counter, &committed) == FD_OK);
    fd_frame_cancel(&f);
    CHECK(fd_frame_feed(&f, wire + 3, 5, 0, &counter, &committed) == FD_OK);
    struct fd_frame before = f;
    committed = 77;
    CHECK(fd_frame_feed(&f, wire, 4, 1, &counter, &committed) == FD_RANGE);
    CHECK(f.used == before.used && f.authority == before.authority &&
          !memcmp(f.bytes, before.bytes, 8));
    CHECK(counter == 10 && committed == 77);
    fd_frame_cancel(&f);
    CHECK(fd_frame_feed(&f, wire, 8, 1, &counter, &committed) == FD_OK);
    CHECK(counter == 16 && committed == 1);
    return 0;
}

int main(int argc, char **argv)
{
    const struct
    {
        const char *name;
        int (*run)(void);
    } cases[] = {{"semantic", semantic},   {"layout", layout},
                 {"cursor", cursor},       {"codec", codec},
                 {"lifecycle", lifecycle}, {"movement", movement},
                 {"registry", registry},   {"frame", frame}};

    if (argc != 2)
    {
        return 2;
    }
    for (unsigned i = 0; i < 8; ++i)
    {
        if (!strcmp(argv[1], cases[i].name))
        {
            int result = cases[i].run();
            if (!result)
            {
                printf("FD_TRANSFER_OK:%s:%lu\n", cases[i].name, observations);
            }
            return result;
        }
    }
    return 2;
}
