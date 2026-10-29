#ifndef CM_DIAGNOSTIC_H
#define CM_DIAGNOSTIC_H
#include <stddef.h>
#include <stdint.h>
enum {
  DG_OK,
  DG_INVALID,
  DG_FULL,
  DG_STALE,
  DG_BUSY,
  DG_FRAME,
  DG_IO,
  DG_END,
  DG_UNIMPLEMENTED = 99
};
struct dg_message {
  uint16_t sequence, length;
  uint8_t payload[32];
};
struct dg_session {
  uint16_t last;
  unsigned accepted;
};
/* Strict nonwrapping sequence policy; session identity is an external contract.
 */
int dg_encode(const struct dg_message *, uint8_t *, size_t, size_t *);
int dg_decode(const uint8_t *, size_t, struct dg_message *);
int dg_accept(struct dg_session *, const uint8_t *, size_t, struct dg_message *);
struct dg_stream {
  uint8_t bytes[38];
  size_t used, needed;
  unsigned ready, poisoned;
};
int dg_stream_feed(struct dg_stream *, const uint8_t *, size_t, size_t *);
int dg_stream_take(struct dg_stream *, struct dg_session *, struct dg_message *);
int dg_stream_end(const struct dg_stream *);
#endif
