#ifndef CHECKPOINT_WRITER_FOLLOWTHROUGH_H
#define CHECKPOINT_WRITER_FOLLOWTHROUGH_H
#include "integration.h"
#include "checkpoint_writer.h"

/*
 * Byte transfer: the ring owns bytes not yet removed, struct in_transfer owns
 * the single active byte, and the sink owns accepted bytes. Controls start
 * zero-initialized; callers serialize access and the writer callback does not
 * reenter.
 */
struct cwf_byte
{
    struct in_transfer active;
    struct cpw_error error;
    unsigned failed;
    unsigned recoveries;
    unsigned calls;
    unsigned requeued;
};

/*
 * Makes at most `budget` bytes of progress through in_drain().
 * Returns 0 drained, 1 budget or retry pending, -1 terminal failure (a null
 * argument, a failed control, a queue value outside 0..255, or a writer
 * error). While failed, the writer is
 * not called again until cwf_byte_recover(). The first terminal cause is
 * recorded in error.primary; later causes do not overwrite it.
 */
int cwf_byte_step(algo_ring *, struct cwf_byte *, f_writer_fn, void *,
                  unsigned budget);

/* Clears the failed state, keeping the active byte. Returns -1 if not failed. */
int cwf_byte_recover(struct cwf_byte *);

/*
 * Span transfer: owns a private copy of at most 16 bytes, so the caller's
 * source may change after a successful begin/take. An active span must stay at
 * a stable address because transfer.data points into bytes[].
 */
struct cwf_span
{
    struct cpw_transfer transfer;
    unsigned char bytes[16];
    unsigned active;
    size_t count;
};

/* Copies n bytes (n <= 16) into an idle span. Returns 0, or -1 if busy/invalid. */
int cwf_span_begin(struct cwf_span *, const unsigned char *, size_t, unsigned);

/*
 * Stages the first n ring values (each 0..255, n <= 16) into an idle span, then
 * removes them from the ring. Returns 0, or -1 with the ring unchanged.
 */
int cwf_span_take(algo_ring *, struct cwf_span *, size_t, unsigned);

/* One writer callback at most. Returns CPW_INVALID for an inactive span. */
enum cpw_status cwf_span_step(struct cwf_span *, cpw_write_fn, void *);

/* Resumes a failed span at its retained offset; CPW_INVALID if inactive. */
enum cpw_status cwf_span_recover(struct cwf_span *);

enum cwf_category
{
    CWF_WAIT,
    CWF_COMPLETE,
    CWF_IO
};

struct cwf_error
{
    enum cwf_category category;
    unsigned operation;
    size_t accepted;
    int primary;
    int cleanup;
};

/*
 * Copies the transfer's diagnostic record into *out without logging or I/O.
 * Returns 0, or -1 for a null argument or a transfer whose status is not
 * pending, done or failed.
 */
int cwf_error(const struct cpw_transfer *, struct cwf_error *);
#endif
