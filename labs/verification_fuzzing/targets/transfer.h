#ifndef CM_VF_TRANSFER_H
#define CM_VF_TRANSFER_H
#include <stddef.h>
#include <stdint.h>
int vt_mul(uint32_t, uint32_t, uint32_t *);
int vt_narrow(int64_t, uint16_t *);

struct vt_record
{
    int32_t key;
    unsigned identity;
};

size_t vt_desc(const struct vt_record *, size_t, int32_t);
typedef void (*vt_notify_fn)(void *, const void *);
typedef int (*vt_prepare_fn)(void *, const void *);

struct vt_queue
{
    const void *payload[3];
    vt_notify_fn notify[3];
    void *context[3];
    unsigned n, busy;
};

int vt_offer(struct vt_queue *, const void *, vt_prepare_fn, vt_notify_fn,
             void *);
int vt_remove(struct vt_queue *);

struct vt_stream
{
    unsigned char bytes[18];
    size_t n;
    unsigned rejected;
};

void vt_reset(struct vt_stream *);
int vt_feed(struct vt_stream *, const unsigned char *, size_t);
int vt_finish(const struct vt_stream *, uint32_t *);

/* Events retain stable identities when deleted. op0 submit,1 cancel,2 complete,
 * 3 faulty release,4 irrelevant. Signature names first premature release. */
struct vt_event
{
    unsigned id, op;
};

unsigned vt_violation(const struct vt_event *, size_t);
size_t vt_shrink(struct vt_event *, size_t);
#endif
