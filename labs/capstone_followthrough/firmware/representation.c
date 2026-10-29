#include "firmware.h"
#include "errors.h"
#include <stdio.h>

static uint32_t effect(uint32_t n, void *p)
{
    ++*(unsigned *)p;
    return n;
}

int main(void)
{
    unsigned effects = 0;
    er_result good = er_ok(17), bad = er_error(ER_IO, 5),
              mapped = er_map(bad, effect, &effects);
    if (good.tag != ER_OK || good.as.value != 17 || mapped.tag != ER_IO ||
        mapped.as.offset != 5 || effects)
        return 41;
    printf(
        "CAP_MCU_RESULT plain=%zu tagged=%zu errors-result=actual failure-callbacks=0 selected-target=plain\n",
        sizeof(enum cf_status), sizeof(er_result));
    return 0;
}
