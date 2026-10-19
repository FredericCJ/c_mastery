/* Intentionally unsolved learner scaffold. Reference implementation is separate. */
#include "checkpoint_depth.h"
uint32_t cp_crc32(const unsigned char *s,size_t n){(void)s;(void)n;return 0;}
CPStatus cp_encode(const CPMessage *m,unsigned char *s,size_t n,size_t *out){(void)m;(void)s;(void)n;(void)out;return CP_INVALID;}
void cp_parser_init(CPParser *p){p->used=0;p->ready=0;}
CPStatus cp_feed(CPParser *p,const unsigned char *s,size_t n,size_t *used){(void)p;(void)s;(void)n;*used=0;return CP_INVALID;}
CPStatus cp_peek(const CPParser *p,CPMessage *m){(void)p;(void)m;return CP_INVALID;}
void cp_queue_init(CPQueue *q){q->head=0;q->count=0;}
CPStatus cp_deliver(CPParser *p,CPQueue *q){(void)p;(void)q;return CP_INVALID;}
CPStatus cp_take(CPQueue *q,CPMessage *m){(void)q;(void)m;return CP_INVALID;}
