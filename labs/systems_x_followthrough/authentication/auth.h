#ifndef SYSTEMS_X_AUTH_H
#define SYSTEMS_X_AUTH_H
#include "b05.h"
typedef int (*sx_mac_fn)(void *key_context, const unsigned char *message,
                         size_t length, unsigned char tag[32]);

struct sx_authenticated
{
    struct b5_redo record;
    unsigned char tag[32];
};

int sx_auth_encode(uint64_t store_id, const struct b5_redo *,
                   unsigned char message[32]);
int sx_auth_recover(uint64_t store_id, uint32_t checkpoint, uint32_t base,
                    const struct sx_authenticated *, size_t count, sx_mac_fn,
                    void *key_context, uint32_t *out);
#endif
