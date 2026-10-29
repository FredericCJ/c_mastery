#include <stdint.h>
#include <stdio.h>
#include <stdckdint.h>

int main(void)
{
    uint16_t raw = 91, safe = 91, temp = 0;
    int a = 0, b = 0;
    bool overflow = ckd_add(&raw, UINT32_C(65535), UINT32_C(1));
    if (!overflow || raw != 0)
    {
        return 1;
    }
    if (!ckd_add(&temp, UINT32_C(65535), UINT32_C(1)))
    {
        safe = temp;
    }
    if (safe != 91)
    {
        return 1;
    }
    if (ckd_add(&temp, ++a, ++b) || temp != 2 || a != 1 || b != 1)
    {
        return 1;
    }
    puts(
        "EAT_C23 raw-overflow-writes-wrap staged-transaction-preserves evaluated-once");
    return 0;
}
