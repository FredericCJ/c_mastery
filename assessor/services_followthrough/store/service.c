#include "service.h"
#include <string.h>

int sfs_apply(store *s, const struct sfs_request *q, struct sfs_reply *out)
{
    if (!s || !q || !out || !q->session || !q->id || q->client < 1 ||
        q->client > 2 || q->flags > 1)
        return SFS_ARGUMENT;
    struct sfs_reply r = {q->session, q->id, 0, 0, 0, {0}};
    if (q->operation == SFS_PUT)
    {
        if (!q->klen || q->klen > STORE_KEY_MAX || q->vlen > STORE_VALUE_MAX)
            return SFS_ARGUMENT;
        r.status = store_begin(s);
        if (!r.status)
        {
            r.status = store_put(s, q->key, q->klen, q->value, q->vlen);
            if (r.status)
                store_abort(s);
            else
                r.status = store_commit(s);
        }
    }
    else if (q->operation == SFS_GET)
    {
        if (!q->klen || q->klen > STORE_KEY_MAX)
            return SFS_ARGUMENT;
        size_t n = 0;
        r.status = store_get(s, q->key, q->klen, r.value, sizeof r.value, &n);
        if (!r.status)
            r.length = (uint32_t)n;
    }
    else if (q->operation == SFS_BATCH)
    {
        size_t n = 0;
        r.status = store_get(s, "left", 4, r.value, 32, &n);
        if (!r.status && n != 4)
            r.status = STORE_CORRUPT;
        if (!r.status)
        {
            r.status = store_get(s, "right", 5, r.value + 4, 32, &n);
            if (!r.status && n != 4)
                r.status = STORE_CORRUPT;
        }
        if (!r.status)
            r.length = 8;
    }
    else if (q->operation == SFS_INCREMENT)
    {
        if (q->id > UINT32_MAX)
            return SFS_ARGUMENT;
        r.status = sdr_apply(s, q->client, q->epoch, (uint32_t)q->id, q->delta,
                             &r.result);
    }
    else if (q->operation == SFS_VALUE)
        r.status = sdr_value(s, &r.result);
    else if (q->operation == SFS_CHECKPOINT)
        r.status = store_checkpoint(s);
    else
        return SFS_ARGUMENT;
    *out = r;
    return 0;
}
