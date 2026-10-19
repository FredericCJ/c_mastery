#ifndef FD_TRANSFER_H
#define FD_TRANSFER_H
#include "depth.h"
/* Withheld changed-condition interfaces. Release only with the transfer brief. */
typedef struct { FDStatus status; int old_value, new_value; } FDStep;
FDStep fd_step_value(int value);
typedef struct { size_t first, second, end; } FDLayout;
FDStatus fd_layout(size_t header, intmax_t first_count, size_t first_width,
                   intmax_t second_count, size_t second_width, size_t limit,
                   FDLayout *out);
typedef struct { const unsigned char *base; size_t length; } FDSegment;
typedef struct { FDSegment segments[2]; size_t position; } FDSegmented;
typedef struct { const unsigned char *owner; size_t owner_length, begin, end, position; } FDOwnerView;
FDStatus fd_owner_subview(const FDOwnerView *source,size_t offset,size_t count,FDOwnerView *out);
FDStatus fd_read_tagged(FDCursor *source,uint16_t *out);
FDStatus fd_copy_segments(FDSegmented *cursor, size_t count,
                          unsigned char *destination, size_t capacity);
FDStatus fd_encode_v2(uintmax_t count, intmax_t value, unsigned char *out,
                      size_t capacity);
FDStatus fd_decode_v2(const unsigned char *wire, size_t length,
                      uintmax_t *count, intmax_t *value);
typedef struct { uint_least16_t count; int_least32_t value; } FDLeastRecord;
FDStatus fd_least_encode(FDLeastRecord record,unsigned char *out,size_t capacity);
FDStatus fd_least_decode(const unsigned char *wire,size_t length,FDLeastRecord *out);
/* The array adapter owns an actual declared FDValue; never treats bytes as it. */
typedef struct { FDValue value; unsigned reserved; } FDTypedStorage;
void *fd_typed_allocate(void *context, size_t size, size_t alignment);
void fd_typed_release(void *context, void *value);
FDStatus fd_slot_transfer(FDSlot *destination, FDSlot *source);
FDStatus fd_stride_move(unsigned char *base, size_t physical_length,
                        size_t logical_length, size_t stride,
                        size_t destination, size_t source, size_t count);
typedef struct { unsigned char *base; size_t length, position, epoch; } FDStreamView;
FDStatus fd_compact(FDStreamView *stream);
FDStatus fd_inplace_xor(unsigned char *base,size_t length,unsigned char mask);
FDStatus fd_pool_borrow(const FDPool *pool,FDHandle handle,const FDValue **out);
/* Registry itself has stable lifetime. Mappings may move between live owners. */
typedef struct { FDPool *owner; unsigned epoch, retired; } FDRegistryEntry;
typedef struct { FDRegistryEntry entries[2]; } FDRegistry;
typedef struct { size_t registry_slot, local_slot; unsigned epoch, generation; } FDRegistryHandle;
void fd_registry_init(FDRegistry *registry);
FDStatus fd_registry_bind(FDRegistry *registry, size_t slot, FDPool *pool);
FDStatus fd_registry_unbind(FDRegistry *registry, size_t slot);
FDStatus fd_registry_relocate(FDRegistry *registry,size_t slot,FDPool *destination);
FDStatus fd_registry_acquire(FDRegistry *registry, size_t slot, FDValue value,
                             FDRegistryHandle *out);
FDStatus fd_registry_get(const FDRegistry *registry, FDRegistryHandle handle,
                         FDValue *out);
typedef struct { unsigned char bytes[8]; size_t used; int authority; } FDFrame;
typedef enum { FD_QUERY, FD_UPDATE } FDEffect;
FDStatus fd_effect(FDEffect effect, const unsigned char *wire, size_t length,
                    int authority, int32_t *counter, int32_t *observed);
void fd_frame_cancel(FDFrame *frame);
FDStatus fd_frame_feed(FDFrame *frame, const unsigned char *bytes, size_t count,
                       int authority, int32_t *counter, int *committed);
#endif
