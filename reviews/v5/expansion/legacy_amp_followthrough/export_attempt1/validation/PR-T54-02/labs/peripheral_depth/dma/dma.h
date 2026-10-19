#ifndef PERIPHERAL_DMA_DEPTH_H
#define PERIPHERAL_DMA_DEPTH_H
#include <stddef.h>
#include <stdint.h>
enum {
  PD_OK,
  PD_INVALID,
  PD_BUSY,
  PD_STALE,
  PD_CANCELLED,
  PD_IO,
  PD_UNIMPLEMENTED
};
enum { PD_F439, PD_H745 };
enum { PD_TC = 32, PD_TE = 8, PD_DME = 4, PD_FE = 1 };
enum { PD_RELEASE, PD_ACQUIRE, PD_CLEAN, PD_INVALIDATE };
typedef struct {
  uint32_t source, destination, bytes, source_capacity, destination_capacity,
      width;
} PdExtent;
typedef struct {
  uint32_t token, enabled, remaining, flags, fifo_empty;
} PdSnapshot;
typedef struct {
  void *context;
  int (*configure)(void *, const PdExtent *, uint32_t);
  void (*enable)(void *);
  void (*stop)(void *);
  PdSnapshot (*snapshot)(void *);
  void (*visibility)(void *, unsigned, uint32_t, uint32_t);
} PdIo;
typedef struct {
  PdExtent extents[4];
  uint32_t last_token, token, count, index, total, done, touched, result, phase,
      board, cached;
} PdTransfer;
typedef struct {
  uint32_t token, status, complete_bytes, possibly_written_bytes;
} PdResult;
int pd_validate(unsigned, unsigned, const PdExtent *, unsigned, uint32_t *);
int pd_submit(PdTransfer *, PdIo, unsigned, unsigned, const PdExtent *,
              unsigned, uint32_t);
int pd_poll(PdTransfer *, PdIo, uint32_t);
int pd_cancel(PdTransfer *, PdIo, uint32_t);
int pd_finalize(PdTransfer *, PdIo, uint32_t, PdResult *);
/* Build retry extents only after finalization. Memory-copy semantics are
   explicitly idempotent; this operation does not claim arbitrary peripheral
   resumability. */
int pd_suffix(const PdExtent *, unsigned, uint32_t, PdExtent *, unsigned *);
typedef struct {
  uint32_t token, configuration;
} PdPort;
PdIo pd_target_io(PdPort *);
enum { PD_NOTICE_PUBLISH, PD_NOTICE_TAKE, PD_NOTICE_ACK, PD_NOTICE_QUERY };
typedef struct {
  uint32_t epoch, sequence, state, status, complete, possible, destination,
      extent;
} PdNotice;
_Static_assert(sizeof(PdNotice) == 32, "One whole metadata line");
int pd_notice(PdNotice *, unsigned, uint32_t, uint32_t, const PdNotice *,
              PdNotice *);
int pd_h745_notice(unsigned, uint32_t, uint32_t, const PdNotice *, PdNotice *);
int pd_h745_notice_cold_start(uint32_t);
#endif
