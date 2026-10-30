#ifndef CMASTERY_EFFECTS_LOGGER_H
#define CMASTERY_EFFECTS_LOGGER_H
#include <stddef.h>
#include <stdatomic.h>

/* Capacity of one log entry, in bytes. */
#define EF_LOG_BYTES 32u

/*
 * One-slot log guarded by a single atomic flag. Initialize owned with
 * ATOMIC_FLAG_INIT. A reader must obtain the same flag before reading bytes
 * and length, and release it afterwards.
 */
struct ef_log
{
    atomic_flag owned;
    unsigned char bytes[EF_LOG_BYTES];
    size_t length;
};

/*
 * Tries once to take ownership of the log, copy length bytes (at most
 * EF_LOG_BYTES), publish the length and release ownership. There is no retry
 * and no spinning, so the critical region has a fixed bound.
 *
 * Returns 1 when the entry was stored, 0 when the log was owned by someone
 * else (the log is unchanged) and -1 for invalid input (a null log, a null
 * source with a nonzero length, or a length above EF_LOG_BYTES).
 *
 * The caller keeps the log and the source bytes alive for the call.
 */
int ef_log(struct ef_log *target, const unsigned char *data, size_t length);
#endif
