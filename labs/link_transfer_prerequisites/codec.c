/* Compiling unfinished prerequisite adapter. Learners replace this with their
 * actual completed original fragment codec, never the assessor implementation. */
#include "com_edu7.h"
int edu_fragment_encode(const EduFragment *f,void *bytes,size_t *n){(void)f;(void)bytes;(void)n;return -1;}
int edu_fragment_decode(const void *bytes,size_t n,EduFragment *f){(void)bytes;(void)n;(void)f;return -1;}
