/* Original parser and independent byte-level oracle; local synthetic data. */
#include "core.h"
#include <assert.h>

int LLVMFuzzerTestOneInput(const unsigned char *data, size_t n)
{
    unsigned payload = 99;
    size_t i;
    unsigned char x = 0;
    int expected = 0, ok;
    if (n >= 2 && data[0] <= 16 && n == (size_t)data[0] + 2)
    {
        for (i = 1; i + 1 < n; ++i)
            x ^= data[i];
        expected = x == data[n - 1];
    }
    ok = vf_frame(data, n, &payload);
    assert(ok == expected);
    if (ok)
        assert(payload == data[0]);
    else
        assert(payload == 99);
    return 0;
}
