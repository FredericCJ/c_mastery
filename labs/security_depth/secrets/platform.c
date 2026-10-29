/* Explicitly declared Linux/glibc or musl primitive. Not ISO C17. */
#define _DEFAULT_SOURCE
#include "secrets.h"
#include <string.h>

void ss_wipe(void *p, size_t count)
{
    if (count)
        explicit_bzero(p, count);
}
