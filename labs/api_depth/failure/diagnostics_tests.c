#include "diagnostics.h"
#include <stdio.h>

int main(void)
{
    unsigned evaluations = 0;
    AE_TRACE(++evaluations);
    AE_DIAG(1, ++evaluations);
    AE_DIAG(3, ++evaluations);
    unsigned first = evaluations;
    ae_threshold = 0;
    AE_DIAG(1, ++evaluations);
    if (first != AE_DEV + AE_INTEGRATOR ||
        evaluations != AE_DEV + 2 * AE_INTEGRATOR ||
        ae_diagnostic_events(0) != AE_DEV ||
        ae_diagnostic_events(1) != 2 * AE_INTEGRATOR ||
        ae_production_capacity(4) != AE_INVALID ||
        ae_production_capacity(3) != AE_OK)
    {
        fprintf(stderr, "DIAGNOSTICS-CHECK\n");
        return 1;
    }
    printf("DIAGNOSTICS-PASS:dev=%d;integrator=%d;evaluations=%u;\n", AE_DEV,
           AE_INTEGRATOR, evaluations);
    return 0;
}
