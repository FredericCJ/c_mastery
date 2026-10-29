#include "hardening.h"

unsigned sh_consume(const unsigned char *src, size_t count)
{
    unsigned sum = 0;
    for (size_t i = 0; i < count; ++i)
    {
        sum += src[i];
    }
    return sum;
}
