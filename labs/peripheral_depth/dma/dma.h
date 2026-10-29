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
struct pd_extent {
  uint32_t source, destination, bytes, source_capacity, destination_capacity,
      width;
};
struct pd_snapshot {
  uint32_t token, enabled, remaining, flags, fifo_empty;
};
struct pd_io {
  void *context;
  int (*configure)(void *, const struct pd_extent *, uint32_t);
  void (*enable)(void *);
  void (*stop)(void *);
  struct pd_snapshot (*snapshot)(void *);
  void (*visibility)(void *, unsigned, uint32_t, uint32_t);
};
struct pd_transfer {
  struct pd_extent extents[4];
  uint32_t last_token, token, count, index, total, done, touched, result, phase,
      board, cached;
};
struct pd_result {
  uint32_t token, status, complete_bytes, possibly_written_bytes;
};
int pd_validate(unsigned, unsigned, const struct pd_extent *, unsigned, uint32_t *);
int pd_submit(struct pd_transfer *, struct pd_io, unsigned, unsigned, const struct pd_extent *,
              unsigned, uint32_t);
int pd_poll(struct pd_transfer *, struct pd_io, uint32_t);
int pd_cancel(struct pd_transfer *, struct pd_io, uint32_t);
int pd_finalize(struct pd_transfer *, struct pd_io, uint32_t, struct pd_result *);
/* Build retry extents only after finalization. Memory-copy semantics are
   explicitly idempotent; this operation does not claim arbitrary peripheral
   resumability. */
int pd_suffix(const struct pd_extent *, unsigned, uint32_t, struct pd_extent *, unsigned *);
struct pd_port {
  uint32_t token, configuration;
};
struct pd_io pd_target_io(struct pd_port *);
enum { PD_NOTICE_PUBLISH, PD_NOTICE_TAKE, PD_NOTICE_ACK, PD_NOTICE_QUERY };
struct pd_notice {
  uint32_t epoch, sequence, state, status, complete, possible, destination,
      extent;
};
_Static_assert(sizeof(struct pd_notice) == 32, "One whole metadata line");
int pd_notice(struct pd_notice *, unsigned, uint32_t, uint32_t, const struct pd_notice *,
              struct pd_notice *);
int pd_h745_notice(unsigned, uint32_t, uint32_t, const struct pd_notice *, struct pd_notice *);
int pd_h745_notice_cold_start(uint32_t);
#endif
