#include "stream.h"

enum msg_status mfq_init(struct mfq_stream *s, unsigned c, struct mfo_value *v,
                         msg_next_fn n, msg_terminal_fn t, bool f)
{
    (void)s;
    (void)c;
    (void)v;
    (void)n;
    (void)t;
    (void)f;
    return MSG_INVALID;
}

enum msg_status mfq_acquire(struct mfq_stream *s, struct mfq_handle *h)
{
    (void)s;
    (void)h;
    return MSG_INVALID;
}

void mfq_release(struct mfq_handle *h)
{
    (void)h;
}

enum msg_status mfq_request(struct mfq_handle *h, uint64_t n)
{
    (void)h;
    (void)n;
    return MSG_INVALID;
}

enum msg_status mfq_submit(struct mfq_handle *h, unsigned v, enum mfq_policy p)
{
    (void)h;
    (void)v;
    (void)p;
    return MSG_INVALID;
}

enum msg_status mfq_end(struct mfq_handle *h, enum msg_status s)
{
    (void)h;
    (void)s;
    return MSG_INVALID;
}

enum msg_status mfq_cancel(struct mfq_handle *h)
{
    (void)h;
    return MSG_INVALID;
}

enum msg_status mfq_close(struct mfq_stream *s)
{
    (void)s;
    return MSG_INVALID;
}

enum msg_status mfq_join(struct mfq_stream *s)
{
    (void)s;
    return MSG_INVALID;
}

enum msg_status mfq_idle(struct mfq_stream *s)
{
    (void)s;
    return MSG_INVALID;
}

enum msg_status mfq_stats(struct mfq_stream *s, struct mfq_stats *out)
{
    (void)s;
    (void)out;
    return MSG_INVALID;
}

enum msg_status mfq_destroy(struct mfq_stream *s)
{
    (void)s;
    return MSG_INVALID;
}
