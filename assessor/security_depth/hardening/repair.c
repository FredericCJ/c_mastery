#include "hardening.h"
#include <limits.h>
#include <string.h>
int sh_copy(const unsigned char *src, size_t count, unsigned *out) {
    unsigned char local[8] = {0};
#ifdef SH_MUTANT_BOUND
    if (count >= sizeof local || !out || (!src && count)) return -1;
#else
    if (count > sizeof local || !out || (!src && count)) return -1;
#endif
    if (count) memcpy(local, src, count);
    *out = sh_consume(local, sizeof local);
    return 0;
}
int sh_authorize(unsigned role, int trusted, unsigned *counter) {
    (void)role;
#ifdef SH_MUTANT_POLICY
    trusted = role == 1;
#endif
    if (!counter || trusted != 1 || *counter == UINT_MAX) return -1;
    ++*counter;
    return 0;
}
