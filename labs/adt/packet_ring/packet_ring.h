#ifndef CMASTERY_ADT_PACKET_RING_H
#define CMASTERY_ADT_PACKET_RING_H
#include "adt.h"
#include "arch.h"
#include <stdint.h>
#include <limits.h>
_Static_assert(CHAR_BIT == 8, "packet wire requires eight-bit bytes");
#define ADP_CAP 32u
typedef enum { ADP_OK, ADP_INVALID, ADP_FULL, ADP_BUSY, ADP_STALE,
 ADP_CANCELLED, ADP_RETIRED, ADP_PENDING, ADP_HEADER, ADP_KIND,
 ADP_LENGTH, ADP_ACK, ADP_IO, ADP_EOF, ADP_OVERSIZE, ADP_CODEC } AdpStatus;
typedef struct AdpRing {
 unsigned char bytes[ADP_CAP];
 size_t head, count, held;
 uint32_t next, generation;
 unsigned active, cancelled, retired, closed;
} AdpRing;
typedef struct { const AdpRing *owner; uint32_t generation; } AdpToken;
typedef struct { const unsigned char *data; size_t length; } AdpPart;
typedef struct { unsigned kind; size_t length; AdpPart part[2]; AdpToken token; } AdpLease;
/* Live disjoint controls/output; initialized rings fixed-address, serialized,
   noncopied and not reinitialized while any token/view survives. Raw payload
   access requires a current unreleased lease; cancellation alone retains it. */
AdpStatus adp_init(AdpRing *, size_t origin, uint32_t first_generation);
AdpStatus adp_feed(AdpRing *, const unsigned char *, size_t);
AdpStatus adp_discard(AdpRing *, size_t);
AdpStatus adp_acquire(AdpRing *, size_t exact_frame_bytes, AdpLease *);
AdpStatus adp_validate(const AdpRing *, AdpToken);
AdpStatus adp_cancel(AdpRing *, AdpToken);
AdpStatus adp_release(AdpRing *, AdpToken);
AdpStatus adp_close(AdpRing *);
AdpStatus adp_copy(const AdpRing *, AdpLease, unsigned char *, size_t);
AdpStatus adp_record(const AdpRing *, AdpLease, unsigned version, ArRecord *);
/* Contiguous parsing preserves the retained error precedence. No payload copy.
   Output does not alias input. NULL+0 input is a header error. */
AdpStatus adp_parse(const unsigned char *, size_t, ADPacket *);
AdpStatus adp_build(unsigned, const unsigned char *, size_t, unsigned char *, size_t, size_t *);
/* Blocking actual io.frame adapter. Caller owns storage until final packet use.
   Transport failures may consume bytes and modify storage, but preserve *out.
   Close-on-error is caller policy; no resumable stream or timeout claim. */
AdpStatus adp_read_pipe(int, unsigned char *, size_t, ADPacket *);
#endif
