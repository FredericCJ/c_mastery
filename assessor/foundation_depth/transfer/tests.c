/* Withheld changed-condition tests for the foundation_depth transfer
 * assignments. `tests <case>` prints "FD_TRANSFER_OK:<case>:<observations>" and
 * exits 0 on success, prints FD_TRANSFER_CHECK to stderr and exits 1 on the
 * first failed check, and exits 2 for a usage error. */
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

/* Runs a helper that itself returns nonzero after a failed CHECK. */
#define RUN_STEP(call)   \
    do                   \
    {                    \
        if ((call) != 0) \
        {                \
            return 1;    \
        }                \
    } while (0)

/* Preloaded into outputs so that a write on a failure path is visible. */
#define UNTOUCHED_SIZE ((size_t)917)
#define UNTOUCHED_BYTE 0xa7u
#define OCTET_VALUES 256u

/* Number of exhaustive or sampled cases executed, for the success line. */
static unsigned long observations;

static int semantic(void)
{
    const int values[] = {INT_MIN, -1, 0, 1, INT_MAX - 1, INT_MAX};
    for (unsigned i = 0; i < sizeof values / sizeof values[0]; ++i)
    {
        const struct fd_step step = fd_step_value(values[i]);
        const int at_maximum = values[i] == INT_MAX;
        CHECK(step.old_value == values[i]);
        CHECK(step.status == (at_maximum ? FD_RANGE : FD_OK));
        CHECK(step.new_value == (at_maximum ? values[i] : values[i] + 1));
        ++observations;
    }
    /* Promotion models are mathematical premises, not claims about this
     * host. */
    CHECK(65535L + 1L == 65536L);
    CHECK(((65535UL + 1UL) % 65536UL) == 0);
    return 0;
}

/* Output members of fd_layout results that must survive a rejection. */
#define LAYOUT_FIRST_SENTINEL 91
#define LAYOUT_SECOND_SENTINEL 92
#define LAYOUT_END_SENTINEL 93

static struct fd_layout layout_sentinel(void)
{
    return (struct fd_layout){LAYOUT_FIRST_SENTINEL, LAYOUT_SECOND_SENTINEL,
                              LAYOUT_END_SENTINEL};
}

static int layout_is_sentinel(struct fd_layout layout)
{
    return layout.first == LAYOUT_FIRST_SENTINEL &&
           layout.second == LAYOUT_SECOND_SENTINEL &&
           layout.end == LAYOUT_END_SENTINEL;
}

/* Header, then first array, then second array, all inside one limit. Signed
 * counts are converted before use; the second extent starts where the first
 * ends, not at the header. Exact arithmetic is done in long. */
static int layout_grid(void)
{
    for (size_t header = 0; header < 5; ++header)
    {
        for (int first_count = -1; first_count < 6; ++first_count)
        {
            for (int second_count = -1; second_count < 6; ++second_count)
            {
                for (size_t first_width = 0; first_width < 5; ++first_width)
                {
                    for (size_t second_width = 0; second_width < 5;
                         ++second_width)
                    {
                        for (size_t limit = 0; limit < 20; ++limit)
                        {
                            const long first_bytes =
                                (long)first_count * (long)first_width;
                            const long exact =
                                (long)header + first_bytes +
                                (long)second_count * (long)second_width;
                            const int valid = first_count >= 0 &&
                                              second_count >= 0 &&
                                              exact <= (long)limit;
                            struct fd_layout out = layout_sentinel();
                            CHECK(fd_layout(header, first_count, first_width,
                                            second_count, second_width, limit,
                                            &out) ==
                                  (valid ? FD_OK : FD_RANGE));
                            if (valid)
                            {
                                CHECK(out.first == header &&
                                      out.second ==
                                          header + (size_t)first_bytes &&
                                      out.end == (size_t)exact);
                            }
                            else
                            {
                                CHECK(layout_is_sentinel(out));
                            }
                            ++observations;
                        }
                    }
                }
            }
        }
    }
    return 0;
}

static int layout(void)
{
    size_t sentinel = UNTOUCHED_SIZE;
    CHECK(fd_extent(1, 0, 1, 0, &sentinel) == FD_RANGE &&
          sentinel == UNTOUCHED_SIZE);
    CHECK(fd_extent(0, SIZE_MAX, 2, SIZE_MAX, &sentinel) == FD_RANGE &&
          sentinel == UNTOUCHED_SIZE);
    CHECK(fd_extent(0, 1, 1, 0, &sentinel) == FD_RANGE &&
          sentinel == UNTOUCHED_SIZE);

    struct fd_layout out = layout_sentinel();
    CHECK(fd_layout(12, 7, 9, 10, 11, 180, &out) == FD_RANGE);
    CHECK(layout_is_sentinel(out));

    RUN_STEP(layout_grid());

    CHECK(fd_layout(SIZE_MAX, 1, 1, 0, 0, SIZE_MAX, &out) == FD_RANGE);
    CHECK(fd_layout(0, INTMAX_MIN, 0, 0, 0, SIZE_MAX, &out) == FD_RANGE);
    return 0;
}

/* Two segments read as one logical array: every position, count and
 * capacity. */
static int cursor_segments(void)
{
    const unsigned char first_segment[] = {11, 22};
    const unsigned char second_segment[] = {33, 44, 55};
    const unsigned char flat[] = {11, 22, 33, 44, 55};
    for (size_t position = 0; position <= sizeof flat; ++position)
    {
        for (size_t count = 0; count <= sizeof flat + 1; ++count)
        {
            for (size_t capacity = 0; capacity <= sizeof flat + 1; ++capacity)
            {
                struct fd_segmented cursor = {
                    {{first_segment, sizeof first_segment},
                     {second_segment, sizeof second_segment}},
                    position};
                unsigned char out[sizeof flat + 2];
                memset(out, UNTOUCHED_BYTE, sizeof out);
                const int valid =
                    count <= sizeof flat - position && count <= capacity;
                CHECK(fd_copy_segments(&cursor, count, out, capacity) ==
                      (valid ? FD_OK : FD_RANGE));
                CHECK(cursor.position == (valid ? position + count : position));
                for (size_t i = 0; i < sizeof out; ++i)
                {
                    CHECK(out[i] == (valid && i < count ? flat[position + i]
                                                        : UNTOUCHED_BYTE));
                }
                ++observations;
            }
        }
    }
    struct fd_segmented empty = {{{0, 0}, {0, 0}}, 0};
    CHECK(fd_copy_segments(&empty, 0, 0, 0) == FD_OK);
    return 0;
}

/* Subviews that carry the owner's identity and bounds with them. out aliases
 * the source, so a rejection must leave the source intact. */
static int cursor_owner_views(void)
{
    const unsigned char data[] = {1, 2, 3, 4, 5, 6};
    const size_t length = sizeof data;
    for (size_t begin = 0; begin <= length; ++begin)
    {
        for (size_t end = begin; end <= length; ++end)
        {
            for (size_t position = 0; position <= end - begin; ++position)
            {
                for (size_t offset = 0; offset <= length + 1; ++offset)
                {
                    for (size_t count = 0; count <= length + 1; ++count)
                    {
                        const struct fd_owner_view source = {
                            data, length, begin, end, position};
                        struct fd_owner_view out = source;
                        const size_t remaining = end - begin - position;
                        const int valid =
                            offset <= remaining && count <= remaining - offset;
                        CHECK(fd_owner_subview(&out, offset, count, &out) ==
                              (valid ? FD_OK : FD_RANGE));
                        CHECK(out.owner == data && out.owner_length == length);
                        CHECK(out.begin ==
                              (valid ? begin + position + offset : begin));
                        CHECK(out.end ==
                                  (valid ? begin + position + offset + count
                                         : end) &&
                              out.position == (valid ? 0 : position));
                        ++observations;
                    }
                }
            }
        }
    }
    return 0;
}

#define TAGGED_BYTES 4u
#define TAGGED_UNTOUCHED 71

/* A two-octet tag followed by a two-octet little-endian value, read as a unit:
 * a short or malformed input must not consume the tag. */
static int cursor_tagged_reads(void)
{
    unsigned char tagged[TAGGED_BYTES] = {0xa5, 2, 0x34, 0x12};
    for (size_t length = 0; length <= TAGGED_BYTES; ++length)
    {
        struct fd_cursor source = {tagged, length, 0};
        uint16_t out = TAGGED_UNTOUCHED;
        const int complete = length == TAGGED_BYTES;
        CHECK(fd_read_tagged(&source, &out) == (complete ? FD_OK : FD_RANGE));
        CHECK(source.position == (complete ? TAGGED_BYTES : 0u) &&
              out == (complete ? 0x1234 : TAGGED_UNTOUCHED));
    }
    tagged[1] = 3;
    struct fd_cursor malformed = {tagged, TAGGED_BYTES, 0};
    uint16_t saved = TAGGED_UNTOUCHED;
    CHECK(fd_read_tagged(&malformed, &saved) == FD_INVALID &&
          !malformed.position && saved == TAGGED_UNTOUCHED);
    return 0;
}

/* Nested view then take, against interval arithmetic derived independently of
 * any pointer formation. */
static int cursor_subview_and_take(void)
{
    const unsigned char data[] = {1, 2, 3, 4, 5, 6};
    const size_t length = sizeof data;
    for (size_t position = 0; position <= length; ++position)
    {
        for (size_t offset = 0; offset <= length + 1; ++offset)
        {
            for (size_t count = 0; count <= length + 1; ++count)
            {
                const struct fd_cursor parent = {data, length, position};
                struct fd_cursor child = {data, 1, 1};
                const struct fd_cursor sentinel = child;
                const int valid = offset <= length - position &&
                                  count <= length - position - offset;
                CHECK(fd_subview(&parent, offset, count, &child) ==
                      (valid ? FD_OK : FD_RANGE));
                CHECK(parent.base == data && parent.length == length &&
                      parent.position == position);
                if (!valid)
                {
                    CHECK(child.base == sentinel.base && child.length == 1 &&
                          child.position == 1);
                    continue;
                }
                CHECK(child.base == data + position + offset &&
                      child.length == count && child.position == 0);
                struct fd_cursor alias = parent;
                CHECK(fd_subview(&alias, offset, count, &alias) == FD_OK);
                CHECK(alias.base == child.base && alias.length == count &&
                      alias.position == 0);
                for (size_t start = 0; start <= count; ++start)
                {
                    for (size_t next = 0; next <= count + 1; ++next)
                    {
                        struct fd_cursor nested = child;
                        nested.position = start;
                        const unsigned char *out = data;
                        const int fits = next <= count - start;
                        CHECK(fd_take(&nested, next, &out) ==
                              (fits ? FD_OK : FD_RANGE));
                        CHECK(nested.position == (fits ? start + next : start));
                        CHECK(out ==
                              (fits ? data + position + offset + start : data));
                        ++observations;
                    }
                }
            }
        }
    }
    return 0;
}

static int cursor(void)
{
    RUN_STEP(cursor_segments());
    RUN_STEP(cursor_owner_views());
    RUN_STEP(cursor_tagged_reads());
    RUN_STEP(cursor_subview_and_take());
    return 0;
}

/* Version-2 wire format: little-endian, version octet 2, 16-bit count. */
static int codec(void)
{
    const unsigned char gold[FD_WIRE_BYTES] = {2, 0, 2, 1, 254, 255, 255, 255};
    unsigned char out[FD_WIRE_BYTES];
    uintmax_t count = 0;
    intmax_t value = 0;
    CHECK(fd_encode_v2(258, -2, out, FD_WIRE_BYTES) == FD_OK &&
          !memcmp(out, gold, FD_WIRE_BYTES));
    CHECK(fd_decode_v2(gold, FD_WIRE_BYTES, &count, &value) == FD_OK &&
          count == 258 && value == -2);
    const struct fd_least_record least = {258, -2};
    struct fd_least_record decoded = {0, 0};
    CHECK(fd_least_encode(least, out, FD_WIRE_BYTES) == FD_OK &&
          !memcmp(out, gold, FD_WIRE_BYTES));
    CHECK(fd_least_decode(out, FD_WIRE_BYTES, &decoded) == FD_OK &&
          decoded.count == 258 && decoded.value == -2);
    out[1] = 1;
    CHECK(fd_least_decode(out, FD_WIRE_BYTES, &decoded) == FD_INVALID &&
          decoded.count == 258 && decoded.value == -2);

    const intmax_t values[] = {INT32_MIN, -1, 0, 1, INT32_MAX};
    const uintmax_t counts[] = {0, 1, 255, 256, 65535, 65536};
    for (unsigned i = 0; i < sizeof counts / sizeof counts[0]; ++i)
    {
        for (unsigned j = 0; j < sizeof values / sizeof values[0]; ++j)
        {
            memset(out, UNTOUCHED_BYTE, sizeof out);
            const enum fd_status status =
                fd_encode_v2(counts[i], values[j], out, FD_WIRE_BYTES);
            if (counts[i] > UINT16_MAX)
            {
                CHECK(status == FD_RANGE);
                for (unsigned k = 0; k < sizeof out; ++k)
                {
                    CHECK(out[k] == UNTOUCHED_BYTE);
                }
            }
            else
            {
                CHECK(status == FD_OK);
                CHECK(fd_decode_v2(out, FD_WIRE_BYTES, &count, &value) ==
                      FD_OK);
                CHECK(count == counts[i] && value == values[j]);
            }
            ++observations;
        }
    }

    memcpy(out, gold, sizeof out);
    out[1] = 1;
    count = 99;
    value = 42;
    CHECK(fd_decode_v2(out, FD_WIRE_BYTES, &count, &value) == FD_INVALID &&
          count == 99 && value == 42);
    for (size_t length = 0; length <= FD_WIRE_BYTES + 1; ++length)
    {
        if (length != FD_WIRE_BYTES)
        {
            CHECK(fd_decode_v2(gold, length, &count, &value) == FD_RANGE &&
                  count == 99 && value == 42);
            ++observations;
        }
    }
    return 0;
}

/* Records the order in which the two synthetic resources A and B are acquired
 * (uppercase) and released (lowercase). */
struct fd_tokens
{
    char events[32];
    size_t used;
    unsigned fail; /* 0: succeed, 1: fail after A, 2: fail after B */
    unsigned a;
    unsigned b;
};

static void event(struct fd_tokens *tokens, char letter)
{
    if (tokens->used < sizeof tokens->events)
    {
        tokens->events[tokens->used++] = letter;
    }
}

static int initialize(void *context, struct fd_value *value)
{
    struct fd_tokens *tokens = context;
    event(tokens, 'A');
    tokens->a = 1;
    if (tokens->fail == 1)
    {
        event(tokens, 'a');
        tokens->a = 0;
        return 1;
    }
    event(tokens, 'B');
    tokens->b = 1;
    if (tokens->fail == 2)
    {
        event(tokens, 'b');
        tokens->b = 0;
        event(tokens, 'a');
        tokens->a = 0;
        return 1;
    }
    *value = (struct fd_value){37, 123};
    return 0;
}

static void destroy(void *context, struct fd_value *value)
{
    struct fd_tokens *tokens = context;
    (void)value;
    event(tokens, 'b');
    tokens->b = 0;
    event(tokens, 'a');
    tokens->a = 0;
}

static struct fd_allocator typed_allocator(struct fd_typed_storage *store)
{
    return (struct fd_allocator){store, fd_typed_allocate, fd_typed_release};
}

/* Initializer failing after A, after B, and succeeding; a successful slot is
 * moved to a second owner, which must then be the only one that can free. */
static int lifecycle_failure_prefixes(void)
{
    for (unsigned fail = 0; fail < 3; ++fail)
    {
        struct fd_typed_storage store = {{0, 0}, 0};
        struct fd_tokens tokens = {{0}, 0, fail, 0, 0};
        struct fd_slot slot = FD_SLOT_INITIALIZER;
        struct fd_slot destination = FD_SLOT_INITIALIZER;
        CHECK(fd_slot_acquire(&slot, typed_allocator(&store)) == FD_OK);
        CHECK(fd_slot_initialize(&slot, initialize, destroy, &tokens) ==
              (fail ? FD_INIT_FAILED : FD_OK));
        if (fail)
        {
            CHECK(!store.reserved && !tokens.a && !tokens.b &&
                  tokens.used == (fail == 1 ? 2u : 4u));
            CHECK(
                !memcmp(tokens.events, fail == 1 ? "Aa" : "ABba", tokens.used));
        }
        else
        {
            CHECK(fd_slot_transfer(&destination, &slot) == FD_OK &&
                  slot.state == FD_EMPTY);
            fd_slot_reset(&slot);
            CHECK(store.reserved && tokens.a && tokens.b);
            const struct fd_value *published = 0;
            CHECK(fd_slot_publish(&destination, &published) == FD_OK &&
                  published->value == 37);
            fd_slot_reset(&destination);
            CHECK(!store.reserved && !tokens.a && !tokens.b &&
                  tokens.used == 4 && !memcmp(tokens.events, "ABba", 4));
        }
        fd_slot_reset(&slot);
        fd_slot_reset(&destination);
        ++observations;
    }
    return 0;
}

/* Operations of the exhaustive state-machine walk. */
enum life_operation
{
    LIFE_ACQUIRE,
    LIFE_INIT_SUCCEEDS,
    LIFE_INIT_FAILS,
    LIFE_PUBLISH,
    LIFE_RESET,
    LIFE_OPERATION_COUNT
};

#define LIFE_STEPS 5u
/* LIFE_OPERATION_COUNT ** LIFE_STEPS */
#define LIFE_SEQUENCES 3125u

/* A slot together with the model of what it should be doing. */
struct life_run
{
    struct fd_slot slot;
    struct fd_typed_storage store;
    struct fd_tokens tokens;
    enum fd_slot_state model;
};

static unsigned model_holds_value(enum fd_slot_state state)
{
    return state == FD_INITIALIZED || state == FD_PUBLISHED;
}

/* Applies one operation to the slot and to the model, then compares them. */
static int life_step(struct life_run *run, enum life_operation operation)
{
    enum fd_status want = FD_OK;
    enum fd_status got = FD_OK;
    if (operation == LIFE_ACQUIRE)
    {
        want = run->model == FD_EMPTY ? FD_OK : FD_STATE;
        got = fd_slot_acquire(&run->slot, typed_allocator(&run->store));
        if (want == FD_OK)
        {
            run->model = FD_ACQUIRED;
        }
    }
    else if (operation == LIFE_INIT_SUCCEEDS || operation == LIFE_INIT_FAILS)
    {
        run->tokens.fail = operation == LIFE_INIT_FAILS;
        if (run->model != FD_ACQUIRED)
        {
            want = FD_STATE;
        }
        else
        {
            want = run->tokens.fail ? FD_INIT_FAILED : FD_OK;
        }
        got = fd_slot_initialize(&run->slot, initialize, destroy, &run->tokens);
        if (run->model == FD_ACQUIRED)
        {
            run->model = run->tokens.fail ? FD_EMPTY : FD_INITIALIZED;
        }
    }
    else if (operation == LIFE_PUBLISH)
    {
        const struct fd_value sentinel = {0, 0};
        const struct fd_value *borrowed = &sentinel;
        want = model_holds_value(run->model) ? FD_OK : FD_STATE;
        got = fd_slot_publish(&run->slot, &borrowed);
        if (want == FD_OK)
        {
            run->model = FD_PUBLISHED;
            CHECK(borrowed->value == 37);
        }
        else
        {
            CHECK(borrowed == &sentinel);
        }
    }
    else
    {
        fd_slot_reset(&run->slot);
        run->model = FD_EMPTY;
    }
    CHECK(got == want && run->slot.state == run->model);
    CHECK(run->store.reserved == (run->model != FD_EMPTY));
    CHECK(run->tokens.a == model_holds_value(run->model) &&
          run->tokens.b == run->tokens.a);
    ++observations;
    return 0;
}

/* Every operation string of length LIFE_STEPS (digits in base
 * LIFE_OPERATION_COUNT). Each successful initialization owns two synthetic
 * tokens, which must all be released by the end of the walk. */
static int lifecycle_operation_strings(void)
{
    for (unsigned sequence = 0; sequence < LIFE_SEQUENCES; ++sequence)
    {
        struct life_run run = {
            FD_SLOT_INITIALIZER, {{0, 0}, 0}, {{0}, 0, 0, 0, 0}, FD_EMPTY};
        unsigned digits = sequence;
        for (unsigned step = 0; step < LIFE_STEPS; ++step)
        {
            const enum life_operation operation =
                (enum life_operation)(digits % LIFE_OPERATION_COUNT);
            digits /= LIFE_OPERATION_COUNT;
            RUN_STEP(life_step(&run, operation));
        }
        fd_slot_reset(&run.slot);
        CHECK(!run.store.reserved && !run.tokens.a && !run.tokens.b);
    }
    return 0;
}

static int lifecycle(void)
{
    RUN_STEP(lifecycle_failure_prefixes());
    RUN_STEP(lifecycle_operation_strings());
    return 0;
}

static int movement_inplace_xor(void)
{
    unsigned char inplace[OCTET_VALUES];
    for (unsigned i = 0; i < OCTET_VALUES; ++i)
    {
        inplace[i] = (unsigned char)i;
    }
    for (unsigned mask = 0; mask < OCTET_VALUES; ++mask)
    {
        CHECK(fd_inplace_xor(inplace, OCTET_VALUES, (unsigned char)mask) ==
              FD_OK);
        for (unsigned i = 0; i < OCTET_VALUES; ++i)
        {
            CHECK(inplace[i] == (unsigned char)(i ^ mask));
        }
        /* XOR twice restores the original, ready for the next mask. */
        CHECK(fd_inplace_xor(inplace, OCTET_VALUES, (unsigned char)mask) ==
              FD_OK);
        ++observations;
    }
    CHECK(fd_inplace_xor(0, 0, 3) == FD_OK &&
          fd_inplace_xor(0, 1, 3) == FD_INVALID);
    return 0;
}

#define STRIDE_PHYSICAL_BYTES 12u
#define STRIDE_LOGICAL_ELEMENTS 4u

/* Elements spaced `stride` bytes apart inside a physical array; only the
 * logical elements may move, snapshot-copy semantics. */
static int movement_stride(void)
{
    for (size_t stride = 1; stride <= 3; ++stride)
    {
        for (size_t destination = 0; destination <= STRIDE_LOGICAL_ELEMENTS;
             ++destination)
        {
            for (size_t source = 0; source <= STRIDE_LOGICAL_ELEMENTS; ++source)
            {
                for (size_t count = 0; count <= STRIDE_LOGICAL_ELEMENTS + 1;
                     ++count)
                {
                    unsigned char data[STRIDE_PHYSICAL_BYTES];
                    unsigned char snapshot[STRIDE_PHYSICAL_BYTES];
                    unsigned char want[STRIDE_PHYSICAL_BYTES];
                    for (unsigned i = 0; i < STRIDE_PHYSICAL_BYTES; ++i)
                    {
                        data[i] = (unsigned char)(i + 10);
                        snapshot[i] = data[i];
                        want[i] = data[i];
                    }
                    const int valid =
                        count <= STRIDE_LOGICAL_ELEMENTS - destination &&
                        count <= STRIDE_LOGICAL_ELEMENTS - source;
                    if (valid)
                    {
                        for (size_t i = 0; i < count; ++i)
                        {
                            want[(destination + i) * stride] =
                                snapshot[(source + i) * stride];
                        }
                    }
                    CHECK(fd_stride_move(data, STRIDE_PHYSICAL_BYTES,
                                         STRIDE_LOGICAL_ELEMENTS, stride,
                                         destination, source,
                                         count) == (valid ? FD_OK : FD_RANGE));
                    CHECK(!memcmp(data, want, STRIDE_PHYSICAL_BYTES));
                    ++observations;
                }
            }
        }
    }
    return 0;
}

static int movement_compact(void)
{
    unsigned char data[] = {10, 20, 30, 40, 50, 60, 70, 80};
    struct fd_stream_view stream = {data, sizeof data, 3, 1};
    CHECK(fd_compact(&stream) == FD_OK && stream.length == 5 &&
          stream.position == 0 && stream.epoch == 2);
    const unsigned char want[] = {40, 50, 60, 70, 80};
    CHECK(!memcmp(data, want, sizeof want));
    /* An epoch counter at its maximum cannot advance. */
    stream.epoch = SIZE_MAX;
    CHECK(fd_compact(&stream) == FD_FULL);
    return 0;
}

static int movement(void)
{
    RUN_STEP(movement_inplace_xor());
    RUN_STEP(movement_stride());
    RUN_STEP(movement_compact());
    return 0;
}

/* Borrowing from a pool that is released afterwards, and relocating a pool
 * to a new owner while its registry handle stays valid. */
static int registry_borrow_and_relocate(void)
{
    struct fd_pool direct;
    fd_pool_init(&direct);
    struct fd_handle local;
    CHECK(fd_pool_acquire(&direct, (struct fd_value){17, 23}, &local) == FD_OK);
    const struct fd_value *borrow = 0;
    CHECK(fd_pool_borrow(&direct, local, &borrow) == FD_OK &&
          borrow->value == 17);
    CHECK(fd_pool_release(&direct, local) == FD_OK);
    /* The expired borrow is discarded unused; a stale lookup must leave a
     * fresh pointer alone. */
    struct fd_value sentinel = {91, 92};
    borrow = &sentinel;
    CHECK(fd_pool_borrow(&direct, local, &borrow) == FD_STALE &&
          borrow == &sentinel);

    struct fd_registry moving;
    fd_registry_init(&moving);
    struct fd_pool source;
    struct fd_pool destination;
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
    return 0;
}

#define REGISTRY_EPOCHS 3u

/* Bind, use and unbind a registry slot once per epoch; a handle from an
 * earlier epoch must stay stale, and the slot retires after the last. */
static int registry_epochs(void)
{
    struct fd_registry registry;
    fd_registry_init(&registry);
    struct fd_pool owners[REGISTRY_EPOCHS];
    struct fd_registry_handle first_epoch_handle = {0, 0, 0, 0};
    for (unsigned epoch = 1; epoch <= REGISTRY_EPOCHS; ++epoch)
    {
        struct fd_pool *owner = &owners[epoch - 1];
        fd_pool_init(owner);
        CHECK(fd_registry_bind(&registry, 0, owner) == FD_OK);
        struct fd_registry_handle handle;
        CHECK(fd_registry_acquire(&registry, 0,
                                  (struct fd_value){(int32_t)epoch, 7},
                                  &handle) == FD_OK);
        struct fd_value value = {99, 99};
        if (epoch > 1)
        {
            CHECK(fd_registry_get(&registry, first_epoch_handle, &value) ==
                      FD_STALE &&
                  value.value == 99);
        }
        CHECK(fd_registry_get(&registry, handle, &value) == FD_OK &&
              value.value == (int32_t)epoch);
        if (epoch == 1)
        {
            first_epoch_handle = handle;
        }
        CHECK(fd_registry_unbind(&registry, 0) == FD_OK);
        value.value = 99;
        CHECK(fd_registry_get(&registry, handle, &value) == FD_STALE &&
              value.value == 99);
        ++observations;
    }
    CHECK(fd_registry_bind(&registry, 0, &owners[0]) == FD_FULL);
    CHECK(registry.entries[0].retired);
    return 0;
}

static int registry(void)
{
    RUN_STEP(registry_borrow_and_relocate());
    RUN_STEP(registry_epochs());
    return 0;
}

/* Initial synthetic counter, and the counter after the wire record
 * {count 2, value 3} is applied. */
#define FRAME_COUNTER_START 10
#define FRAME_COUNTER_APPLIED 16
#define FRAME_UNTOUCHED 71
#define FRAME_FLAG_PATTERNS 4u
/* Bit i of a cut mask splits the wire after octet i + 1: seven interior
 * boundaries, so 2^7 chunkings. */
#define FRAME_CUT_MASKS (1u << (FD_WIRE_BYTES - 1))

/* One-shot effects (query and update) under every flag pattern and both
 * authority values. */
static int frame_effects(void)
{
    for (unsigned flags = 0; flags < FRAME_FLAG_PATTERNS; ++flags)
    {
        for (int authority = 0; authority < 2; ++authority)
        {
            const unsigned char request[FD_WIRE_BYTES] = {
                1, (unsigned char)flags, 0, 2, 0, 0, 0, 3};
            int32_t state = FRAME_COUNTER_START;
            CHECK(fd_apply(request, FD_WIRE_BYTES, authority, &state) ==
                  (authority ? FD_OK : FD_DENIED));
            CHECK(state ==
                  (authority ? FRAME_COUNTER_APPLIED : FRAME_COUNTER_START));
            ++observations;

            int32_t observed = FRAME_UNTOUCHED;
            state = INT32_MAX;
            CHECK(fd_effect(FD_QUERY, request, FD_WIRE_BYTES, authority, &state,
                            &observed) == FD_OK);
            CHECK(state == INT32_MAX && observed == INT32_MAX);
            observed = FRAME_UNTOUCHED;
            CHECK(fd_effect(FD_UPDATE, request, FD_WIRE_BYTES, authority,
                            &state, &observed) == FD_RANGE);
            CHECK(state == INT32_MAX && observed == FRAME_UNTOUCHED);
            state = FRAME_COUNTER_START;
            CHECK(fd_effect(FD_UPDATE, request, FD_WIRE_BYTES, authority,
                            &state,
                            &observed) == (authority ? FD_OK : FD_DENIED));
            CHECK(state == (authority ? FRAME_COUNTER_APPLIED
                                      : FRAME_COUNTER_START) &&
                  observed ==
                      (authority ? FRAME_COUNTER_APPLIED : FRAME_UNTOUCHED));
            observed = FRAME_UNTOUCHED;
            CHECK(fd_effect(FD_QUERY, request, FD_WIRE_BYTES - 1, authority,
                            &state, &observed) == FD_RANGE &&
                  observed == FRAME_UNTOUCHED);
        }
    }
    return 0;
}

/* Feed a prefix, cancel, then feed a whole record: only the second feed's
 * authority may count. */
static int frame_cancel_after_prefix(const unsigned char *wire)
{
    for (size_t cut = 0; cut < FD_WIRE_BYTES; ++cut)
    {
        for (int first = 0; first < 2; ++first)
        {
            for (int next = 0; next < 2; ++next)
            {
                struct fd_frame cancelled = {{0}, 0, 0};
                int32_t state = FRAME_COUNTER_START;
                int done = 77;
                CHECK(fd_frame_feed(&cancelled, wire, cut, first, &state,
                                    &done) == FD_OK);
                fd_frame_cancel(&cancelled);
                CHECK(fd_frame_feed(&cancelled, wire, FD_WIRE_BYTES, next,
                                    &state,
                                    &done) == (next ? FD_OK : FD_DENIED));
                CHECK(state == (next ? FRAME_COUNTER_APPLIED
                                     : FRAME_COUNTER_START) &&
                      done == next);
                ++observations;
            }
        }
    }
    return 0;
}

/* Every way to split the record into nonempty chunks. The first chunk carries
 * the real authority and later chunks carry its negation, so the frame must
 * latch authority from the first chunk only. */
static int frame_all_chunkings(const unsigned char *wire)
{
    for (unsigned cuts = 0; cuts < FRAME_CUT_MASKS; ++cuts)
    {
        for (int authority = 0; authority < 2; ++authority)
        {
            struct fd_frame frame = {{0}, 0, 0};
            int32_t counter = FRAME_COUNTER_START;
            int committed = 77;
            size_t start = 0;
            for (size_t end = 1; end <= FD_WIRE_BYTES; ++end)
            {
                const int chunk_ends_here =
                    end == FD_WIRE_BYTES || (cuts & (1u << (end - 1)));
                if (!chunk_ends_here)
                {
                    continue;
                }
                const int is_last = end == FD_WIRE_BYTES;
                const int supplied = start == 0 ? authority : !authority;
                const enum fd_status status =
                    fd_frame_feed(&frame, wire + start, end - start, supplied,
                                  &counter, &committed);
                CHECK(status == (is_last && !authority ? FD_DENIED : FD_OK));
                CHECK(committed == (is_last && authority));
                CHECK(counter == (is_last && authority ? FRAME_COUNTER_APPLIED
                                                       : FRAME_COUNTER_START));
                start = end;
                ++observations;
            }
            CHECK(frame.used == 0);
        }
    }
    return 0;
}

/* Cancelled and rejected feeds: authority does not survive a cancel, and an
 * oversized feed leaves the frame byte-for-byte unchanged. */
static int frame_cancel_and_overflow(const unsigned char *wire)
{
    struct fd_frame frame = {{0}, 0, 0};
    int32_t counter = FRAME_COUNTER_START;
    int committed = 77;
    CHECK(fd_frame_feed(&frame, wire, 3, 1, &counter, &committed) == FD_OK);
    fd_frame_cancel(&frame);
    CHECK(fd_frame_feed(&frame, wire, FD_WIRE_BYTES, 0, &counter, &committed) ==
          FD_DENIED);
    CHECK(counter == FRAME_COUNTER_START && !committed);
    CHECK(fd_frame_feed(&frame, wire, 3, 0, &counter, &committed) == FD_OK);
    fd_frame_cancel(&frame);
    CHECK(fd_frame_feed(&frame, wire + 3, 5, 0, &counter, &committed) == FD_OK);
    const struct fd_frame before = frame;
    committed = 77;
    CHECK(fd_frame_feed(&frame, wire, 4, 1, &counter, &committed) == FD_RANGE);
    CHECK(frame.used == before.used && frame.authority == before.authority &&
          !memcmp(frame.bytes, before.bytes, sizeof frame.bytes));
    CHECK(counter == FRAME_COUNTER_START && committed == 77);
    fd_frame_cancel(&frame);
    CHECK(fd_frame_feed(&frame, wire, FD_WIRE_BYTES, 1, &counter, &committed) ==
          FD_OK);
    CHECK(counter == FRAME_COUNTER_APPLIED && committed == 1);
    return 0;
}

static int frame(void)
{
    /* Flag bit 0 set, to show that no flag grants authority. */
    const unsigned char wire[FD_WIRE_BYTES] = {1, 1, 0, 2, 0, 0, 0, 3};
    RUN_STEP(frame_effects());
    RUN_STEP(frame_cancel_after_prefix(wire));
    RUN_STEP(frame_all_chunkings(wire));
    RUN_STEP(frame_cancel_and_overflow(wire));
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
    for (size_t i = 0; i < sizeof cases / sizeof cases[0]; ++i)
    {
        if (!strcmp(argv[1], cases[i].name))
        {
            const int result = cases[i].run();
            if (!result)
            {
                printf("FD_TRANSFER_OK:%s:%lu\n", cases[i].name, observations);
            }
            return result;
        }
    }
    return 2;
}
