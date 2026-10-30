/* Header independence: the header is the first include of this unit. */
#include <headers.h>
#include <limits.h>

int hh_header_first(const int *p, size_t n)
{
    int out;
    return hh_read(p, n, 0, &out);
}
