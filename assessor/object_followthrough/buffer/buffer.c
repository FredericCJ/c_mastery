/*
 * Reference implementation of the buffer laboratory (private assessor copy).
 * Contracts are stated in labs/object_followthrough/buffer/buffer.h and
 * CONTRACTS.md; this file adds implementation notes only.
 *
 * Test hooks: each OB_MUTANT_* macro breaks exactly one contract so the
 * campaign can prove that the tests reject the breakage.
 */
#include "buffer.h"

/* Bits of struct ob_construction.initialized: one per member, set only after
 * that member's store completed. */
enum
{
    OB_HAS_NUMBER = 1u,
    OB_HAS_POINTER = 2u,
    OB_HAS_TAG = 4u
};

/* ob_construct() failure injection points (its fail_after argument). */
enum
{
    OB_FAIL_AFTER_NUMBER = 1,
    OB_FAIL_AFTER_POINTER = 2
};

/* Four-byte record encoding: number (big-endian, 16 bits), tag, pointer
 * flag. */
enum
{
    OB_ENCODED_BYTES = 4,
    OB_ENCODED_NUMBER_MAX = 65535,
    OB_OCTET_BITS = 8
};

/* A table's generation starts at 1 and saturates at OB_MAX_GENERATION, so at
 * most OB_MAX_GENERATION - 1 invalidating operations (resize or erase)
 * succeed. A shared block can have OB_MAX_REFERENCES references. Both limits
 * are finite on purpose so that exhaustion can be tested, and neither wraps. */
enum
{
    OB_MAX_GENERATION = 3,
    OB_MAX_REFERENCES = 3
};

static int allocator_valid(struct ob_allocator allocator)
{
    return allocator.allocate != NULL && allocator.resize != NULL &&
           allocator.release != NULL;
}

/* A table that currently owns a block and has not been retired. */
static int table_is_live(const struct ob_table *table)
{
    return table != NULL && table->value != NULL && !table->retired;
}

static void zero_fill(unsigned char *bytes, size_t from, size_t to)
{
    for (size_t i = from; i < to; ++i)
    {
        bytes[i] = 0;
    }
}

int ob_construct(struct ob_construction *construction, int number,
                 void *pointer, char tag, unsigned fail_after)
{
    if (construction == NULL)
    {
        return -1;
    }
    /* The mask is cleared before the first store and extended after each
     * one, so every failure edge leaves it describing exactly what is set. */
    construction->initialized = 0;
    construction->value.number = number;
    construction->initialized = OB_HAS_NUMBER;
    if (fail_after == OB_FAIL_AFTER_NUMBER)
    {
        return -1;
    }
    construction->value.pointer = pointer;
    construction->initialized |= OB_HAS_POINTER;
    if (fail_after == OB_FAIL_AFTER_POINTER)
    {
        return -1;
    }
    construction->value.tag = tag;
    construction->initialized |= OB_HAS_TAG;
    return 0;
}

int ob_record_equal(const struct ob_record *left, const struct ob_record *right)
{
    return left != NULL && right != NULL && left->number == right->number &&
           left->pointer == right->pointer && left->tag == right->tag;
}

int ob_record_encode(const struct ob_record *record, unsigned char *out,
                     size_t capacity_bytes)
{
    if (record == NULL || out == NULL || capacity_bytes < OB_ENCODED_BYTES ||
        record->number < 0 || record->number > OB_ENCODED_NUMBER_MAX)
    {
        return -1;
    }
    out[0] = (unsigned char)((unsigned)record->number >> OB_OCTET_BITS);
    out[1] = (unsigned char)record->number;
    out[2] = (unsigned char)record->tag;
    out[3] = (unsigned char)(record->pointer != NULL);
    return 0;
}

int ob_cleanup(struct ob_construction *construction,
               struct ob_allocator allocator)
{
    if (construction == NULL || allocator.release == NULL)
    {
        return -1;
    }
#ifdef OB_MUTANT_CLEANUP
    if (construction->initialized & OB_HAS_NUMBER)
    {
        /* False release event, without reading an uninitialized pointer. */
        allocator.release(allocator.context, NULL);
    }
#else
    /* The pointer is read only when its own bit says it was stored. */
    if ((construction->initialized & OB_HAS_POINTER) != 0 &&
        construction->value.pointer != NULL)
    {
        allocator.release(allocator.context, construction->value.pointer);
    }
#endif
    construction->initialized = 0;
    return 0;
}

/*
 * The product is tested as count > (limit - prefix) / width, which cannot
 * overflow, before prefix + count * width is formed.
 */
int ob_size(size_t prefix_bytes, size_t width_bytes, size_t count,
            size_t limit_bytes, size_t *out)
{
    if (out == NULL || width_bytes == 0 || prefix_bytes > limit_bytes ||
        count > (limit_bytes - prefix_bytes) / width_bytes)
    {
        return -1;
    }
#ifdef OB_MUTANT_SIZE
    *out = prefix_bytes;
#else
    *out = prefix_bytes + count * width_bytes;
#endif
    return 0;
}

int ob_create(struct ob_table *table, struct ob_allocator allocator,
              uint32_t identity, size_t capacity)
{
    size_t bytes = 0;
    if (table == NULL || table->value != NULL || table->retired ||
        identity == 0 || !allocator_valid(allocator) ||
        ob_size(sizeof(struct ob_flex), 1, capacity, SIZE_MAX, &bytes) != 0)
    {
        return -1;
    }
    struct ob_flex *block = allocator.allocate(allocator.context, bytes);
    if (block == NULL)
    {
        return -1;
    }
    block->length = 0;
    block->capacity = capacity;
    zero_fill(block->payload, 0, capacity);
    table->value = block;
    table->allocation = allocator;
    table->identity = identity;
    table->generation = 1;
    return 0;
}

int ob_resize(struct ob_table *table, size_t capacity)
{
    size_t bytes = 0;
    if (!table_is_live(table) || capacity == 0 ||
        table->generation >= OB_MAX_GENERATION ||
        ob_size(sizeof(struct ob_flex), 1, capacity, SIZE_MAX, &bytes) != 0)
    {
        return -1;
    }
    const size_t old_capacity = table->value->capacity;
    const size_t old_bytes = sizeof(struct ob_flex) + old_capacity;
    struct ob_flex *moved = table->allocation.resize(
        table->allocation.context, table->value, old_bytes, bytes);
#ifdef OB_MUTANT_RESIZE
    if (moved == NULL)
    {
        ++table->generation;
        return -1;
    }
#else
    if (moved == NULL)
    {
        return -1;
    }
#endif
    /* After a successful resize the old pointer must not be evaluated, even
     * for comparison: only `moved` is used from here on. */
    table->value = moved;
    ++table->generation;
    if (moved->length > capacity)
    {
        moved->length = capacity;
    }
    moved->capacity = capacity;
    zero_fill(moved->payload, old_capacity, capacity);
    return 0;
}

int ob_append(struct ob_table *table, unsigned char byte)
{
    if (!table_is_live(table) || table->value->length == table->value->capacity)
    {
        return -1;
    }
    table->value->payload[table->value->length] = byte;
    ++table->value->length;
    return 0;
}

int ob_erase(struct ob_table *table, size_t index)
{
    if (!table_is_live(table) || index >= table->value->length ||
        table->generation >= OB_MAX_GENERATION)
    {
        return -1;
    }
    struct ob_flex *flex = table->value;
    for (size_t i = index; i + 1 < flex->length; ++i)
    {
        flex->payload[i] = flex->payload[i + 1];
    }
    --flex->length;
#ifndef OB_MUTANT_ERASE
    ++table->generation;
#endif
    return 0;
}

void ob_reset(struct ob_table *table)
{
    if (table == NULL)
    {
        return;
    }
    if (table->value != NULL)
    {
        table->allocation.release(table->allocation.context, table->value);
    }
    table->value = NULL;
    /* Retired for good, even if generations remain: a new owner needs a new
     * identity. */
    table->retired = 1;
}

int ob_handle(const struct ob_table *table, size_t index, struct ob_handle *out)
{
    if (out == NULL || !table_is_live(table) || index >= table->value->length)
    {
        return -1;
    }
    *out = (struct ob_handle){table->identity, table->generation, index};
    return 0;
}

int ob_resolve(const struct ob_table *table, struct ob_handle handle,
               unsigned char *out)
{
    if (out == NULL || !table_is_live(table) ||
        handle.identity != table->identity ||
        handle.index >= table->value->length)
    {
        return -1;
    }
#ifndef OB_MUTANT_GENERATION
    if (handle.generation != table->generation)
    {
        return -1;
    }
#endif
    *out = table->value->payload[handle.index];
    return 0;
}

int ob_table_move(struct ob_table *to, struct ob_table *from)
{
    if (to == NULL || from == NULL || to == from || to->value != NULL ||
        to->retired || from->value == NULL || from->retired)
    {
        return -1;
    }
    *to = *from;
    /* Ownership moved: the source must never release or resolve again. */
    from->value = NULL;
    from->identity = 0;
    from->retired = 1;
    return 0;
}

int ob_split_create(struct ob_allocator allocator, size_t capacity,
                    struct ob_split **out)
{
    if (out == NULL || !allocator_valid(allocator))
    {
        return -1;
    }
    struct ob_split *split =
        allocator.allocate(allocator.context, sizeof *split);
    if (split == NULL)
    {
        return -1;
    }
    split->length = 0;
    split->capacity = capacity;
    split->payload = NULL;
    split->allocation = allocator;
    if (capacity != 0)
    {
        split->payload = allocator.allocate(allocator.context, capacity);
        if (split->payload == NULL)
        {
            /* Roll back the header: nothing is published. */
            allocator.release(allocator.context, split);
            return -1;
        }
        zero_fill(split->payload, 0, capacity);
    }
    *out = split;
    return 0;
}

void ob_split_drop(struct ob_split **owner)
{
    if (owner == NULL || *owner == NULL)
    {
        return;
    }
    struct ob_split *split = *owner;
    /* Copy the allocator first: it lives inside the block being released. */
    struct ob_allocator allocator = split->allocation;
    if (split->payload != NULL)
    {
        allocator.release(allocator.context, split->payload);
    }
    allocator.release(allocator.context, split);
    *owner = NULL;
}

int ob_shared_create(struct ob_allocator allocator, const unsigned char *bytes,
                     size_t n, struct ob_shared **out)
{
    size_t total_bytes = 0;
    if (out == NULL || (bytes == NULL && n != 0) ||
        !allocator_valid(allocator) ||
        ob_size(sizeof(struct ob_shared), 1, n, SIZE_MAX, &total_bytes) != 0)
    {
        return -1;
    }
    struct ob_shared *shared =
        allocator.allocate(allocator.context, total_bytes);
    if (shared == NULL)
    {
        return -1;
    }
    shared->references = 1;
    shared->length = n;
    shared->allocation = allocator;
    for (size_t i = 0; i < n; ++i)
    {
        shared->payload[i] = bytes[i];
    }
    *out = shared;
    return 0;
}

int ob_shared_retain(struct ob_shared *shared, struct ob_shared **out)
{
    if (shared == NULL || out == NULL || shared->references == 0 ||
        shared->references >= OB_MAX_REFERENCES)
    {
        return -1;
    }
#ifndef OB_MUTANT_SHARED
    ++shared->references;
#endif
    *out = shared;
    return 0;
}

void ob_shared_drop(struct ob_shared **owner)
{
    if (owner == NULL || *owner == NULL)
    {
        return;
    }
    struct ob_shared *shared = *owner;
    *owner = NULL;
    --shared->references;
    if (shared->references == 0)
    {
        /* Copy the allocator first: it lives inside the block. */
        struct ob_allocator allocator = shared->allocation;
        allocator.release(allocator.context, shared);
    }
}
