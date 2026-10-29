#ifndef CMASTERY_ADT_PACKET_RING_H
#define CMASTERY_ADT_PACKET_RING_H
#include "adt.h"
#include "arch.h"
#include <stdint.h>
#include <limits.h>
_Static_assert(CHAR_BIT == 8, "packet wire requires eight-bit bytes");
#define ADP_CAP 32u

enum adp_status
{
    ADP_OK,
    ADP_INVALID,
    ADP_FULL,
    ADP_BUSY,
    ADP_STALE,
    ADP_CANCELLED,
    ADP_RETIRED,
    ADP_PENDING,
    ADP_HEADER,
    ADP_KIND,
    ADP_LENGTH,
    ADP_ACK,
    ADP_IO,
    ADP_EOF,
    ADP_OVERSIZE,
    ADP_CODEC
};

struct adp_ring
{
    unsigned char bytes[ADP_CAP];
    size_t head, count, held;
    uint32_t next, generation;
    unsigned active, cancelled, retired, closed;
};

struct adp_token
{
    const struct adp_ring *owner;
    uint32_t generation;
};

struct adp_part
{
    const unsigned char *data;
    size_t length;
};

struct adp_lease
{
    unsigned kind;
    size_t length;
    struct adp_part part[2];
    struct adp_token token;
};
/* Live disjoint controls/output; initialized rings fixed-address, serialized,
   noncopied and not reinitialized while any token/view survives. Raw payload
   access requires a current unreleased lease; cancellation alone retains it. */
enum adp_status adp_init(struct adp_ring *, size_t origin,
                         uint32_t first_generation);
enum adp_status adp_feed(struct adp_ring *, const unsigned char *, size_t);
enum adp_status adp_discard(struct adp_ring *, size_t);
enum adp_status adp_acquire(struct adp_ring *, size_t exact_frame_bytes,
                            struct adp_lease *);
enum adp_status adp_validate(const struct adp_ring *, struct adp_token);
enum adp_status adp_cancel(struct adp_ring *, struct adp_token);
enum adp_status adp_release(struct adp_ring *, struct adp_token);
enum adp_status adp_close(struct adp_ring *);
enum adp_status adp_copy(const struct adp_ring *, struct adp_lease,
                         unsigned char *, size_t);
enum adp_status adp_record(const struct adp_ring *, struct adp_lease,
                           unsigned version, struct ar_record *);
/* Contiguous parsing preserves the retained error precedence. No payload copy.
   Output does not alias input. NULL+0 input is a header error. */
enum adp_status adp_parse(const unsigned char *, size_t, struct ad_packet *);
enum adp_status adp_build(unsigned, const unsigned char *, size_t,
                          unsigned char *, size_t, size_t *);
/* Blocking actual io.frame adapter. Caller owns storage until final packet use.
   Transport failures may consume bytes and modify storage, but preserve *out.
   Close-on-error is caller policy; no resumable stream or timeout claim. */
enum adp_status adp_read_pipe(int, unsigned char *, size_t, struct ad_packet *);
#endif
