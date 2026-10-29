#include <stdint.h>
#include <stdio.h>
#define X(n, o, w)                                    \
    enum                                              \
    {                                                 \
        XM_##n##_SHIFT = o,                           \
        XM_##n##_WIDTH = w,                           \
        XM_##n##_MASK = ((UINT32_C(1) << w) - 1) << o \
    };
#include "fields.inc"
#undef X
_Static_assert((XM_tag_MASK & XM_length_MASK) == 0, "overlap tag length");
_Static_assert((XM_tag_MASK & XM_flags_MASK) == 0, "overlap tag flags");
_Static_assert((XM_length_MASK & XM_flags_MASK) == 0, "overlap length flags");
#ifdef XM_SEMANTIC
_Static_assert(XM_length_WIDTH >= XM_tag_WIDTH, "semantic length>=tag");
#endif
int main(void)
{
    printf("%u %u %u\n", (unsigned)XM_tag_MASK, (unsigned)XM_length_MASK,
           (unsigned)XM_flags_MASK);
    return 0;
}
