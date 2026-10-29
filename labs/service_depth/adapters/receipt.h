#ifndef SD_RECEIPT_H
#define SD_RECEIPT_H
#include "store.h"

enum
{
    SDR_OK = 0,
    SDR_INVALID = -1,
    SDR_STALE = -2,
    SDR_CONFLICT = -3,
    SDR_OVERFLOW = -4,
    SDR_STORAGE = -5,
    SDR_UNCERTAIN = -6,
    SDR_UNIMPLEMENTED = -7
};

/* Two provisioned clients (1,2), one fixed nonzero epoch per client, three
 * remembered receipts each. New IDs strictly increase but need not be adjacent.
 * Evicted IDs remain stale through the durable high-water mark. */
int sdr_initialize(store *s);
int sdr_apply(store *s, uint32_t client, uint32_t epoch, uint32_t id,
              int32_t delta, int32_t *out);
int sdr_value(store *s, int32_t *out);
#endif
