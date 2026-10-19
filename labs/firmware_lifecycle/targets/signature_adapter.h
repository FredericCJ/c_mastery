#ifndef CM_SIGNATURE_ADAPTER_H
#define CM_SIGNATURE_ADAPTER_H
#include <stddef.h>
#include <stdint.h>
typedef struct {int accepted;unsigned verified_key;} LCESignatureResult;
typedef struct {unsigned char public_keys[2][32];unsigned present;} LCEKeyring;
LCESignatureResult lce_ed25519(const LCEKeyring*,unsigned,const unsigned char*,size_t,const unsigned char*,size_t);
int lce_signature_bridge(void*,unsigned,const unsigned char*,size_t,const unsigned char*,size_t);
#endif
