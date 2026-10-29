#ifndef SS_PROVIDER_H
#define SS_PROVIDER_H
#include <stddef.h>

struct ss_ledger
{
    unsigned fetched, contexts, freed_fetches, freed_contexts, key_cleared;
};

int ss_mac(const unsigned char key[4], const unsigned char *message,
           size_t length, unsigned fault, unsigned char out[32],
           struct ss_ledger *ledger);
#endif
