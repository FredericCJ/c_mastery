#ifndef CMASTERY_FOUNDATION_H
#define CMASTERY_FOUNDATION_H
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef enum {
  F_OK,
  F_INVALID,
  F_RANGE,
  F_FULL,
  F_STALE,
  F_IO,
  F_PENDING,
  F_NOT_IMPLEMENTED,
  F_DENIED
} FStatus;
typedef struct {
  const uint8_t *data;
  size_t length, position;
} FCursor;
typedef struct {
  uint8_t data[32];
  size_t length;
  uint32_t generation;
} FBuffer;
typedef struct {
  const FBuffer *owner;
  size_t offset, length;
  uint32_t generation;
} FView;
typedef struct {
  int32_t key;
  uint32_t ordinal;
} FRecord;
typedef struct {
  uint64_t comparisons, shifts, placements;
} FWork;
typedef enum { F_IDLE, F_INFLIGHT, F_CANCEL_REQUESTED, F_DONE } FAsyncState;
typedef struct {
  FAsyncState state;
  uint32_t generation, releases;
} FAsync;
/* Writer: positive count<=requested, 0=no progress, -1=terminal, -2=retryable.
   Retry budget counts all callback invocations; accepted prefix is never
   replayed. */
typedef ptrdiff_t (*FWriter)(void *, const uint8_t *, size_t);
typedef struct {
  void *context;
  int (*acquire)(void *, unsigned);
  void (*release)(void *, unsigned);
} FResources;
FStatus f_construct(FResources resources, unsigned *owned_mask);
FStatus f_packet(const uint8_t *bytes, size_t length, int authorized,
                 uint16_t *output, size_t *count);
FStatus f_prefix(const int32_t *source, size_t count, int32_t *output,
                 size_t capacity);
FStatus f_checked_size(size_t count, size_t width, size_t extra,
                       size_t *result);
FStatus f_take(FCursor *cursor, size_t count, const uint8_t **bytes);
FStatus f_append(FBuffer *buffer, const uint8_t *source, size_t count);
FStatus f_view(const FBuffer *buffer, size_t offset, size_t count, FView *view);
FStatus f_view_read(FView view, size_t index, uint8_t *value);
FStatus f_parse_i32(const uint8_t *bytes, size_t count, int32_t *result);
FStatus f_sort(FRecord *records, size_t count, FWork *work);
FStatus f_write_all(FWriter writer, void *context, const uint8_t *source,
                    size_t length, unsigned calls, size_t *accepted);
FStatus f_submit(FAsync *operation, uint32_t *ticket);
FStatus f_cancel(FAsync *operation, uint32_t ticket);
FStatus f_complete(FAsync *operation, uint32_t ticket);
const char *f_status_name(FStatus status);
#ifdef __cplusplus
}
#endif
#endif
