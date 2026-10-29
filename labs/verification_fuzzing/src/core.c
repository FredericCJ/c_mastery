#include "core.h"

int vf_add(uint32_t a, uint32_t b, uint32_t *out)
{
    return 0;
}

size_t vf_lower_bound(const uint32_t *values, size_t n, uint32_t key)
{
    return 0;
}

int vf_ring_step(struct vf_ring *ring, unsigned op)
{
    return 0;
}

int vf_frame(const unsigned char *data, size_t n, unsigned *length)
{
    return 0;
}

size_t vf_shrink(unsigned char *data, size_t n, unsigned threshold)
{
    return 0;
}

int vf_request(struct vf_request *request, unsigned event)
{
    return 0;
}

int vf_digest(const unsigned char *frame, size_t n, uint32_t *out)
{
    return 0;
}

int vf_claim(unsigned success, unsigned unwound, unsigned witnessed,
             unsigned unchecked)
{
    return 0;
}
