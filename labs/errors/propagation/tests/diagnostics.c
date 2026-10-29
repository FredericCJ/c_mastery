#include "propagation.h"
#include "propagation_macro.h"
#ifndef EPR_DIAGNOSTIC
#define EPR_DIAGNOSTIC 0
#endif
int main(void)
{
    er_result out = er_ok(0);
#if EPR_DIAGNOSTIC == 1
    er_result value = EPR_TRY(fresh, er_ok(7), &out, failed);
    (void)value;
#elif EPR_DIAGNOSTIC == 2
    uint32_t temporary = 7;
    EPR_TRY(temporary, er_ok(temporary), &out, failed);
    (void)temporary;
#else
    uint32_t temporary = 7;
    if (temporary)
    {
        EPR_TRY(fresh, er_ok(temporary), &out, failed);
    }
    else
    {
        return 2;
    }
    if (out.tag != ER_OK || out.as.value != 7)
    {
        return 3;
    }
    return 0;
#endif
failed:
    return 1;
}
