#include "raw_ring.h"
#include <errno.h>
#include <string.h>
int cm_ring_init(struct cm_raw_ring *r,unsigned n){(void)n;memset(r,0,sizeof *r);r->fd=-1;return -ENOSYS;}
int cm_ring_push(struct cm_raw_ring *r,const struct io_uring_sqe *s){(void)r;(void)s;return -ENOSYS;}
int cm_ring_submit(struct cm_raw_ring *r){(void)r;return -ENOSYS;}
int cm_ring_wait(struct cm_raw_ring *r,struct io_uring_cqe *c){(void)r;(void)c;return -ENOSYS;}
void cm_ring_close(struct cm_raw_ring *r){(void)r;}
