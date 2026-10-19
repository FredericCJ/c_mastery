#ifndef CM_STREAM_DEPTH_H
#define CM_STREAM_DEPTH_H
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
_Static_assert(CHAR_BIT == 8, "This wire protocol requires octets");
_Static_assert(UINT_MAX >= 65536u, "Sequence retirement needs value 65536");
typedef struct {
  unsigned length;
  unsigned char bytes[8];
} RfFrame;
typedef struct {
  unsigned width, header_used, length, used, failed;
  unsigned char payload[8];
} RfParser;
/* width1 or2; two-byte header is big-endian; length0..8; invalid init
 * unchanged. */
int rf_parser_init(RfParser *, unsigned width);
/* Return1 complete frame,0 consumed incomplete byte,-1 malformed/invalid.
 * Output unchanged unless1. Malformed length latches failure until explicit
 * init. Caller consumes exactly one byte on0/1; malformed stream is discarded.
 */
int rf_push(RfParser *, unsigned char byte, RfFrame *);
/* Span adapter: consume at most through the first complete frame; caller feeds
 * the unconsumed suffix. Empty span returns0 with consumed0. Invalid arguments
 * do not mutate state/output; malformed-byte consumption is reported. */
int rf_feed(RfParser *, const unsigned char *, size_t length, size_t *consumed,
            RfFrame *);
/* Return0 only at a frame boundary; incomplete/malformed EOF latches failure.
 */
int rf_eof(RfParser *);
enum RfOwner { RF_CALLER, RF_OUTBOX, RF_DELIVERED, RF_CANCELLED };
typedef struct {
  unsigned id, payload_length, wire_length, offset;
  unsigned char wire[10];
  enum RfOwner owner;
} RfMessage;
typedef struct {
  RfMessage *items[8];
  unsigned head, count, active;
} RfConnection;
typedef struct {
  RfConnection connections[4];
  size_t bytes, limit;
} RfOutbox;
int rf_message_init(RfMessage *, unsigned id, unsigned width, const RfFrame *);
/* Fresh zeroed outbox; byte limit0..256. Limit zero admits only empty payloads,
 * still bounded by4connections and8frames each. Headers/metadata separate. */
int rf_outbox_init(RfOutbox *, size_t byte_limit);
int rf_open(RfOutbox *, unsigned *slot);
/* Enqueue1 transfers exclusive access,0 bound rejects,-1 contract rejects.
 * Live request IDs and pointers must be unique across all connections.
 * Borrowed wire returned by peek valid only until advance/disconnect; no async
 * kernel-retained buffers are allowed under this synchronous write contract. */
int rf_enqueue(RfOutbox *, unsigned slot, RfMessage *);
int rf_peek(RfOutbox *, unsigned slot, const unsigned char **bytes,
            size_t *length);
int rf_advance(RfOutbox *, unsigned slot, size_t committed_bytes);
/* Writer returns positive actual committed count,0 would-block,-1 failure.
 * It borrows bytes only during the call, and must not reenter/mutate this box.
 * An actual socket adapter normalizes EAGAIN/EINTR separately. Never advance
 * by requested length when the writer committed a shorter prefix. */
int rf_send_once(RfOutbox *, unsigned slot,
                 int (*writer)(void *, const unsigned char *, size_t),
                 void *context);
int rf_disconnect(RfOutbox *, unsigned slot);
int rf_outbox_invariant(const RfOutbox *);

typedef struct {
  unsigned id, delta;
  uint32_t result;
} RfCached;
typedef struct {
  unsigned session, count;
  uint32_t total;
  RfCached cache[4];
} RfDedup;
/* Serialized, live-server cache. Same(session,id,delta) returns cached result0;
 * new accepted effect1. Collision/different session/full/overflow reject-1
 * unchanged. No eviction, persistence or cross-crash exactly-once guarantee. */
int rf_dedup_init(RfDedup *, unsigned session);
int rf_effect(RfDedup *, unsigned session, unsigned id, unsigned delta,
              uint32_t *result);

typedef struct {
  unsigned session, next, pending, recent[3], recent_count;
} RfSequence;
enum {
  RF_SEQ_ACCEPT = 1,
  RF_SEQ_DUPLICATE = 0,
  RF_SEQ_STALE = -1,
  RF_SEQ_SESSION = -2,
  RF_SEQ_WINDOW = -3,
  RF_SEQ_RETIRED = -4,
  RF_SEQ_INVALID = -5
};
/* Window3, no modulo comparison: sequence0..65535, next65536 retires session.
 * 'delivered' has3slots; output count initialized to0 for valid arguments.
 * A new explicit session rejects delayed packets from previous sessions. */
int rf_sequence_init(RfSequence *, unsigned session, unsigned first);
int rf_arrive(RfSequence *, unsigned session, unsigned sequence,
              unsigned delivered[3], unsigned *count);
/* Explicit policy expiration when a gap has later buffered data. Report one
 * missing sequence and drain newly contiguous data; this is not proof of loss.
 */
int rf_expire_gap(RfSequence *, unsigned *missing, unsigned delivered[3],
                  unsigned *count);
enum { RF_PAYLOAD_TRUNCATED = 1, RF_CONTROL_TRUNCATED = 2 };
/* Selected application requires complete payload AND ancillary metadata. A
 * reported length may exceed capacity when recvmsg uses input MSG_TRUNC.
 * Return1 complete,0 rejected truncation/oversize,-1 unknown flag bits. Zero
 * length is a valid complete datagram; it does not denote stream EOF. */
int rf_datagram_accept(size_t reported_length, size_t capacity, unsigned flags);
#endif
