#include "target.h"
#include <stdio.h>

int main(void)
{
    sh_failure_record = 0;
    sh_record_failure();
    if (sh_failure_record != UINT32_C(0x53484746))
    {
        fputs("SH_ASSERT:bounded_failure_record\n", stderr);
        return 41;
    }
    puts("OBS bounded_failure_record no allocation logging retry");
    return 0;
}
