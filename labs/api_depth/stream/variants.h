#ifndef CMASTERY_STREAM_VARIANTS_H
#define CMASTERY_STREAM_VARIANTS_H
#include "stream.h"

enum { SV_PROGRESS, SV_WAIT, SV_RETRY, SV_ERROR };
struct sv_write_result {
  size_t accepted;
  unsigned disposition;
};
typedef struct sv_write_result (*sv_write_fn)(void *, const unsigned char *, size_t);
struct sv_span {
  const unsigned char *data;
  size_t length;
};
typedef struct sv_write_result (*sv_write_v_fn)(void *, const struct sv_span *, size_t);
struct sv_stream {
  struct sf_stream frame;
  struct sf_handle base;
  int status;
};
struct sv_handle {
  struct sv_stream *owner;
  uint32_t generation;
};

void sv_init(struct sv_stream *);
int sv_start(struct sv_stream *, const unsigned char *, size_t, struct sv_handle *);
int sv_pump(struct sv_handle, sv_write_fn, void *, unsigned, size_t *);
int sv_pumpv(struct sv_handle, sv_write_v_fn, void *, unsigned, size_t *);
int sv_cancel(struct sv_handle);

struct sv_queue {
  struct sv_stream slots[2];
  uint64_t tickets[2], next_ticket;
  unsigned active[2], tainted;
};
struct sv_event {
  struct sv_handle handle;
  size_t accepted;
  int status;
};

void svq_init(struct sv_queue *);
int svq_submit(struct sv_queue *, const unsigned char *, size_t, struct sv_handle *);
int svq_pump(struct sv_queue *, sv_write_fn, void *, unsigned, struct sv_event *);
int svq_cancel(struct sv_queue *, struct sv_handle, struct sv_event *);
int svq_reset_transport(struct sv_queue *, unsigned verified_quiescent);
#endif
