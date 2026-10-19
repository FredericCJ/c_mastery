#include "raw_ring.h"
#include <errno.h>
#include <string.h>
int cm_ring_init(CMRawRing *r,unsigned n){(void)n;memset(r,0,sizeof *r);r->fd=-1;return -ENOSYS;}
int cm_ring_push(CMRawRing *r,const struct io_uring_sqe *s){(void)r;(void)s;return -ENOSYS;}
int cm_ring_submit(CMRawRing *r){(void)r;return -ENOSYS;}
int cm_ring_wait(CMRawRing *r,struct io_uring_cqe *c){(void)r;(void)c;return -ENOSYS;}
void cm_ring_close(CMRawRing *r){(void)r;}
