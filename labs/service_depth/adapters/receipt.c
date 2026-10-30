#include "receipt.h"

/* Learner scaffold: compiles, ignores its inputs and returns
 * SDR_UNIMPLEMENTED until you implement the contracts in receipt.h. */

int sdr_initialize(store *s)
{
    (void)s;
    return SDR_UNIMPLEMENTED;
}

int sdr_apply(store *s, uint32_t client, uint32_t epoch, uint32_t id,
              int32_t delta, int32_t *out)
{
    (void)s;
    (void)client;
    (void)epoch;
    (void)id;
    (void)delta;
    (void)out;
    return SDR_UNIMPLEMENTED;
}

int sdr_value(store *s, int32_t *out)
{
    (void)s;
    (void)out;
    return SDR_UNIMPLEMENTED;
}
