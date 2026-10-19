#include "signature_adapter.h"
LCESignatureResult lce_ed25519(const LCEKeyring*k,unsigned id,const unsigned char*p,size_t n,const unsigned char*s,size_t z){(void)k;(void)id;(void)p;(void)n;(void)s;(void)z;LCESignatureResult r={0,0};return r;}
int lce_signature_bridge(void*c,unsigned id,const unsigned char*p,size_t n,const unsigned char*s,size_t z){(void)c;(void)id;(void)p;(void)n;(void)s;(void)z;return 0;}
