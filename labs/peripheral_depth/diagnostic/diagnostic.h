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
typedef struct {
  uint16_t sequence, length;
  uint8_t payload[32];
} DGMessage;
typedef struct {
  uint16_t last;
  unsigned accepted;
} DGSession;
/* Strict nonwrapping sequence policy; session identity is an external contract.
 */
int dg_encode(const DGMessage *, uint8_t *, size_t, size_t *);
int dg_decode(const uint8_t *, size_t, DGMessage *);
int dg_accept(DGSession *, const uint8_t *, size_t, DGMessage *);
typedef struct {
  uint8_t bytes[38];
  size_t used, needed;
  unsigned ready, poisoned;
} DGStream;
int dg_stream_feed(DGStream *, const uint8_t *, size_t, size_t *);
int dg_stream_take(DGStream *, DGSession *, DGMessage *);
int dg_stream_end(const DGStream *);
#endif
