/* Version1 consumer: this translation unit has no new-producer header. */
#include "old_api.h"
#include "depth.h"

int bf_old_render_wire(const unsigned char *p, size_t n, unsigned char *o,
                       size_t c, size_t *w)
{
    if (!p || !w || (!o && c) || n < 4)
        return B2_INVALID;
    if (p[0] != 1 || p[1] > 2)
        return B2_INVALID;
    size_t length = (size_t)p[2] * 256u + p[3];
    if (length > 256 || n != length + 4)
        return B2_INVALID;
    struct b2_result r;
    r.tag = p[1];
    if (r.tag == 1)
    {
        r.value.text.data = p + 4;
        r.value.text.length = length;
    }
    else
    {
        if (length != 4)
            return B2_INVALID;
        uint32_t raw = (uint32_t)p[4] * UINT32_C(16777216) +
                       (uint32_t)p[5] * 65536u + (uint32_t)p[6] * 256u + p[7];
        int32_t signed_value =
            raw <= INT32_MAX ? (int32_t)raw : -1 - (int32_t)(UINT32_MAX - raw);
        if (signed_value < -9999 || signed_value > 9999)
            return B2_RANGE;
        if (r.tag == 0)
            r.value.number = (int)signed_value;
        else
            r.value.error = (int)signed_value;
    }
    return b2_render(&r, o, c, w);
}
