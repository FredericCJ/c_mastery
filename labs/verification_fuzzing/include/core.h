#ifndef B06_VF_H
#define B06_VF_H
#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#include <math.h>

struct vf_ring
{
    unsigned head, size, capacity;
};

struct vf_request
{
    unsigned submitted, completed, cancelled, released;
};

int vf_add(uint32_t a, uint32_t b, uint32_t *out);
size_t vf_lower_bound(const uint32_t *values, size_t n, uint32_t key);
int vf_ring_step(struct vf_ring *ring, unsigned op);
int vf_frame(const unsigned char *data, size_t n, unsigned *length);
size_t vf_shrink(unsigned char *data, size_t n, unsigned threshold);
int vf_request(struct vf_request *request, unsigned event);
int vf_digest(const unsigned char *frame, size_t n, uint32_t *out);
int vf_claim(unsigned success, unsigned unwound, unsigned witnessed,
             unsigned unchecked);
#endif
