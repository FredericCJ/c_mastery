#ifndef FD_TRANSFER_H
#define FD_TRANSFER_H
#include "depth.h"
/* Withheld changed-condition interfaces. Release only with the transfer brief. */
struct fd_step { enum fd_status status; int old_value, new_value; };
struct fd_step fd_step_value(int value);
struct fd_layout { size_t first, second, end; };
enum fd_status fd_layout(size_t header, intmax_t first_count, size_t first_width,
                   intmax_t second_count, size_t second_width, size_t limit,
                   struct fd_layout *out);
struct fd_segment { const unsigned char *base; size_t length; };
struct fd_segmented { struct fd_segment segments[2]; size_t position; };
struct fd_owner_view { const unsigned char *owner; size_t owner_length, begin, end, position; };
enum fd_status fd_owner_subview(const struct fd_owner_view *source,size_t offset,size_t count,struct fd_owner_view *out);
enum fd_status fd_read_tagged(struct fd_cursor *source,uint16_t *out);
enum fd_status fd_copy_segments(struct fd_segmented *cursor, size_t count,
                          unsigned char *destination, size_t capacity);
enum fd_status fd_encode_v2(uintmax_t count, intmax_t value, unsigned char *out,
                      size_t capacity);
enum fd_status fd_decode_v2(const unsigned char *wire, size_t length,
                      uintmax_t *count, intmax_t *value);
struct fd_least_record { uint_least16_t count; int_least32_t value; };
enum fd_status fd_least_encode(struct fd_least_record record,unsigned char *out,size_t capacity);
enum fd_status fd_least_decode(const unsigned char *wire,size_t length,struct fd_least_record *out);
/* The array adapter owns an actual declared FDValue; never treats bytes as it. */
struct fd_typed_storage { struct fd_value value; unsigned reserved; };
void *fd_typed_allocate(void *context, size_t size, size_t alignment);
void fd_typed_release(void *context, void *value);
enum fd_status fd_slot_transfer(struct fd_slot *destination, struct fd_slot *source);
enum fd_status fd_stride_move(unsigned char *base, size_t physical_length,
                        size_t logical_length, size_t stride,
                        size_t destination, size_t source, size_t count);
struct fd_stream_view { unsigned char *base; size_t length, position, epoch; };
enum fd_status fd_compact(struct fd_stream_view *stream);
enum fd_status fd_inplace_xor(unsigned char *base,size_t length,unsigned char mask);
enum fd_status fd_pool_borrow(const struct fd_pool *pool,struct fd_handle handle,const struct fd_value **out);
/* Registry itself has stable lifetime. Mappings may move between live owners. */
struct fd_registry_entry { struct fd_pool *owner; unsigned epoch, retired; };
struct fd_registry { struct fd_registry_entry entries[2]; };
struct fd_registry_handle { size_t registry_slot, local_slot; unsigned epoch, generation; };
void fd_registry_init(struct fd_registry *registry);
enum fd_status fd_registry_bind(struct fd_registry *registry, size_t slot, struct fd_pool *pool);
enum fd_status fd_registry_unbind(struct fd_registry *registry, size_t slot);
enum fd_status fd_registry_relocate(struct fd_registry *registry,size_t slot,struct fd_pool *destination);
enum fd_status fd_registry_acquire(struct fd_registry *registry, size_t slot, struct fd_value value,
                             struct fd_registry_handle *out);
enum fd_status fd_registry_get(const struct fd_registry *registry, struct fd_registry_handle handle,
                         struct fd_value *out);
struct fd_frame { unsigned char bytes[8]; size_t used; int authority; };
enum fd_effect { FD_QUERY, FD_UPDATE };
enum fd_status fd_effect(enum fd_effect effect, const unsigned char *wire, size_t length,
                    int authority, int32_t *counter, int32_t *observed);
void fd_frame_cancel(struct fd_frame *frame);
enum fd_status fd_frame_feed(struct fd_frame *frame, const unsigned char *bytes, size_t count,
                       int authority, int32_t *counter, int *committed);
#endif
