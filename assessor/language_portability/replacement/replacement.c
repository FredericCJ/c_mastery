#include "replacement.h"
#ifndef LE_MUTANT
#define LE_MUTANT 0
#endif
/* Release the owned resource and leave *owner empty. The slot is cleared
   before the callback runs, so a callback that observes it sees no owner and
   a repeated drop is a no-op. */
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

/* Acquire resources 1 (8 bytes) then 2 (16 bytes), fill them, and release in
   reverse order. transfer 1 or 2 moves that resource into *out instead of
   releasing it. Returns LE_INVALID (bad arguments, no callback run), LE_NOMEM
   (an acquire failed; earlier resources are released) or LE_OK. */
enum le_status le_run(struct le_ops ops, unsigned transfer,
                      struct le_owner *out)
{
    if (!out || out->data || !ops.acquire || !ops.release || transfer > 2)
    {
        return LE_INVALID;
    }
    struct le_owner a = {0};
    struct le_owner b = {0};
    enum le_status result = LE_NOMEM;
    a = (struct le_owner){
        .data = ops.acquire(ops.context, 1, 8), .size = 8, .id = 1, .ops = ops};
    if (!a.data)
    {
        goto done;
    }
    b = (struct le_owner){.data = ops.acquire(ops.context, 2, 16),
                          .size = 16,
                          .id = 2,
                          .ops = ops};
    if (!b.data)
    {
        goto done;
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
#if LE_MUTANT != 3
        *selected = (struct le_owner){0};
#endif
    }
    result = LE_OK;
done:
#if LE_MUTANT == 2
    le_drop(&a);
    le_drop(&b);
#else
    le_drop(&b);
#if LE_MUTANT == 1
    if (result == LE_OK)
    {
        le_drop(&a);
    }
#else
    le_drop(&a);
#endif
#endif
    return result;
}
