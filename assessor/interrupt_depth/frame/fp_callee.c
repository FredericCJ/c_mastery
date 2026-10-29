#include <stdint.h>
volatile float xf_fp_result;

__attribute__((noinline)) void xf_fp_callee(uint32_t seed)
{
    volatile float values[8];
    for (unsigned i = 0; i < 8; ++i)
    {
        values[i] = (float)(seed + i);
    }
    float value = values[0];
    for (unsigned i = 1; i < 8; ++i)
    {
        value = value * 1.25f + values[i];
    }
    xf_fp_result = value;
}
