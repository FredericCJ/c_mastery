#include "api_depth.h"
#ifndef API_MUTANT
#define API_MUTANT 0
#endif
ApiStatus api_append(ApiBuffer *b, const unsigned char *s, size_t n) {
    if (!b || b->capacity > 32 || b->length > b->capacity || (n && !s))
        return API_INVALID;
    if (n > b->capacity - b->length) {
        if (API_MUTANT == 2) b->length = b->capacity;
        return API_FULL;
    }
    /* Pointer equality only: no relational comparison across unrelated arrays. */
    const unsigned char *representation = (const unsigned char *)b;
    if (API_MUTANT != 1)
        for (size_t i = 0; i < n; ++i)
            for (size_t j = 0; j < sizeof *b; ++j)
                if (s + i == representation + j) return API_OVERLAP;
    for (size_t i = 0; i < n; ++i) b->data[b->length + i] = s[i];
    b->length += n;
    return API_OK;
}
void api_registry_init(ApiRegistry *r) {
    if (!r) return;
    r->dispatching = r->releasing = r->closing = r->closed = 0;
    for (unsigned i = 0; i < 4; ++i) {
        r->slots[i].callback = 0; r->slots[i].drop = 0; r->slots[i].context = 0;
        r->slots[i].generation = 0; r->slots[i].active = r->slots[i].removing = 0;
    }
}
static void release_slot(ApiRegistry *r, unsigned i) {
    ApiDrop drop = r->slots[i].drop;
    void *context = r->slots[i].context;
    r->slots[i].active = r->slots[i].removing = 0;
    r->slots[i].callback = 0; r->slots[i].drop = 0; r->slots[i].context = 0;
    r->releasing = 1;
    if (drop) drop(context);
    r->releasing = 0;
}
ApiStatus api_add(ApiRegistry *r, ApiCallback callback, void *context, ApiDrop drop, ApiHandle *out) {
    if (!r || !callback || !out) return API_INVALID;
    if (r->releasing) return API_BUSY;
    if (r->closing || r->closed) return API_CLOSED;
    for (unsigned i = 0; i < 4; ++i) {
        ApiSlot *s = r->slots + i;
        if (s->active || s->generation == UINT32_MAX) continue;
        s->generation++;
        s->callback = callback; s->context = context; s->drop = drop;
        s->active = 1; s->removing = 0;
        out->index = i; out->generation = s->generation;
        return API_OK;
    }
    return API_FULL;
}
ApiStatus api_remove(ApiRegistry *r, ApiHandle h) {
    if (!r || h.index >= 4 || !h.generation) return API_INVALID;
    if (r->releasing) return API_BUSY;
    if (r->closed) return API_CLOSED;
    ApiSlot *s = r->slots + h.index;
    if (!s->active || s->generation != h.generation) return API_STALE;
    if (r->dispatching && API_MUTANT != 3) { s->removing = 1; return API_PENDING; }
    release_slot(r, h.index);
    return API_OK;
}
ApiStatus api_destroy(ApiRegistry *r) {
    if (!r) return API_INVALID;
    if (r->releasing) return API_BUSY;
    if (r->closed) return API_OK;
    r->closing = 1;
    if (r->dispatching) return API_PENDING;
    for (unsigned i = 0; i < 4; ++i)
        if (r->slots[i].active) release_slot(r, i);
    r->closed = 1; r->closing = 0;
    return API_OK;
}
ApiStatus api_dispatch(ApiRegistry *r) {
    ApiHandle snapshot[4]; unsigned count = 0;
    if (!r) return API_INVALID;
    if (r->dispatching || r->releasing) return API_BUSY;
    if (r->closed || r->closing) return API_CLOSED;
    for (unsigned i = 0; i < 4; ++i)
        if (r->slots[i].active) {
            snapshot[count].index = i;
            snapshot[count++].generation = r->slots[i].generation;
        }
    r->dispatching = 1;
    for (unsigned k = 0; k < count && !r->closing; ++k) {
        ApiHandle h = snapshot[k];
        ApiSlot *s = r->slots + h.index;
        if (s->active && s->generation == h.generation)
            s->callback(s->context, r, h);
    }
    r->dispatching = 0;
    unsigned closing = r->closing;
    for (unsigned i = 0; i < 4; ++i)
        if (r->slots[i].active && (closing || r->slots[i].removing)) {
            if (API_MUTANT == 4 && closing && i == 1) continue;
            release_slot(r, i);
        }
    if (closing) { r->closed = 1; r->closing = 0; return API_CLOSED; }
    return API_OK;
}
