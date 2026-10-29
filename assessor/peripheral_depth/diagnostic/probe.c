#include "diagnostic.h"
#include <stdio.h>

int main(void)
{
    for (unsigned seq = 0; seq <= 65535; ++seq)
    {
        struct dg_message m = {(uint16_t)seq, (uint16_t)(seq % 33), {0}},
                          out = {0};
        uint8_t wire[36];
        size_t n = 0;
        for (unsigned i = 0; i < m.length; ++i)
            m.payload[i] = (uint8_t)((seq + i * 17u) % 256u);
        int rc = dg_encode(&m, wire, sizeof wire, &n);
        printf("%u %u %d ", seq, m.length, rc);
        if (!rc)
        {
            for (size_t i = 0; i < n; ++i)
                printf("%02x", wire[i]);
            rc = dg_decode(wire, n, &out);
            printf(" %d %u %u", rc, out.sequence, out.length);
        }
        else
            printf("-");
        puts("");
    }
    return 0;
}
