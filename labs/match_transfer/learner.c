#include "match.h"
/* Intentionally unsolved: prerequisite algorithms must be supplied separately. */
int mt_init(MTStream*s,const unsigned char*p,size_t n){(void)s;(void)p;(void)n;return AL_INVALID;}
int mt_feed(MTStream*s,const unsigned char*p,size_t n,ALPublish cb,void*c,MTReport*r){(void)s;(void)p;(void)n;(void)cb;(void)c;(void)r;return AL_INVALID;}
int mt_drain(MTStream*s,ALPublish cb,void*c,MTReport*r){(void)s;(void)cb;(void)c;(void)r;return AL_INVALID;}
