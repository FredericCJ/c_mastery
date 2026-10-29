#include "foundation.h"
#include <limits.h>
const char *f_status_name(enum f_status s) {
  static const char *const names[] = {"OK",      "INVALID",         "RANGE",
                                      "FULL",    "STALE",           "IO",
                                      "PENDING", "NOT_IMPLEMENTED", "DENIED"};
  return (unsigned)s < sizeof names / sizeof names[0] ? names[s] : "UNKNOWN";
}
enum f_status f_construct(struct f_resources resources, unsigned *owned_mask) {
  unsigned acquired = 0;
  if (!resources.acquire || !resources.release || !owned_mask)
    return F_INVALID;
  for (unsigned i = 0; i < 3; i++) {
    if (resources.acquire(resources.context, i)) {
      while (acquired)
        resources.release(resources.context, --acquired);
      return F_IO;
    }
    acquired++;
  }
  *owned_mask = 7;
  return F_OK;
}
enum f_status f_packet(const uint8_t *bytes, size_t n, int authorized,
                 uint16_t *output, size_t *count) {
  uint16_t staged[4];
  unsigned size;
  if (!bytes || n < 2 || !count || !output)
    return F_INVALID;
  if (bytes[0] != 1 && bytes[0] != 2)
    return F_INVALID;
  size = bytes[1];
  if (size > 4)
    return F_RANGE;
  if (n != 2u + 2u * size)
    return F_INVALID;
  if (bytes[0] == 2 && !authorized)
    return F_DENIED;
  for (unsigned i = 0; i < size; i++)
    staged[i] = (uint16_t)((uint16_t)bytes[2 + 2 * i] << 8) | bytes[3 + 2 * i];
  for (unsigned i = 0; i < size; i++)
    output[i] = staged[i];
  *count = size;
  return F_OK;
}
enum f_status f_prefix(const int32_t *input, size_t n, int32_t *output,
                 size_t capacity) {
  int32_t stage[16], sum = 0;
  if (n > 16 || n > capacity)
    return F_FULL;
  if (n && (!input || !output))
    return F_INVALID;
  for (size_t i = 0; i < n; i++) {
    int32_t x = input[i];
    if ((x > 0 && sum > INT32_MAX - x) || (x < 0 && sum < INT32_MIN - x))
      return F_RANGE;
    sum += x;
    stage[i] = sum;
  }
  for (size_t i = 0; i < n; i++)
    output[i] = stage[i];
  return F_OK;
}
enum f_status f_checked_size(size_t n, size_t width, size_t extra, size_t *result) {
  if (!result)
    return F_INVALID;
#ifndef F_MUTATE_SIZE
  if (width && n > (SIZE_MAX - extra) / width)
    return F_RANGE;
#endif
  *result = n * width + extra;
  return F_OK;
}
enum f_status f_take(struct f_cursor *c, size_t n, const uint8_t **out) {
  if (!c || !out || c->position > c->length || (c->length && !c->data))
    return F_INVALID;
  if (n > c->length - c->position)
    return F_RANGE;
  *out = c->data ? c->data + c->position : 0;
  c->position += n;
  return F_OK;
}
enum f_status f_append(struct f_buffer *b, const uint8_t *source, size_t n) {
  uint8_t copy[32];
  if (!b || b->length > 32 || (n && !source))
    return F_INVALID;
  if (n > 32 - b->length)
    return F_FULL;
  if (!n)
    return F_OK;
  if (b->generation == UINT32_MAX)
    return F_RANGE;
  for (size_t i = 0; i < n; i++)
    copy[i] = source[i];
  for (size_t i = 0; i < n; i++)
    b->data[b->length + i] = copy[i];
  b->length += n;
  b->generation++;
  return F_OK;
}
enum f_status f_view(const struct f_buffer *b, size_t offset, size_t n, struct f_view *out) {
  if (!b || !out || b->length > 32)
    return F_INVALID;
  if (offset > b->length || n > b->length - offset)
    return F_RANGE;
  out->owner = b;
  out->offset = offset;
  out->length = n;
  out->generation = b->generation;
  return F_OK;
}
enum f_status f_view_read(struct f_view view, size_t i, uint8_t *out) {
  if (!view.owner || !out)
    return F_INVALID;
  if (view.generation != view.owner->generation)
    return F_STALE;
  if (view.owner->length > 32 || view.offset > view.owner->length ||
      view.length > view.owner->length - view.offset)
    return F_INVALID;
  if (i >= view.length)
    return F_RANGE;
  *out = view.owner->data[view.offset + i];
  return F_OK;
}
enum f_status f_parse_i32(const uint8_t *b, size_t n, int32_t *out) {
  size_t i = 0;
  uint32_t value = 0, limit = INT32_MAX;
  int negative = 0;
  if (!b || !out || !n)
    return F_INVALID;
  if (b[0] == '-') {
    negative = 1;
    limit = UINT32_C(2147483648);
    i++;
  }
  if (i == n)
    return F_INVALID;
  for (; i < n; i++) {
    uint32_t digit;
    if (b[i] < '0' || b[i] > '9')
      return F_INVALID;
    digit = (uint32_t)(b[i] - '0');
    if (value > (limit - digit) / 10u)
      return F_RANGE;
    value = value * 10u + digit;
  }
  *out = negative ? (int32_t)(-(int64_t)value) : (int32_t)value;
  return F_OK;
}
enum f_status f_sort(struct f_record *a, size_t n, struct f_work *work) {
  struct f_work w = {0};
  if (!work || (n && !a) || n > 256)
    return F_INVALID;
  for (size_t i = 1; i < n; i++) {
    struct f_record item = a[i];
    size_t j = i;
    while (j) {
      w.comparisons++;
#ifdef F_MUTATE_STABILITY
      if (a[j - 1].key < item.key)
        break;
#else
      if (a[j - 1].key <= item.key)
        break;
#endif
      a[j] = a[j - 1];
      j--;
      w.shifts++;
    }
    a[j] = item;
    w.placements++;
  }
  *work = w;
  return F_OK;
}
enum f_status f_write_all(f_writer_fn writer, void *ctx, const uint8_t *source, size_t n,
                    unsigned calls, size_t *accepted) {
  size_t done = 0;
  if (!writer || !accepted || (n && !source))
    return F_INVALID;
  *accepted = 0;
  while (done < n) {
    ptrdiff_t step;
    if (!calls)
      return F_PENDING;
    calls--;
    step = writer(ctx, source + done, n - done);
    if (step == -2)
      continue;
    if (step <= 0 || (size_t)step > n - done)
      return F_IO;
    done += (size_t)step;
    *accepted = done;
  }
  return F_OK;
}
enum f_status f_submit(struct f_async *a, uint32_t *ticket) {
  if (!a || !ticket)
    return F_INVALID;
  if (a->state == F_INFLIGHT || a->state == F_CANCEL_REQUESTED)
    return F_PENDING;
  if (a->generation == UINT32_MAX)
    return F_RANGE;
  a->generation++;
  a->state = F_INFLIGHT;
  *ticket = a->generation;
  return F_OK;
}
enum f_status f_cancel(struct f_async *a, uint32_t ticket) {
  if (!a)
    return F_INVALID;
  if (ticket != a->generation)
    return F_STALE;
  if (a->state != F_INFLIGHT)
    return F_INVALID;
  a->state = F_CANCEL_REQUESTED;
  return F_PENDING;
}
enum f_status f_complete(struct f_async *a, uint32_t ticket) {
  if (!a)
    return F_INVALID;
  if (ticket != a->generation)
    return F_STALE;
  if (a->state != F_INFLIGHT && a->state != F_CANCEL_REQUESTED)
    return F_INVALID;
  if (a->releases == UINT32_MAX)
    return F_RANGE;
  a->state = F_DONE;
  a->releases++;
  return F_OK;
}
