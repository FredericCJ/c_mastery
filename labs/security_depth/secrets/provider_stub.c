#include "provider.h"

int ss_mac(const unsigned char key[4], const unsigned char *m, size_t n,
           unsigned f, unsigned char out[32], struct ss_ledger *l)
{
    (void)key;
    (void)m;
    (void)n;
    (void)f;
    (void)out;
    (void)l;
    return -1;
}
