#include "format_stream.h"
#ifndef MFP_DIAGNOSTIC
#define MFP_DIAGNOSTIC 0
#endif
int main(void)
{
#if MFP_DIAGNOSTIC == 1
    struct mp_arg a = MP_ARG(1.5);
    return (int)a.kind;
#elif MFP_DIAGNOSTIC == 2
    struct mfp_arg a = MFP_ARG("unbounded");
    return (int)a.kind;
#else
    unsigned calls = 0;
    struct mfp_span s = {NULL, 0};
    struct mfp_arg a = MFP_ARG((++calls, (uint32_t)42));
    struct mfp_arg b = MFP_ARG(s);
    return !(calls == 1 && a.kind == MFP_NUMBER && a.number == 42 &&
             b.kind == MFP_BYTES);
#endif
}
