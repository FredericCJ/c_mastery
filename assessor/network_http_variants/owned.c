#include "owned.h"
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#ifndef NV_MUTATION
#define NV_MUTATION 0
#endif
/* Admission/borrowing/closing share this mutex. No callback or cancellation
   point in the published critical sections except Linux close; callers must
   disable pthread cancellation around each API call. */
static int valid(struct nv_owned *p, struct hf_handle h)
{
    return h.index < 4 && p->core.slot[h.index].live &&
           p->core.slot[h.index].generation == h.generation;
}

static void finish(struct nv_owned *p, struct hf_handle h)
{
    int fd = -1;
    (void)hf_close(&p->core, h, &fd);
    /* On Linux a valid owned fd is released even if close reports EINTR/EIO.
       Do not retry a numeric fd after close. No peer-delivery claim. */
    (void)close(fd);
    p->closing[h.index] = 0;
}

int nv_owned_init(struct nv_owned *p)
{
    if (!p)
    {
        return NV_INVALID;
    }
    memset(&p->core, 0, sizeof p->core);
    memset(p->closing, 0, sizeof p->closing);
    return pthread_mutex_init(&p->mutex, 0) ? NV_INVALID : NV_OK;
}

int nv_owned_admit(struct nv_owned *p, int fd, struct hf_handle *out)
{
    if (!p || !out || fd < 0)
    {
        return NV_INVALID;
    }
    int fl = fcntl(fd, F_GETFL), df = fcntl(fd, F_GETFD);
    if (fl < 0 || df < 0 || !(fl & O_NONBLOCK) || !(df & FD_CLOEXEC))
    {
        return NV_INVALID;
    }
    pthread_mutex_lock(&p->mutex);
    struct hf_handle h;
    int r = hf_admit(&p->core, fd, 3, &h);
    if (!r)
    {
        *out = h;
    }
    pthread_mutex_unlock(&p->mutex);
    return r == 0 ? NV_OK : r == 1 ? NV_FULL : NV_INVALID;
}

int nv_owned_borrow(struct nv_owned *p, struct hf_handle h,
                    struct nv_lease *out)
{
    if (!p || !out)
    {
        return NV_INVALID;
    }
    pthread_mutex_lock(&p->mutex);
    int r = NV_STALE;
    if (valid(p, h) && (!p->closing[h.index] || NV_MUTATION == 8))
    {
        if (!hf_pin(&p->core, h))
        {
            *out = (struct nv_lease){p, h, p->core.slot[h.index].fd, 1};
            r = NV_OK;
        }
        else
        {
            r = NV_FULL;
        }
    }
    pthread_mutex_unlock(&p->mutex);
    return r;
}

int nv_owned_close(struct nv_owned *p, struct hf_handle h)
{
    if (!p)
    {
        return NV_INVALID;
    }
    pthread_mutex_lock(&p->mutex);
    int r = NV_STALE;
    if (valid(p, h) && !p->closing[h.index])
    {
        p->closing[h.index] = 1;
        r = NV_OK;
        if (!p->core.slot[h.index].pending || NV_MUTATION == 7)
        {
            finish(p, h);
        }
    }
    pthread_mutex_unlock(&p->mutex);
    return r;
}

int nv_owned_release(struct nv_lease *l)
{
    if (!l || !l->active || !l->owner)
    {
        return NV_STALE;
    }
    struct nv_owned *p = l->owner;
    pthread_mutex_lock(&p->mutex);
    int r = NV_STALE;
    if (valid(p, l->handle) && p->core.slot[l->handle.index].pending)
    {
        int ignored;
        (void)hf_callback(&p->core, l->handle, &ignored);
        l->active = 0;
        r = NV_OK;
        if (p->closing[l->handle.index] &&
            !p->core.slot[l->handle.index].pending)
        {
            finish(p, l->handle);
        }
    }
    pthread_mutex_unlock(&p->mutex);
    return r;
}

int nv_owned_destroy(struct nv_owned *p)
{
    if (!p)
    {
        return NV_INVALID;
    }
    pthread_mutex_lock(&p->mutex);
    for (unsigned i = 0; i < 4; i++)
    {
        if (p->core.slot[i].live || p->core.slot[i].pending)
        {
            pthread_mutex_unlock(&p->mutex);
            return NV_BUSY;
        }
    }
    pthread_mutex_unlock(&p->mutex);
    return pthread_mutex_destroy(&p->mutex) ? NV_BUSY : NV_OK;
}
