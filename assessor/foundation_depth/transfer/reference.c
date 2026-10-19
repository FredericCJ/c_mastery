#include "transfer.h"

FDStatus fd_owner_subview(const FDOwnerView *source,size_t offset,size_t count,FDOwnerView *out) {
    if (!source || !out || source->begin>source->end || source->end>source->owner_length ||
        source->position>source->end-source->begin || (!source->owner && source->owner_length)) return FD_INVALID;
    size_t remaining=source->end-source->begin-source->position;
    if (offset>remaining || count>remaining-offset) return FD_RANGE;
    FDOwnerView next=*source;
    next.begin+=next.position+offset;next.end=next.begin+count;next.position=0;
    *out=next;return FD_OK;
}
FDStatus fd_read_tagged(FDCursor *source,uint16_t *out) {
    if (!source || !out) return FD_INVALID;
    FDCursor trial=*source;const unsigned char *part;
    FDStatus status=fd_take(&trial,2,&part);
    if (status!=FD_OK) return status;
    if (part[0]!=0xa5 || part[1]!=2) return FD_INVALID;
#ifdef FD_TRANSFER_TAG_COMMIT
    *source=trial;
#endif
    status=fd_take(&trial,2,&part);
    if (status!=FD_OK) return status;
    uint16_t value=(uint16_t)((uint16_t)part[0] | (uint16_t)((uint16_t)part[1]<<8));
    *source=trial;*out=value;return FD_OK;
}
FDStatus fd_least_encode(FDLeastRecord record,unsigned char *out,size_t capacity) {
    return fd_encode_v2(record.count,record.value,out,capacity);
}
FDStatus fd_least_decode(const unsigned char *wire,size_t length,FDLeastRecord *out) {
    if (!out) return FD_INVALID;
    uintmax_t count;intmax_t value;
    FDStatus status=fd_decode_v2(wire,length,&count,&value);
    if (status!=FD_OK) return status;
    /* Decoded ranges are a subset of each least-width destination range. */
    *out=(FDLeastRecord){(uint_least16_t)count,(int_least32_t)value};return FD_OK;
}
FDStatus fd_inplace_xor(unsigned char *base,size_t length,unsigned char mask) {
    if (!base && length) return FD_INVALID;
    for (size_t i=0;i<length;++i) base[i]=(unsigned char)(base[i]^mask);
    return FD_OK;
}
FDStatus fd_pool_borrow(const FDPool *pool,FDHandle handle,const FDValue **out) {
    if (!out) return FD_INVALID;
    FDValue copy;
    FDStatus status=fd_pool_get(pool,handle,&copy);
    if (status!=FD_OK) return status;
    *out=&pool->entries[handle.index].value;return FD_OK;
}
FDStatus fd_registry_relocate(FDRegistry *registry,size_t slot,FDPool *destination) {
    if (!registry || slot>=2 || !destination) return FD_INVALID;
    FDRegistryEntry *entry=&registry->entries[slot];
    if (!entry->owner || entry->retired) return FD_STATE;
    for (size_t i=0;i<2;++i) if (registry->entries[i].owner==destination) return FD_STATE;
    FDPool *old=entry->owner;
    *destination=*old;
#ifndef FD_TRANSFER_OWNER_DUPLICATE
    for (size_t i=0;i<FD_POOL_CAPACITY;++i) { old->entries[i].live=0;old->entries[i].retired=1; }
#endif
    entry->owner=destination;
    return FD_OK;
}

FDStatus fd_effect(FDEffect effect,const unsigned char *wire,size_t length,
                    int authority,int32_t *counter,int32_t *observed) {
    if (!counter || !observed || (effect!=FD_QUERY && effect!=FD_UPDATE)) return FD_INVALID;
    FDRecord record;
    FDStatus status=fd_decode(wire,length,&record);
    if (status!=FD_OK) return status;
#ifdef FD_TRANSFER_QUERY_AUTH
    if (!authority) return FD_DENIED;
#endif
    if (effect==FD_UPDATE) {
        status=fd_apply(wire,length,authority,counter);
        if (status!=FD_OK) return status;
    }
    *observed=*counter;
    return FD_OK;
}

FDStep fd_step_value(int value) {
    FDStep result = {FD_RANGE, value, value};
    if (value != INT_MAX) {
        result.status = FD_OK;
        result.new_value = value + 1;
    }
    return result;
}
FDStatus fd_layout(size_t h, intmax_t ac, size_t aw, intmax_t bc, size_t bw,
                   size_t limit, FDLayout *out) {
    if (!out) return FD_INVALID;
    size_t a, b, second, end;
    if (fd_from_signed(ac, &a) != FD_OK || fd_from_signed(bc, &b) != FD_OK)
        return FD_RANGE;
    if (fd_extent(h, a, aw, limit, &second) != FD_OK) return FD_RANGE;
#ifdef FD_TRANSFER_LAYOUT
    if (fd_extent(h, b, bw, limit, &end) != FD_OK) return FD_RANGE;
#else
    if (fd_extent(second, b, bw, limit, &end) != FD_OK) return FD_RANGE;
#endif
    *out = (FDLayout){h, second, end};
    return FD_OK;
}
FDStatus fd_copy_segments(FDSegmented *c, size_t n, unsigned char *out, size_t cap) {
    if (!c || (!out && n)) return FD_INVALID;
    size_t total;
    for (unsigned i=0; i<2; ++i)
        if (!c->segments[i].base && c->segments[i].length) return FD_INVALID;
    if (fd_extent(c->segments[0].length, 1, c->segments[1].length,
                  SIZE_MAX, &total) != FD_OK || c->position > total) return FD_INVALID;
    if (n > total-c->position || n > cap) return FD_RANGE;
    size_t first = c->segments[0].length;
    for (size_t i=0; i<n; ++i) {
        size_t index = c->position+i;
        out[i] = index < first ? c->segments[0].base[index]
                              : c->segments[1].base[index-first];
    }
    c->position += n;
    return FD_OK;
}
FDStatus fd_encode_v2(uintmax_t count, intmax_t value, unsigned char *out, size_t cap) {
    if (!out) return FD_INVALID;
    if (cap != 8 || count > UINT16_MAX || value < INT32_MIN || value > INT32_MAX)
        return FD_RANGE;
    uint32_t u = (uint32_t)value;
    out[0]=2; out[1]=0; out[2]=(unsigned char)count; out[3]=(unsigned char)(count>>8);
    for (unsigned i=0; i<4; ++i) out[4+i]=(unsigned char)(u>>(8*i));
    return FD_OK;
}
FDStatus fd_decode_v2(const unsigned char *wire, size_t n, uintmax_t *count, intmax_t *value) {
    if (!wire || !count || !value) return FD_INVALID;
    if (n != 8) return FD_RANGE;
    if (wire[0] != 2 || wire[1] != 0) return FD_INVALID;
    uint32_t u=0;
    for (unsigned i=0; i<4; ++i) u |= (uint32_t)wire[4+i] << (8*i);
    *count=(uintmax_t)wire[2]+((uintmax_t)wire[3]<<8);
    *value=u<=INT32_MAX ? (intmax_t)u : -1-(intmax_t)(UINT32_MAX-u);
    return FD_OK;
}
void *fd_typed_allocate(void *context, size_t n, size_t alignment) {
    FDTypedStorage *s=context;
    if (!s || s->reserved || n!=sizeof s->value || alignment!=_Alignof(FDValue)) return 0;
    s->reserved=1;
    return &s->value;
}
void fd_typed_release(void *context, void *value) {
    FDTypedStorage *s=context;
    /* Only the exact outstanding element is admitted by this adapter. */
    if (s && value==&s->value) s->reserved=0;
}
FDStatus fd_slot_transfer(FDSlot *destination, FDSlot *source) {
    if (!destination || !source || destination==source) return FD_INVALID;
    if (destination->state!=FD_EMPTY || source->state==FD_EMPTY) return FD_STATE;
    *destination=*source;
    *source=(FDSlot)FD_SLOT_INITIALIZER;
    return FD_OK;
}
FDStatus fd_stride_move(unsigned char *base, size_t physical, size_t logical,
                        size_t stride, size_t dst, size_t src, size_t n) {
    size_t extent=0;
    if ((!base && physical) || !stride) return FD_INVALID;
    if (logical && (fd_extent(1, logical-1, stride, physical, &extent)!=FD_OK)) return FD_RANGE;
    if (dst>logical || src>logical || n>logical-dst || n>logical-src) return FD_RANGE;
    if (dst>src) for (size_t i=n; i>0; --i) base[(dst+i-1)*stride]=base[(src+i-1)*stride];
    else for (size_t i=0; i<n; ++i) base[(dst+i)*stride]=base[(src+i)*stride];
    return FD_OK;
}
FDStatus fd_compact(FDStreamView *s) {
    if (!s || s->position>s->length || (!s->base && s->length)) return FD_INVALID;
    if (s->epoch==SIZE_MAX) return FD_FULL;
    size_t length=s->length-s->position;
    FDStatus status=fd_move(s->base,s->length,0,s->position,length);
    if (status!=FD_OK) return status;
    s->length=length; s->position=0; ++s->epoch;
    return FD_OK;
}
void fd_registry_init(FDRegistry *r) {
    for (unsigned i=0;i<2;++i) r->entries[i]=(FDRegistryEntry){0,1,0};
}
FDStatus fd_registry_bind(FDRegistry *r, size_t slot, FDPool *p) {
    if (!r || !p || slot>=2) return FD_INVALID;
    FDRegistryEntry *e=&r->entries[slot];
    if (e->retired) return FD_FULL;
    if (e->owner) return FD_STATE;
    /* One current registry binding per owner. */
    for (unsigned i=0;i<2;++i) if (r->entries[i].owner==p) return FD_STATE;
    e->owner=p;
    return FD_OK;
}
FDStatus fd_registry_unbind(FDRegistry *r, size_t slot) {
    if (!r || slot>=2) return FD_INVALID;
    FDRegistryEntry *e=&r->entries[slot];
    if (!e->owner) return FD_STATE;
    e->owner=0;
#ifdef FD_TRANSFER_EPOCH
    e->epoch=e->epoch==3 ? 1 : e->epoch+1;
#else
    if (e->epoch==3) e->retired=1; else ++e->epoch;
#endif
    return FD_OK;
}
FDStatus fd_registry_acquire(FDRegistry *r, size_t slot, FDValue value, FDRegistryHandle *out) {
    if (!r || !out || slot>=2) return FD_INVALID;
    FDRegistryEntry *e=&r->entries[slot];
    if (!e->owner) return FD_STATE;
    FDHandle local;
    FDStatus status=fd_pool_acquire(e->owner,value,&local);
    if (status!=FD_OK) return status;
    *out=(FDRegistryHandle){slot,local.index,e->epoch,local.generation};
    return FD_OK;
}
FDStatus fd_registry_get(const FDRegistry *r, FDRegistryHandle h, FDValue *out) {
    if (!r || !out) return FD_INVALID;
    if (h.registry_slot>=2) return FD_STALE;
    const FDRegistryEntry *e=&r->entries[h.registry_slot];
    if (!e->owner || e->retired || e->epoch!=h.epoch) return FD_STALE;
    return fd_pool_get(e->owner,(FDHandle){e->owner,h.local_slot,h.generation},out);
}
void fd_frame_cancel(FDFrame *f) {
    if (!f) return;
    f->used=0;
#ifndef FD_TRANSFER_AUTHORITY
    f->authority=0;
#endif
    for (unsigned i=0;i<8;++i) f->bytes[i]=0;
}
FDStatus fd_frame_feed(FDFrame *f, const unsigned char *bytes, size_t n,
                       int authority, int32_t *counter, int *committed) {
    if (!f || !counter || !committed || (!bytes && n) || f->used>8) return FD_INVALID;
    if (n>8-f->used) return FD_RANGE;
    *committed=0;
    if (!n) return FD_OK;
#ifdef FD_TRANSFER_AUTHORITY
    if (!f->used) f->authority = f->authority || authority;
#else
    if (!f->used) f->authority=authority!=0;
#endif
    for (size_t i=0;i<n;++i) f->bytes[f->used+i]=bytes[i];
    f->used+=n;
    if (f->used<8) return FD_OK;
    FDStatus status=fd_apply(f->bytes,8,f->authority,counter);
    fd_frame_cancel(f);
    if (status==FD_OK) *committed=1;
    return status;
}
