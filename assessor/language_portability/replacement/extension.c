#include "replacement.h"

void le_drop(struct le_owner *owner)
{
    if (!owner || !owner->data)
    {
        return;
    }
    struct le_owner old = *owner;
    *owner = (struct le_owner){0};
    old.ops.release(old.ops.context, old.id, old.data, old.size);
}

enum le_status le_run(struct le_ops ops, unsigned transfer,
                      struct le_owner *out)
{
    if (!out || out->data || !ops.acquire || !ops.release || transfer > 2)
    {
        return LE_INVALID;
    }
    [[gnu::cleanup(le_drop)]] struct le_owner a = {0};
    [[gnu::cleanup(le_drop)]] struct le_owner b = {0};
    a = (struct le_owner){
        .data = ops.acquire(ops.context, 1, 8), .size = 8, .id = 1, .ops = ops};
    if (!a.data)
    {
        return LE_NOMEM;
    }
    b = (struct le_owner){.data = ops.acquire(ops.context, 2, 16),
                          .size = 16,
                          .id = 2,
                          .ops = ops};
    if (!b.data)
    {
        return LE_NOMEM;
    }
    for (size_t i = 0; i < a.size; i++)
    {
        a.data[i] = (unsigned char)(i + 1);
    }
    for (size_t i = 0; i < b.size; i++)
    {
        b.data[i] = (unsigned char)(i + 17);
    }
    if (transfer)
    {
        struct le_owner *selected = transfer == 1 ? &a : &b;
        *out = *selected;
        *selected = (struct le_owner){0};
    }
    return LE_OK;
}
