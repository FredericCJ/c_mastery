#include "strides.h"
#ifndef OSV_MUTATION
#define OSV_MUTATION 0
#endif
enum ow_status osv_indices(size_t extent, struct osv_shape s, size_t out[64],
                           size_t *count)
{
    if (!out || !count || s.offset > extent || s.rows > 64 || s.columns > 64)
    {
        return OW_INVALID;
    }
    if (!s.rows || !s.columns)
    {
        *count = 0;
        return OW_OK;
    }
    if (s.columns > 64 / s.rows || s.offset == extent)
    {
        return OW_INVALID;
    }
    size_t room = extent - 1 - s.offset;
    if (s.row_stride && (s.rows - 1) > room / s.row_stride)
    {
        return OW_INVALID;
    }
    room -= (s.rows - 1) * s.row_stride;
    if (s.column_stride && (s.columns - 1) > room / s.column_stride)
    {
        return OW_INVALID;
    }
    size_t staged[64], n = 0;
    for (size_t r = 0; r < s.rows; ++r)
    {
        for (size_t c = 0; c < s.columns; ++c)
        {
            size_t index = s.offset + r * s.row_stride + c * s.column_stride;
            for (size_t j = 0; j < n; ++j)
            {
                if (staged[j] == index && OSV_MUTATION != 3)
                {
                    return OW_INVALID;
                }
            }
            staged[n++] = index;
        }
    }
    for (size_t i = 0; i < n; ++i)
    {
        out[i] = staged[i];
    }
    *count = n;
    return OW_OK;
}

static int live(const struct osv_book *b, struct osv_token t)
{
    return b && t.owner == b && t.slot < 4 && b->loan[t.slot].active &&
           (OSV_MUTATION == 5 || b->loan[t.slot].serial == t.serial);
}

static int conflict(const size_t *a, size_t na, const size_t *b, size_t nb)
{
    if (OSV_MUTATION == 1 && na && nb)
    {
        return a[0] <= b[nb - 1] && b[0] <= a[na - 1];
    }
    for (size_t i = 0; i < na; ++i)
    {
        for (size_t j = 0; j < nb; ++j)
        {
            if (a[i] == b[j])
            {
                return 1;
            }
        }
    }
    return 0;
}

enum ow_status osv_acquire(struct osv_book *b, struct osv_shape s, int writer,
                           struct osv_token *out)
{
    if (!b || !out || (writer != 0 && writer != 1))
    {
        return OW_INVALID;
    }
    size_t indices[64], n;
    if (osv_indices(b->extent, s, indices, &n) != OW_OK)
    {
        return OW_INVALID;
    }
    unsigned slot = 4;
    for (unsigned i = 0; i < 4; ++i)
    {
        struct osv_loan *l = &b->loan[i];
        if (l->active && (writer || l->writer))
        {
            size_t prior[64], m;
            if (osv_indices(b->extent, l->shape, prior, &m) != OW_OK)
            {
                return OW_INVALID;
            }
            if (conflict(indices, n, prior, m) && OSV_MUTATION != 2)
            {
                return OW_BUSY;
            }
        }
        if (!l->active && !l->retired && l->serial < UINT32_MAX && slot == 4)
        {
            slot = i;
        }
    }
    if (slot == 4)
    {
        return OW_FULL;
    }
    struct osv_loan *l = &b->loan[slot];
    ++l->serial;
    l->shape = s;
    l->writer = (unsigned)writer;
    l->active = 1;
    *out = (struct osv_token){b, slot, l->serial};
    return OW_OK;
}

enum ow_status osv_release(struct osv_book *b, struct osv_token t)
{
    if (!live(b, t))
    {
        return OW_STALE;
    }
    struct osv_loan *l = &b->loan[t.slot];
    l->active = 0;
    if (l->serial == UINT32_MAX)
    {
        l->retired = 1;
    }
    return OW_OK;
}

enum ow_status osv_split(struct osv_book *b, struct osv_token parent,
                         unsigned axis, size_t cut, struct osv_token out[2])
{
    if (!out || !live(b, parent))
    {
        return OW_STALE;
    }
    struct osv_loan l = b->loan[parent.slot];
    size_t dimension = axis == 0 ? l.shape.rows : l.shape.columns;
    if (axis > 1 || !l.writer || cut > dimension)
    {
        return OW_INVALID;
    }
    struct osv_shape left = l.shape, right = l.shape;
    if (axis == 0)
    {
        left.rows = cut;
        right.rows -= cut;
        if (right.rows && right.columns)
        {
            right.offset +=
                cut * (OSV_MUTATION == 6 ? right.columns : right.row_stride);
        }
    }
    else
    {
        left.columns = cut;
        right.columns -= cut;
        if (right.rows && right.columns)
        {
            right.offset += cut * right.column_stride;
        }
    }
    struct osv_book next = *b;
    next.loan[parent.slot].active = 0;
    if (l.serial == UINT32_MAX)
    {
        next.loan[parent.slot].retired = 1;
    }
    struct osv_token children[2];
    enum ow_status status = osv_acquire(&next, left, 1, &children[0]);
    if (status == OW_OK)
    {
        status = osv_acquire(&next, right, 1, &children[1]);
    }
    if (status != OW_OK)
    {
        if (OSV_MUTATION == 4)
        {
            *b = next;
        }
        return status;
    }
    *b = next;
    children[0].owner = b;
    children[1].owner = b;
    out[0] = children[0];
    out[1] = children[1];
    return OW_OK;
}

static enum ow_status at(const struct osv_book *b, struct osv_token t,
                         size_t extent, size_t row, size_t column,
                         size_t *index)
{
    if (!live(b, t))
    {
        return OW_STALE;
    }
    struct osv_shape s = b->loan[t.slot].shape;
    if (extent != b->extent || row >= s.rows || column >= s.columns)
    {
        return OW_INVALID;
    }
    *index = s.offset + row * s.row_stride + column * s.column_stride;
    return OW_OK;
}

enum ow_status osv_read(const struct osv_book *b, struct osv_token t,
                        const int32_t *data, size_t extent, size_t row,
                        size_t column, int32_t *out)
{
    if (!data || !out)
    {
        return OW_INVALID;
    }
    size_t index;
    enum ow_status s = at(b, t, extent, row, column, &index);
    if (s != OW_OK)
    {
        return s;
    }
    *out = data[index];
    return OW_OK;
}

enum ow_status osv_write(struct osv_book *b, struct osv_token t, int32_t *data,
                         size_t extent, size_t row, size_t column,
                         int32_t value)
{
    if (!data)
    {
        return OW_INVALID;
    }
    size_t index;
    enum ow_status s = at(b, t, extent, row, column, &index);
    if (s != OW_OK)
    {
        return s;
    }
    if (!b->loan[t.slot].writer)
    {
        return OW_BUSY;
    }
    data[index] = value;
    return OW_OK;
}
