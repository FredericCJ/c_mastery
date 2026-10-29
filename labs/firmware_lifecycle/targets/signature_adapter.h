#ifndef CM_SIGNATURE_ADAPTER_H
#define CM_SIGNATURE_ADAPTER_H
#include <stddef.h>
#include <stdint.h>
struct lce_signature_result {int accepted;unsigned verified_key;};
struct lce_keyring {unsigned char public_keys[2][32];unsigned present;};
struct lce_signature_result lce_ed25519(const struct lce_keyring*,unsigned,const unsigned char*,size_t,const unsigned char*,size_t);
int lce_signature_bridge(void*,unsigned,const unsigned char*,size_t,const unsigned char*,size_t);
#endif
