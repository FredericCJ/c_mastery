/* Original intentionally flawed parser. Executed only in isolated ASan child.
   TF_REPAIRED is the separate corrected control, not a patch to the specimen. */
#include <stddef.h>
#include <stdint.h>
#ifndef TF_REPAIRED
#define TF_REPAIRED 0
#endif
volatile uint32_t tf_diagnostic_sink;

unsigned tf_teaching_bounds(const unsigned char *p, size_t n)
{
    if (n < 2 || n > 10 || p[0] != 1 || p[1] > 4)
    {
        return 1;
    }
    if (TF_REPAIRED && n != 2 + 2 * (size_t)p[1])
    {
        return 1;
    }
    uint32_t sum = 0;
    for (unsigned i = 0; i < p[1]; i++)
    {
        /* Missing exact payload admission exposes this specific one-byte read. */
        unsigned high = p[2 + 2 * i];
        unsigned low = p[3 + 2 * i];
        sum += high * 256 + low;
    }
    tf_diagnostic_sink = sum;
    return 0;
}
