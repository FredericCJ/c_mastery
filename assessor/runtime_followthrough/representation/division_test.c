#include <stdint.h>
#include <stdio.h>
void rv_signed_words(const uint32_t *, uint32_t *);

int main(void)
{
    const int64_t values[] = {
        INT64_MIN, INT64_MIN + 1, -INT64_C(4294967297), -65537,   -7, -1, 0, 1,
        7,         65537,         INT64_C(4294967297),  INT64_MAX};
    unsigned count = 0;
    for (unsigned i = 0; i < 12; i++)
        for (unsigned j = 0; j < 12; j++)
        {
            int64_t a = values[i], b = values[j];
            if (!b || (a == INT64_MIN && b == -1))
                continue;
            uint64_t ua = (uint64_t)a, ub = (uint64_t)b;
            uint32_t in[4] = {(uint32_t)ua, (uint32_t)(ua >> 32), (uint32_t)ub,
                              (uint32_t)(ub >> 32)},
                     out[4];
            rv_signed_words(in, out);
            uint64_t q = (uint64_t)out[0] | ((uint64_t)out[1] << 32),
                     r = (uint64_t)out[2] | ((uint64_t)out[3] << 32);
            if (q != (uint64_t)(a / b) || r != (uint64_t)(a % b))
                return 1;
            count++;
        }
    printf("RV_SIGNED_HELPER_PASS cases=%u native_division_vs_original_limb\n",
           count);
    return 0;
}
