#include <stdint.h>
#if defined(__SIZEOF_INT128__)
typedef unsigned __int128 wide;

wide divide_wide(wide n, wide d)
{
    return n / d;
}

wide multiply_wide(wide n, wide d)
{
    return n * d;
}

__int128 signed_divide(__int128 n, __int128 d)
{
    return n / d;
}
#else
typedef uint64_t wide;

wide divide_wide(wide n, wide d)
{
    return n / d;
}

wide multiply_wide(wide n, wide d)
{
    return n * d;
}

int64_t signed_divide(int64_t n, int64_t d)
{
    return n / d;
}
#endif
uint64_t convert_double(double x)
{
    return (uint64_t)x;
}

double convert_wide(uint64_t x)
{
    return (double)x;
}
