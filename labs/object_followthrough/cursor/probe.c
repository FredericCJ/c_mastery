/*
 * Exhaustive enumeration of the cursor operations over every admitted
 * interval of an 8-byte array (cursor lengths 0..8, every position up to the
 * length). Requests (counts, offsets) run past the array on purpose. One CSV
 * line per case:
 *   take,length,position,count,ok,position_after,index_after
 *   reverse,length,position,count,ok,position_after,index_after
 *   sub,length,position,offset,count,ok,base_index,length_after,position_after
 * ok is 1 for FD_OK. index_after / base_index is the offset of the returned
 * pointer from the array start, or -1 if it is not inside [0, ARRAY_BYTES].
 */
#include "cursor.h"
#include <stdio.h>

enum
{
    ARRAY_BYTES = 8,
    MAX_REQUEST = 10 /* beyond ARRAY_BYTES so rejections are enumerated */
};

/* Offset of p from the start of the array, or -1 if it is not one of the
 * ARRAY_BYTES + 1 addresses that can be formed in or just past it. */
static int index_of(const unsigned char *array, const unsigned char *p)
{
    for (int i = 0; i <= ARRAY_BYTES; ++i)
    {
        if (p == array + i)
        {
            return i;
        }
    }
    return -1;
}

int main(void)
{
    unsigned char a[ARRAY_BYTES] = {0};
    for (size_t n = 0; n <= ARRAY_BYTES; ++n)
    {
        for (size_t position = 0; position <= n; ++position)
        {
            for (size_t count = 0; count <= MAX_REQUEST; ++count)
            {
                struct fd_cursor c = {a, n, position};
                const unsigned char *p = a + ARRAY_BYTES;
                int ok = oc_take(&c, count, &p) == FD_OK;
                printf("take,%zu,%zu,%zu,%d,%zu,%d\n", n, position, count, ok,
                       c.position, index_of(a, p));
                c = (struct fd_cursor){a, n, position};
                p = a + ARRAY_BYTES;
                ok = oc_previous(&c, count, &p) == FD_OK;
                printf("reverse,%zu,%zu,%zu,%d,%zu,%d\n", n, position, count,
                       ok, c.position, index_of(a, p));
            }
            for (size_t offset = 0; offset <= MAX_REQUEST; ++offset)
            {
                for (size_t count = 0; count <= MAX_REQUEST; ++count)
                {
                    struct fd_cursor c = {a, n, position};
                    int ok = oc_subview(&c, offset, count, &c) == FD_OK;
                    printf("sub,%zu,%zu,%zu,%zu,%d,%d,%zu,%zu\n", n, position,
                           offset, count, ok, index_of(a, c.base), c.length,
                           c.position);
                }
            }
        }
    }
    return 0;
}
