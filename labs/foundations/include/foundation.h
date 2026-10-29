#ifndef CMASTERY_FOUNDATION_H
#define CMASTERY_FOUNDATION_H
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
enum f_status {
  F_OK,
  F_INVALID,
  F_RANGE,
  F_FULL,
  F_STALE,
  F_IO,
  F_PENDING,
  F_NOT_IMPLEMENTED,
  F_DENIED
};
struct f_cursor {
  const uint8_t *data;
  size_t length, position;
};
struct f_buffer {
  uint8_t data[32];
  size_t length;
  uint32_t generation;
};
struct f_view {
  const struct f_buffer *owner;
  size_t offset, length;
  uint32_t generation;
};
struct f_record {
  int32_t key;
  uint32_t ordinal;
};
struct f_work {
  uint64_t comparisons, shifts, placements;
};
enum f_async_state { F_IDLE, F_INFLIGHT, F_CANCEL_REQUESTED, F_DONE };
struct f_async {
  enum f_async_state state;
  uint32_t generation, releases;
};
/* Writer: positive count<=requested, 0=no progress, -1=terminal, -2=retryable.
   Retry budget counts all callback invocations; accepted prefix is never
   replayed. */
typedef ptrdiff_t (*f_writer_fn)(void *, const uint8_t *, size_t);
struct f_resources {
  void *context;
  int (*acquire)(void *, unsigned);
  void (*release)(void *, unsigned);
};
enum f_status f_construct(struct f_resources resources, unsigned *owned_mask);
enum f_status f_packet(const uint8_t *bytes, size_t length, int authorized,
                 uint16_t *output, size_t *count);
enum f_status f_prefix(const int32_t *source, size_t count, int32_t *output,
                 size_t capacity);
enum f_status f_checked_size(size_t count, size_t width, size_t extra,
                       size_t *result);
enum f_status f_take(struct f_cursor *cursor, size_t count, const uint8_t **bytes);
enum f_status f_append(struct f_buffer *buffer, const uint8_t *source, size_t count);
enum f_status f_view(const struct f_buffer *buffer, size_t offset, size_t count, struct f_view *view);
enum f_status f_view_read(struct f_view view, size_t index, uint8_t *value);
enum f_status f_parse_i32(const uint8_t *bytes, size_t count, int32_t *result);
enum f_status f_sort(struct f_record *records, size_t count, struct f_work *work);
enum f_status f_write_all(f_writer_fn writer, void *context, const uint8_t *source,
                    size_t length, unsigned calls, size_t *accepted);
enum f_status f_submit(struct f_async *operation, uint32_t *ticket);
enum f_status f_cancel(struct f_async *operation, uint32_t ticket);
enum f_status f_complete(struct f_async *operation, uint32_t ticket);
const char *f_status_name(enum f_status status);
#ifdef __cplusplus
}
#endif
#endif
