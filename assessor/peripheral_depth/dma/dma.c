#include "dma.h"
#ifndef PD_MUTANT
#define PD_MUTANT 0
#endif
static int valid_io(struct pd_io io) {
  return io.configure && io.enable && io.stop && io.snapshot && io.visibility;
}
static int extent(uint32_t a, uint32_t n, unsigned board) {
  uint32_t low = board == PD_F439 ? 0x20000000u : 0x30040040u;
  uint32_t high = board == PD_F439 ? 0x20030000u : 0x30048000u;
  return n && a >= low && a < high && n <= high - a;
}
static int overlaps(uint32_t a, uint32_t n, uint32_t b, uint32_t m) {
  return a < b ? b - a < n : a - b < m;
}
int pd_validate(unsigned board, unsigned cached, const struct pd_extent *v, unsigned n,
                uint32_t *out) {
  if (!v || !out || !n || n > 4 || board > PD_H745 || cached > 1 ||
      (board == PD_F439 && cached))
    return PD_INVALID;
  uint32_t total = 0;
  for (unsigned i = 0; i < n; i++) {
    const struct pd_extent *e = &v[i];
    if ((e->width != 1 && e->width != 2 && e->width != 4) || !e->bytes ||
        e->bytes > 4096 || e->bytes % e->width || e->source % e->width ||
        e->destination % e->width || e->bytes > e->source_capacity ||
        e->bytes > e->destination_capacity)
      return PD_INVALID;
    if (PD_MUTANT != 1 &&
        (!extent(e->source, e->source_capacity, board) ||
         !extent(e->destination, e->destination_capacity, board)))
      return PD_INVALID;
    if (cached && PD_MUTANT != 2 &&
        ((e->source | e->destination | e->bytes | e->source_capacity |
          e->destination_capacity) &
         31u))
      return PD_INVALID;
    for (unsigned j = 0; j < n; j++) {
      if (overlaps(e->destination, e->bytes, v[j].source, v[j].bytes))
        return PD_INVALID;
      if (i != j &&
          overlaps(e->destination, e->bytes, v[j].destination, v[j].bytes))
        return PD_INVALID;
    }
    total += e->bytes;
  }
  *out = total;
  return PD_OK;
}
static void clean(struct pd_transfer *s, struct pd_io io) {
  if (s->cached)
    for (unsigned i = 0; i < s->count; i++) {
      struct pd_extent *e = &s->extents[i];
      io.visibility(io.context, PD_CLEAN, e->source, e->bytes);
      /* Exclusive destination lines: clean then invalidate prevents later
         eviction from overwriting DMA data and preserves bytes outside a
         partial result. */
      if (PD_MUTANT != 3)
        io.visibility(io.context, PD_CLEAN, e->destination, e->bytes);
      io.visibility(io.context, PD_INVALIDATE, e->destination, e->bytes);
    }
  io.visibility(io.context, PD_RELEASE, 0, 0);
}
int pd_submit(struct pd_transfer *s, struct pd_io io, unsigned board, unsigned cached,
              const struct pd_extent *v, unsigned n, uint32_t token) {
  uint32_t total;
  if (!s || !valid_io(io) || !token ||
      pd_validate(board, cached, v, n, &total) != PD_OK)
    return PD_INVALID;
  if (s->phase)
    return PD_BUSY;
  if (token <= s->last_token)
    return PD_STALE;
  /* Configure cannot publish and leaves the stream disabled on refusal. */
  if (!io.configure(io.context, &v[0], token))
    return PD_BUSY;
  struct pd_transfer next = {0};
  next.last_token = next.token = token;
  next.count = n;
  next.total = total;
  next.board = board;
  next.cached = cached;
  next.phase = 1;
  for (unsigned i = 0; i < n; i++)
    next.extents[i] = v[i];
  *s = next;
  clean(s, io);
  io.enable(io.context);
  return PD_OK;
}
int pd_cancel(struct pd_transfer *s, struct pd_io io, uint32_t token) {
  if (!s || !valid_io(io))
    return PD_INVALID;
  if (!s->phase || token != s->token)
    return PD_STALE;
  if (s->phase == 3)
    return PD_BUSY;
  if (s->phase == 1) {
    s->phase = 2;
    s->result = PD_CANCELLED;
    io.stop(io.context);
  }
  return PD_BUSY;
}
int pd_poll(struct pd_transfer *s, struct pd_io io, uint32_t token) {
  if (!s || !valid_io(io))
    return PD_INVALID;
  if (!s->phase || token != s->token)
    return PD_STALE;
  if (s->phase == 3)
    return PD_OK; /* Ready to finalize; not a second terminal result. */
  struct pd_snapshot x = io.snapshot(io.context);
  if (PD_MUTANT != 4 && x.token != token)
    return PD_BUSY;
  struct pd_extent *e = &s->extents[s->index];
  uint32_t items = e->bytes / e->width;
  if (x.remaining > items || x.enabled > 1 || x.fifo_empty > 1 ||
      x.flags & (PD_TE | PD_DME | PD_FE)) {
    s->result = PD_IO;
    s->phase = 2;
    io.stop(io.context);
  }
  /* EN=0 alone is insufficient: a FIFO flush can still publish to memory. */
  if (PD_MUTANT != 5 && (x.enabled || !x.fifo_empty || !(x.flags & PD_TC)))
    return PD_BUSY;
  io.visibility(io.context, PD_ACQUIRE, 0, 0);
  if (x.remaining > items) {
    s->result = PD_IO;
    s->phase = 2;
  }
  uint32_t partial =
      x.remaining <= items ? (items - x.remaining) * e->width : 0;
  if (s->result == PD_IO) {
    s->touched = s->done + e->bytes;
    s->phase = 3;
    return PD_OK;
  }
  if (s->phase == 2) {
    s->done += partial;
    s->touched = s->done;
    s->phase = 3;
    return PD_OK;
  }
  if (x.remaining) {
    s->result = PD_IO;
    s->touched = s->done + e->bytes;
    s->phase = 3;
    return PD_OK;
  }
  s->done += e->bytes;
  s->touched = s->done;
  if (++s->index == s->count) {
    s->phase = 3;
    return PD_OK;
  }
  if (!io.configure(io.context, &s->extents[s->index], s->token)) {
    s->result = PD_IO;
    s->phase = 3;
    return PD_OK;
  }
  io.visibility(io.context, PD_RELEASE, 0, 0);
  io.enable(io.context);
  return PD_BUSY;
}
int pd_finalize(struct pd_transfer *s, struct pd_io io, uint32_t token, struct pd_result *out) {
  if (!s || !valid_io(io) || !out)
    return PD_INVALID;
  if (!s->phase || token != s->token)
    return PD_STALE;
  if (s->phase != 3)
    return PD_BUSY;
  if (s->cached && PD_MUTANT != 6)
    for (unsigned i = 0; i < s->count; i++)
      io.visibility(io.context, PD_INVALIDATE, s->extents[i].destination,
                    s->extents[i].bytes);
  io.visibility(io.context, PD_ACQUIRE, 0, 0);
  struct pd_result r = {token, s->result, s->done, s->touched};
  *out = r;
  s->phase = 0;
  return PD_OK;
}
int pd_suffix(const struct pd_extent *v, unsigned n, uint32_t done, struct pd_extent *out,
              unsigned *count) {
  if (!v || !out || !count || !n || n > 4)
    return PD_INVALID;
  uint32_t total = 0;
  for (unsigned i = 0; i < n; i++) {
    if (!v[i].bytes || v[i].bytes > 4096 ||
        (v[i].width != 1 && v[i].width != 2 && v[i].width != 4) ||
        v[i].bytes % v[i].width || v[i].source % v[i].width ||
        v[i].destination % v[i].width || v[i].source_capacity < v[i].bytes ||
        v[i].destination_capacity < v[i].bytes ||
        v[i].source_capacity - 1 > UINT32_MAX - v[i].source ||
        v[i].destination_capacity - 1 > UINT32_MAX - v[i].destination)
      return PD_INVALID;
    total += v[i].bytes;
  }
  if (done > total)
    return PD_INVALID;
  struct pd_extent temp[4];
  unsigned used = 0;
  for (unsigned i = 0; i < n; i++) {
    if (done >= v[i].bytes) {
      done -= v[i].bytes;
      continue;
    }
    if (done % v[i].width || v[i].source > UINT32_MAX - done ||
        v[i].destination > UINT32_MAX - done || v[i].source_capacity < done ||
        v[i].destination_capacity < done)
      return PD_INVALID;
    temp[used] = v[i];
    temp[used].source += done;
    temp[used].destination += done;
    temp[used].bytes -= done;
    temp[used].source_capacity -= done;
    temp[used].destination_capacity -= done;
    used++;
    done = 0;
  }
  for (unsigned i = 0; i < used; i++)
    out[i] = temp[i];
  *count = used;
  return PD_OK;
}
