#include "service_io.h"

int rzv_request_encode(struct rzj_request r, unsigned char b[RZV_FRAME_BYTES])
{
    (void)r;
    (void)b;
    return RZ_INVALID;
}

void rzv_frame_init(struct rzv_frame *f)
{
    if (f)
    {
        *f = (struct rzv_frame){0};
    }
}

int rzv_frame_feed(struct rzv_frame *f, const unsigned char *p, size_t n,
                   size_t *c, struct rzj_request *o)
{
    (void)f;
    (void)p;
    (void)n;
    (void)c;
    (void)o;
    return RZ_INVALID;
}

int rzv_frame_end(struct rzv_frame *f)
{
    (void)f;
    return RZ_INVALID;
}

int rzv_reply_decode(const unsigned char b[RZV_FRAME_BYTES],
                     struct rzv_reply *o)
{
    (void)b;
    (void)o;
    return RZ_INVALID;
}

int rzv_reply_encode(struct rzv_reply r, unsigned char b[RZV_FRAME_BYTES])
{
    (void)r;
    (void)b;
    return RZ_INVALID;
}

int rzv_now(uint64_t *o)
{
    (void)o;
    return RZ_INVALID;
}

int rzv_init(struct rzv_service *s, struct rzs_outbox *o, unsigned c,
             uint64_t n)
{
    (void)s;
    (void)o;
    (void)c;
    (void)n;
    return RZ_INVALID;
}

int rzv_serve(struct rzv_service *s, struct rzf_scope *p, int f, int c,
              size_t n, struct rzv_observation *o)
{
    (void)s;
    (void)p;
    (void)f;
    (void)c;
    (void)n;
    (void)o;
    return RZ_INVALID;
}

int rzv_close(struct rzv_service *s)
{
    (void)s;
    return RZ_INVALID;
}

int rzv_destroy(struct rzv_service *s)
{
    (void)s;
    return RZ_INVALID;
}

int rzv_notify(struct rzv_service *s, unsigned p, unsigned n,
               struct rzs_event *o)
{
    (void)s;
    (void)p;
    (void)n;
    (void)o;
    return RZ_INVALID;
}
