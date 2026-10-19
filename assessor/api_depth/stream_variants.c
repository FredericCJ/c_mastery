#include "variants.h"
#ifndef SV_MUTANT
#define SV_MUTANT 0
#endif

void sv_init(SVStream *stream) {
  if (stream) {
    sf_init(&stream->frame);
    stream->base = (SFHandle){0};
    stream->status = SF_STATE;
  }
}

static int valid(SVHandle handle) {
  return handle.owner && handle.generation &&
         handle.generation == handle.owner->frame.generation;
}

int sv_start(SVStream *stream, const unsigned char *bytes, size_t length,
             SVHandle *out) {
  if (!stream || !out || (!bytes && length))
    return SF_INVALID;
  if (length > 64 || stream->frame.generation == UINT32_MAX)
    return SF_RANGE;
  if (stream->status == SF_PENDING)
    return SF_STATE;
  /* A diagnostic may have terminated the wrapper after positive progress.
     Cancel only the old producer request, after new admission is validated. */
  if (stream->frame.state == 1) {
    int result = sf_cancel(stream->base);
    if (result != SF_OK)
      return result;
  }
  int result = sf_start(&stream->frame, bytes, length, &stream->base);
  if (result != SF_OK)
    return result;
  stream->status = SF_PENDING;
  *out = (SVHandle){stream, stream->frame.generation};
  return SF_OK;
}

typedef struct {
  SVWrite write;
  void *context;
  SVWriteResult result;
  int invalid;
} Bridge;
static ptrdiff_t bridge(void *context, const unsigned char *bytes,
                        size_t length) {
  Bridge *adapter = context;
  adapter->result = adapter->write(adapter->context, bytes, length);
  SVWriteResult result = adapter->result;
  if (result.accepted > length || result.disposition > SV_ERROR ||
      (result.disposition == SV_RETRY && result.accepted) ||
      (result.disposition == SV_PROGRESS && !result.accepted)) {
    adapter->invalid = 1;
    return (ptrdiff_t)length + 1;
  }
  if (SV_MUTANT == 1 && result.disposition == SV_ERROR)
    return -1;
  if (result.accepted)
    return (ptrdiff_t)result.accepted;
  if (result.disposition == SV_ERROR)
    return -1;
  return result.disposition == SV_RETRY ? -2 : 0;
}

int sv_pump(SVHandle handle, SVWrite write, void *context, unsigned budget,
            size_t *accepted) {
  if (!valid(handle) || !write || !accepted)
    return SF_INVALID;
  SVStream *stream = handle.owner;
  *accepted = stream->frame.offset;
  if (stream->status != SF_PENDING)
    return stream->status;
  while (budget--) {
    Bridge adapter = {write, context, {0, SV_WAIT}, 0};
    int result = sf_pump(stream->base, bridge, &adapter, 1, accepted);
    if (adapter.invalid)
      return stream->status = SF_INVALID;
    if (adapter.result.disposition == SV_ERROR &&
        !(SV_MUTANT == 2 && result == SF_DONE))
      return stream->status = SF_IO;
    if (result != SF_PENDING)
      return stream->status = result;
    if (adapter.result.disposition == SV_WAIT)
      return SF_PENDING;
  }
  return SF_PENDING;
}

typedef struct {
  SVHandle handle;
  SVWriteV write;
  void *context;
} VectorBridge;
static SVWriteResult vectors(void *context, const unsigned char *bytes,
                             size_t length) {
  VectorBridge *adapter = context;
  const SFStream *frame = &adapter->handle.owner->frame;
  SVSpan spans[3];
  size_t used = 0, start = 0, offset = frame->offset;
  size_t ends[3] = {1, frame->length - 1, frame->length};
  (void)bytes;
  (void)length;
  for (unsigned i = 0; i < 3; ++i) {
    size_t from = offset > start ? offset : start;
    if (from < ends[i] && !(SV_MUTANT == 4 && i == 2)) {
      spans[used++] = (SVSpan){frame->bytes + (SV_MUTANT == 3 ? start : from),
                               ends[i] - from};
    }
    start = ends[i];
  }
  return adapter->write(adapter->context, spans, used);
}

int sv_pumpv(SVHandle handle, SVWriteV write, void *context, unsigned budget,
             size_t *accepted) {
  if (!valid(handle) || !write || !accepted)
    return SF_INVALID;
  VectorBridge adapter = {handle, write, context};
  return sv_pump(handle, vectors, &adapter, budget, accepted);
}

int sv_cancel(SVHandle handle) {
  if (!valid(handle))
    return SF_INVALID;
  if (handle.owner->status != SF_PENDING)
    return SF_STATE;
  int result = sf_cancel(handle.owner->base);
  if (result == SF_OK)
    handle.owner->status = SF_CANCELLED;
  return result;
}

void svq_init(SVQueue *queue) {
  if (!queue)
    return;
  for (unsigned i = 0; i < 2; ++i) {
    sv_init(&queue->slots[i]);
    queue->tickets[i] = 0;
    queue->active[i] = 0;
  }
  queue->next_ticket = 1;
  queue->tainted = 0;
}

int svq_submit(SVQueue *queue, const unsigned char *bytes, size_t length,
               SVHandle *out) {
  if (!queue || !out || (!bytes && length))
    return SF_INVALID;
  if (length > 64 || queue->next_ticket == UINT64_MAX)
    return SF_RANGE;
  unsigned slot;
  for (slot = 0; slot < 2; ++slot)
    if (!queue->active[slot] &&
        queue->slots[slot].frame.generation != UINT32_MAX)
      break;
  if (slot == 2)
    return SF_STATE;
  int result = sv_start(&queue->slots[slot], bytes, length, out);
  if (result != SF_OK)
    return result;
  queue->active[slot] = 1;
  queue->tickets[slot] = queue->next_ticket++;
  return SF_OK;
}

static unsigned head(const SVQueue *queue) {
  if (!queue->active[0])
    return queue->active[1] ? 1 : 2;
  if (!queue->active[1])
    return 0;
  if (SV_MUTANT == 7)
    return queue->tickets[0] > queue->tickets[1] ? 0 : 1;
  return queue->tickets[0] < queue->tickets[1] ? 0 : 1;
}

int svq_pump(SVQueue *queue, SVWrite write, void *context, unsigned budget,
             SVEvent *out) {
  if (!queue || !write || !out)
    return SF_INVALID;
  if (queue->tainted && SV_MUTANT != 6)
    return SF_STATE;
  unsigned slot = head(queue);
  if (slot == 2)
    return SF_STATE;
  SVHandle handle = {&queue->slots[slot], queue->slots[slot].frame.generation};
  size_t accepted = 0;
  int result = sv_pump(handle, write, context, budget, &accepted);
  *out = (SVEvent){handle, accepted, result};
  if (result != SF_PENDING) {
    queue->active[slot] = 0;
    if (result == SF_IO || result == SF_INVALID)
      queue->tainted = 1;
  }
  return result;
}

int svq_cancel(SVQueue *queue, SVHandle handle, SVEvent *out) {
  if (!queue || !out)
    return SF_INVALID;
  unsigned slot;
  for (slot = 0; slot < 2; ++slot)
    if (handle.owner == &queue->slots[slot])
      break;
  if (slot == 2 || !queue->active[slot])
    return SF_INVALID;
  if (SV_MUTANT == 5)
    handle.generation = queue->slots[slot].frame.generation;
  int result = sv_cancel(handle);
  if (result != SF_OK)
    return result;
  size_t accepted = queue->slots[slot].frame.offset;
  *out = (SVEvent){handle, accepted, SF_CANCELLED};
  queue->active[slot] = 0;
  if (accepted)
    queue->tainted = 1;
  return SF_OK;
}

int svq_reset_transport(SVQueue *queue, unsigned verified_quiescent) {
  if (!queue)
    return SF_INVALID;
  if (verified_quiescent != 1)
    return SF_STATE;
  queue->tainted = 0;
  return SF_OK;
}
