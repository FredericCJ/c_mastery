#include "verifier.h"

int vv_verify(const struct cvm_program *p, unsigned d, unsigned i,
              struct vv_work *w, struct vv_report *r)
{
    (void)p;
    (void)d;
    (void)i;
    (void)w;
    (void)r;
    return VV_UNIMPLEMENTED;
}

int vv_load(const uint8_t *b, size_t n, unsigned d, unsigned i,
            struct vv_work *w, struct cvm_program *p, struct vv_report *r)
{
    (void)b;
    (void)n;
    (void)d;
    (void)i;
    (void)w;
    (void)p;
    (void)r;
    return VV_UNIMPLEMENTED;
}

int vv_lower(const struct cvm_program *p, unsigned d, struct vv_work *w,
             struct cvm_program *out, struct vv_report *r)
{
    (void)p;
    (void)d;
    (void)w;
    (void)out;
    (void)r;
    return VV_UNIMPLEMENTED;
}
