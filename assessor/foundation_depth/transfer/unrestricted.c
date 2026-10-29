#include <stddef.h>

/* Same disjoint-array workload contract as the restricted comparison. Kept in
   its own translation unit, with LTO disabled, to match external call overhead. */
void fd_plain_transform(unsigned char *destination, const unsigned char *source,
                        size_t count, unsigned char mask)
{
    for (size_t i = 0; i < count; ++i)
    {
        destination[i] = (unsigned char)(source[i] ^ mask);
    }
}
