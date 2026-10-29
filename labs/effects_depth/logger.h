#ifndef CMASTERY_EFFECTS_LOGGER_H
#define CMASTERY_EFFECTS_LOGGER_H
#include <stddef.h>
#include <stdatomic.h>

struct ef_log
{
    atomic_flag owned;
    unsigned char bytes[32];
    size_t length;
};

/* Initialise owned with ATOMIC_FLAG_INIT. At most32 bytes; no retry/spin.
 * Fixed bounded critical region; caller must retain log/source lifetime.
 * A reader obtains the same ownership before reading and releasing. */
int ef_log(struct ef_log *, const unsigned char *, size_t);
#endif
