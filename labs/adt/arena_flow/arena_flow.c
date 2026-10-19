#include "arena_flow.h"
AdfStatus adf_arena(const ADNode *n,size_t count,AdfArena *out){(void)n;(void)count;(void)out;return ADF_INVALID;}
AdfStatus adf_root(const AdfArena *a,ADNodeIndex r,uint32_t *o){(void)a;(void)r;(void)o;return ADF_INVALID;}
AdfStatus adf_start(const AdfNode *n,unsigned c,AdfSession *o){(void)n;(void)c;(void)o;return ADF_INVALID;}
AdfStatus adf_step(AdfSession *s,unsigned b,AdfResult *o){(void)s;(void)b;(void)o;return ADF_INVALID;}
AdfStatus adf_boolean(const AdfBoolean *n,unsigned c,unsigned b,unsigned *o){(void)n;(void)c;(void)b;(void)o;return ADF_INVALID;}
