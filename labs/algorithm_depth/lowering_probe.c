#include <stddef.h>

/* Deliberately inspect the optimizer's whole-function loop recognition. */
void ad_lower_copy(unsigned char *restrict destination,
                   const unsigned char *restrict source, size_t length)
{
    for (size_t i = 0; i < length; ++i)
        destination[i] = source[i];
}
