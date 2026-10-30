/*
 * Reference implementation of the cursor laboratory (private assessor copy).
 * Contracts are stated in labs/object_followthrough/cursor/cursor.h and
 * CONTRACTS.md; this file adds implementation notes only. It is profile C:
 * no library calls, so it also links into the freestanding target image.
 *
 * Test hooks: each OC_MUTANT_* macro breaks exactly one contract so the
 * campaign can prove that the tests reject the breakage.
 */
#include "cursor.h"

enum
{
    OC_ROW_COLUMNS = 3, /* ints per row of struct oc_rows */
    OC_INDEX_BYTES = 4, /* wire size of an encoded index */
    OC_OCTET_BITS = 8
};

/* Wire value of a null reference; also why count may not exceed UINT32_MAX:
 * the largest valid index must stay below it. */
#define OC_NULL_INDEX UINT32_MAX

int oc_rows_read(void *context, size_t row, size_t col, int *out)
{
    const struct oc_rows *rows = context;
    if (rows == NULL || out == NULL || rows->rows == NULL ||
        row >= rows->count || col >= OC_ROW_COLUMNS)
    {
        return -1;
    }
#ifdef OC_MUTANT_ROW
    *out = rows->rows[0][col];
#else
    *out = rows->rows[row][col];
#endif
    return 0;
}

int oc_table_read(void *context, size_t row, size_t col, int *out)
{
    const struct oc_table *table = context;
    if (table == NULL || out == NULL || table->rows == NULL ||
        table->lengths == NULL || row >= table->count)
    {
        return -1;
    }
    /* Each row has its own length and its own allocation. */
    if (col >= table->lengths[row] || table->rows[row] == NULL)
    {
        return -1;
    }
    *out = table->rows[row][col];
    return 0;
}

int oc_read(const struct oc_matrix *matrix, size_t row, size_t col, int *out)
{
    if (matrix == NULL || matrix->read == NULL || out == NULL ||
        row >= matrix->rows || col >= matrix->columns)
    {
        return -1;
    }
    return matrix->read(matrix->context, row, col, out);
}

int oc_representation(int (*matrix)[2][3], unsigned char *out,
                      size_t capacity_bytes)
{
    if (matrix == NULL || out == NULL || capacity_bytes != sizeof(*matrix))
    {
        return -1;
    }
    /* A pointer to the whole object, so all sizeof(*matrix) bytes are in
     * its extent; a pointer derived from row 0 alone would not cover them. */
    const unsigned char *bytes = (const unsigned char *)matrix;
    for (size_t i = 0; i < sizeof(*matrix); ++i)
    {
        out[i] = bytes[i];
    }
    return 0;
}

enum fd_status oc_take(struct fd_cursor *cursor, size_t count,
                       const unsigned char **out)
{
    return fd_take(cursor, count, out);
}

enum fd_status oc_subview(const struct fd_cursor *cursor, size_t offset,
                          size_t count, struct fd_cursor *out)
{
    return fd_subview(cursor, offset, count, out);
}

enum fd_status oc_previous(struct fd_cursor *cursor, size_t count,
                           const unsigned char **out)
{
    if (cursor == NULL || out == NULL || cursor->position > cursor->length ||
        (cursor->base == NULL && cursor->length != 0))
    {
        return FD_INVALID;
    }
    /* Admit the count as an integer before forming any pointer. */
    if (count > cursor->position)
    {
#ifdef OC_MUTANT_REVERSE
        cursor->position = 0;
#endif
        return FD_RANGE;
    }
    const size_t previous = cursor->position - count;
    *out = cursor->base != NULL ? cursor->base + previous : NULL;
    cursor->position = previous;
    return FD_OK;
}

int oc_difference(const unsigned char *base, size_t length, size_t left,
                  size_t right, ptrdiff_t *out)
{
    if (out == NULL || base == NULL || left > length || right > length)
    {
        return -1;
    }
    const size_t magnitude = left > right ? left - right : right - left;
    if (magnitude > (size_t)PTRDIFF_MAX)
    {
        return -1;
    }
    /* Both operands point into (or one past) the same array. */
    *out = (base + left) - (base + right);
    return 0;
}

/*
 * Logical offset of a position within fragments of the given lengths: the
 * total length is first proven to fit size_t, so the prefix sum below cannot
 * wrap. No address is involved.
 */
static int logical_offset(const size_t *lengths, size_t count,
                          struct oc_position where, size_t *out)
{
    if (lengths == NULL || where.segment >= count ||
        where.index > lengths[where.segment])
    {
        return -1;
    }
    size_t total = 0;
    for (size_t i = 0; i < count; ++i)
    {
        if (lengths[i] > SIZE_MAX - total)
        {
            return -1;
        }
        total += lengths[i];
    }
    size_t before = 0;
    for (size_t i = 0; i < where.segment; ++i)
    {
        before += lengths[i];
    }
    *out = before + where.index;
    return 0;
}

int oc_order(const size_t *lengths, size_t count, struct oc_position left,
             struct oc_position right, int *out)
{
    size_t left_offset = 0;
    size_t right_offset = 0;
    if (out == NULL ||
        logical_offset(lengths, count, left, &left_offset) != 0 ||
        logical_offset(lengths, count, right, &right_offset) != 0)
    {
        return -1;
    }
#ifdef OC_MUTANT_FRAGMENT
    left_offset = left.index;
    right_offset = right.index;
#endif
    if (left_offset < right_offset)
    {
        *out = -1;
    }
    else if (left_offset > right_offset)
    {
        *out = 1;
    }
    else
    {
        *out = 0;
    }
    return 0;
}

/*
 * Finds pointer among base[0..count) by pointer equality, which is only
 * meaningful for pointers into the same array, hence the search instead of
 * subtraction. Returns 0 and the index, or -1 if there is no match.
 */
static int element_index(const int *base, size_t count, const int *pointer,
                         size_t *index)
{
    for (size_t i = 0; i < count; ++i)
    {
        if (pointer == &base[i])
        {
            *index = i;
            return 0;
        }
    }
    return -1;
}

int oc_encode_index(int *base, size_t count, const int *pointer,
                    unsigned char *out)
{
    if (out == NULL || (base == NULL && count != 0))
    {
        return -1;
    }
#if SIZE_MAX > UINT32_MAX
    if (count > UINT32_MAX)
    {
        return -1;
    }
#endif
    uint32_t index = OC_NULL_INDEX;
    if (pointer != NULL)
    {
        size_t found = 0;
        if (element_index(base, count, pointer, &found) != 0)
        {
            return -1;
        }
        index = (uint32_t)found;
    }
    for (size_t i = 0; i < OC_INDEX_BYTES; ++i)
    {
        out[i] = (unsigned char)(index >>
                                 (OC_OCTET_BITS * (OC_INDEX_BYTES - 1 - i)));
    }
    return 0;
}

int oc_decode_index(int *base, size_t count, const unsigned char *wire,
                    int **out)
{
    if (wire == NULL || out == NULL || (base == NULL && count != 0))
    {
        return -1;
    }
#if SIZE_MAX > UINT32_MAX
    if (count > UINT32_MAX)
    {
        return -1;
    }
#endif
    uint32_t index = 0;
    for (size_t i = 0; i < OC_INDEX_BYTES; ++i)
    {
        index = (index << OC_OCTET_BITS) | wire[i];
    }
    if (index == OC_NULL_INDEX)
    {
#ifdef OC_MUTANT_NULL
        return -1;
#else
        *out = NULL;
        return 0;
#endif
    }
    if (index >= count)
    {
        return -1;
    }
    *out = &base[index];
    return 0;
}

static int identity(int x)
{
    return x;
}

int oc_roundtrip(int *pointer)
{
    /* Object pointer through void *, function pointer through another
     * function pointer type: both are restored to the original type before
     * use. */
    void *untyped = pointer;
    int *again = untyped;
    int (*function)(int) = identity;
    void (*other)(void) = (void (*)(void))function;
    int (*restored)(int) = (int (*)(int))other;
    if (again != pointer || restored != function || restored(7) != 7)
    {
        return -1;
    }
    return 0;
}
