#include "store_host.h"
#include "store.h"
#include <limits.h>

int cvm_store_host(void *ctx, uint8_t service, int32_t input, int32_t *output)
{
    static const char key[] = "vm-result";
    store *s = ctx;
    uint8_t bytes[4];
    size_t n = 0;
    uint32_t u;
    int status;
    if (!s || !output)
    {
        return STORE_INVALID;
    }
    if (service == 1)
    {
        u = (uint32_t)input;
        for (unsigned i = 0; i < 4; i++)
        {
            bytes[i] = (uint8_t)u;
            u >>= 8;
        }
        status = store_begin(s);
        if (status != STORE_OK)
        {
            return status;
        }
        status = store_put(s, key, sizeof key - 1, bytes, sizeof bytes);
        if (status != STORE_OK)
        {
            store_abort(s);
            return status;
        }
        status = store_commit(s);
        if (status != STORE_OK)
        {
            return status;
        }
        *output = input;
        return 0;
    }
    if (service == 2)
    {
        status = store_get(s, key, sizeof key - 1, bytes, sizeof bytes, &n);
        if (status != STORE_OK)
        {
            return status;
        }
        if (n != 4)
        {
            return STORE_CORRUPT;
        }
        u = (uint32_t)bytes[0] | ((uint32_t)bytes[1] << 8) |
            ((uint32_t)bytes[2] << 16) | ((uint32_t)bytes[3] << 24);
        *output = u <= INT32_MAX ? (int32_t)u
                                 : (int32_t)((int64_t)u - INT64_C(4294967296));
        return 0;
    }
    return STORE_INVALID;
}
