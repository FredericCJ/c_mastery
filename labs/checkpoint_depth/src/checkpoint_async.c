#include "checkpoint_async.h"

enum cpa_status cpa_init(struct cpa_queue *q, unsigned n)
{
    (void)q;
    (void)n;
    return CPA_ERROR;
}

enum cpa_status cpa_offer(struct cpa_queue *q, const struct cp_message *m,
                          int b)
{
    (void)q;
    (void)m;
    (void)b;
    return CPA_ERROR;
}

enum cpa_status cpa_take(struct cpa_queue *q, struct cpa_handle *h,
                         struct cp_message *m, int b)
{
    (void)q;
    (void)h;
    (void)m;
    (void)b;
    return CPA_ERROR;
}

enum cpa_status cpa_finish(struct cpa_queue *q, struct cpa_handle h, int f)
{
    (void)q;
    (void)h;
    (void)f;
    return CPA_ERROR;
}

enum cpa_status cpa_close(struct cpa_queue *q, int a)
{
    (void)q;
    (void)a;
    return CPA_ERROR;
}

enum cpa_status cpa_wait_closed(struct cpa_queue *q, const struct timespec *t)
{
    (void)q;
    (void)t;
    return CPA_ERROR;
}

enum cpa_status cpa_destroy(struct cpa_queue *q)
{
    (void)q;
    return CPA_ERROR;
}
