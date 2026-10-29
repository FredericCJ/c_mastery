#include "stream.h"
void pt_session_init(struct pt_session *s){(void)s;}
enum cp_status pt_session_open(struct pt_session *s){(void)s;return CP_INVALID;}
void pt_session_close(struct pt_session *s){(void)s;}
enum cp_status pt_session_feed(struct pt_session *s,const unsigned char *b,size_t n,size_t *u){(void)s;(void)b;(void)n;(void)u;return CP_INVALID;}
enum cp_status pt_session_take(struct pt_session *s,struct cp_message *m){(void)s;(void)m;return CP_INVALID;}
