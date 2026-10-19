#include "auth.h"
int sx_auth_encode(uint64_t id,const B5Redo*r,unsigned char m[32]){(void)id;(void)r;(void)m;return B5_UNIMPLEMENTED;}
int sx_auth_recover(uint64_t id,uint32_t g,uint32_t b,const SxAuthenticated*r,size_t n,SxMac f,void*c,uint32_t*out){(void)id;(void)g;(void)b;(void)r;(void)n;(void)f;(void)c;(void)out;return B5_UNIMPLEMENTED;}
