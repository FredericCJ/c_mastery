#include "match.h"
/* Intentionally unsolved: prerequisite algorithms must be supplied separately. */
int mt_init(struct mt_stream*s,const unsigned char*p,size_t n){(void)s;(void)p;(void)n;return AL_INVALID;}
int mt_feed(struct mt_stream*s,const unsigned char*p,size_t n,al_publish_fn cb,void*c,struct mt_report*r){(void)s;(void)p;(void)n;(void)cb;(void)c;(void)r;return AL_INVALID;}
int mt_drain(struct mt_stream*s,al_publish_fn cb,void*c,struct mt_report*r){(void)s;(void)cb;(void)c;(void)r;return AL_INVALID;}
