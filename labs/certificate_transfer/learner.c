/* Deliberately incomplete learner artifact. Keep author solutions separate. */
#include "transfer.h"
void ct_init(CTStream*s){if(s)*s=(CTStream){0};}
enum CTStatus ct_feed(CTStream*s,const unsigned char*p,size_t n,unsigned f,const AGCertificate*c,CDCSR*out){(void)s;(void)p;(void)n;(void)f;(void)c;(void)out;return CT_INVALID;}
int ct_tree(const ATCertificate*c,unsigned p,CTWorkspace*w,unsigned n,int*out){(void)c;(void)p;(void)w;(void)n;(void)out;return ALGO_BAD;}
