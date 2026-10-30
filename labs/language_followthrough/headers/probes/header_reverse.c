/* Header independence: <limits.h> first, then the header included twice. */
#include <limits.h>
#include <headers.h>
#include <headers.h>

int hh_header_reverse(const int *p, size_t n)
{
    int out;
    return hh_read(p, n, 0, &out);
}
