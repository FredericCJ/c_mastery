/* Intentionally unsolved learner scaffold. Reference implementation is separate. */
#include "checkpoint_depth.h"
uint32_t cp_crc32(const unsigned char *s,size_t n){(void)s;(void)n;return 0;}
enum cp_status cp_encode(const struct cp_message *m,unsigned char *s,size_t n,size_t *out){(void)m;(void)s;(void)n;(void)out;return CP_INVALID;}
void cp_parser_init(struct cp_parser *p){p->used=0;p->ready=0;}
enum cp_status cp_feed(struct cp_parser *p,const unsigned char *s,size_t n,size_t *used){(void)p;(void)s;(void)n;*used=0;return CP_INVALID;}
enum cp_status cp_peek(const struct cp_parser *p,struct cp_message *m){(void)p;(void)m;return CP_INVALID;}
void cp_queue_init(struct cp_queue *q){q->head=0;q->count=0;}
enum cp_status cp_deliver(struct cp_parser *p,struct cp_queue *q){(void)p;(void)q;return CP_INVALID;}
enum cp_status cp_take(struct cp_queue *q,struct cp_message *m){(void)q;(void)m;return CP_INVALID;}
