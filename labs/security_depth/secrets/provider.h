#ifndef SS_PROVIDER_H
#define SS_PROVIDER_H
#include <stddef.h>
typedef struct { unsigned fetched,contexts,freed_fetches,freed_contexts,key_cleared; } SSLedger;
int ss_mac(const unsigned char key[4],const unsigned char *message,size_t length,
           unsigned fault,unsigned char out[32],SSLedger *ledger);
#endif
