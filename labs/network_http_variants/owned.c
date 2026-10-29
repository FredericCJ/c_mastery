#include "owned.h"

int nv_owned_init(struct nv_owned *p)
{
    (void)p;
    return NV_UNIMPLEMENTED;
}

int nv_owned_admit(struct nv_owned *p, int f, struct hf_handle *h)
{
    (void)p;
    (void)f;
    (void)h;
    return NV_UNIMPLEMENTED;
}

int nv_owned_borrow(struct nv_owned *p, struct hf_handle h, struct nv_lease *l)
{
    (void)p;
    (void)h;
    (void)l;
    return NV_UNIMPLEMENTED;
}

int nv_owned_close(struct nv_owned *p, struct hf_handle h)
{
    (void)p;
    (void)h;
    return NV_UNIMPLEMENTED;
}

int nv_owned_release(struct nv_lease *l)
{
    (void)l;
    return NV_UNIMPLEMENTED;
}

int nv_owned_destroy(struct nv_owned *p)
{
    (void)p;
    return NV_UNIMPLEMENTED;
}
