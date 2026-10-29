#include "cmx.h"

/* Replace these with your prior scalar CMX artifact; no assessor is exported.
 */
int cmx_spsc_init(cmx_spsc *q, unsigned n)
{
    (void)q;
    (void)n;
    return 0;
}

enum cmx_result cmx_spsc_push(cmx_spsc *q, uint32_t v)
{
    (void)q;
    (void)v;
    return CMX_UNIMPLEMENTED;
}

enum cmx_result cmx_spsc_pop(cmx_spsc *q, uint32_t *v)
{
    (void)q;
    (void)v;
    return CMX_UNIMPLEMENTED;
}
