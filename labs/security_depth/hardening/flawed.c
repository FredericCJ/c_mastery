/* Original isolated teaching target. Never run count>8 without the
 * explicit fortification/ASan sensitivity campaign. No exploit payload. */
#include "hardening.h"
#include <string.h>

unsigned sh_flawed_copy(const unsigned char *src, size_t count)
{
    unsigned char local[8] = {0};
    memcpy(local, src, count);
    return sh_consume(local, sizeof local);
}

int sh_flawed_authorize(unsigned claimed_role, int trusted, unsigned *counter)
{
    (void)trusted;
    if (claimed_role == 1)
    {
        ++*counter;
        return 0;
    }
    return -1;
}

unsigned sh_optimized_away(unsigned x)
{
    unsigned local[8] = {x};
    return local[0];
}
