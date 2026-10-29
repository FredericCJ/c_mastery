#ifndef CMASTERY_REPLICATED_OUTBOX_TRANSPORT_H
#define CMASTERY_REPLICATED_OUTBOX_TRANSPORT_H
#include "outbox.h"
#include "stream.h"
/* The original record protocol uses exactly 8-byte payload frames with a
 * one-octet length header: eight frames/request, three/reply. A record ends
 * with EOF. Fresh connections/parsers after any failure. Synchronous callbacks
 * borrow storage only during the call and may not reenter either adapter.
 * read: positive prefix <=capacity, 0 EOF, -2 retry, other negative error.
 * write: positive prefix <=length, 0 retry, negative error. All attempts count
 * against limit; no callback may block indefinitely in the bounded model. */
typedef ptrdiff_t (*rpo_reader_fn)(void *, unsigned char *, size_t);
typedef int (*rpo_writer_fn)(void *, const unsigned char *, size_t);
int rpo_wire_read(rpo_reader_fn read, void *context, unsigned limit,
                  size_t length, unsigned char *out);
int rpo_wire_write(rpo_writer_fn write, void *context, unsigned limit,
                   const unsigned char *bytes, size_t length);
int rpo_reply_encode(struct rpo_reply reply, unsigned char out[24]);
int rpo_reply_decode(const unsigned char in[24], struct rpo_reply *out);
#endif
