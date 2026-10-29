#include "evolution_v1.h"

int ac_transform_v1(const unsigned char *input, size_t count,
                    ac_borrowed_fn callback, void *context, size_t *written)
{
    unsigned char output[8];
#ifdef AC_MUTANT_FAILURE_OUTPUT
    if (written)
        *written = 0;
#endif
    if ((count && !input) || count > sizeof output || !callback || !written)
        return 1;
    for (size_t i = 0; i < count; ++i)
        output[i] = input[i] ^ 0x20u;
    /* Caller callback may have effects: only this core has no allocation. */
    callback(context, output, count);
    *written = count;
    return 0;
}
