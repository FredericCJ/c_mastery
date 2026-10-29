#include "journal.h"
int rzj_open(struct rzj_io i,unsigned w,struct rzj_journal *o){(void)i;(void)w;(void)o;return RZ_INVALID;}
int rzj_issue(struct rzj_journal *j,uint64_t n,uint64_t *o){(void)j;(void)n;(void)o;return RZ_INVALID;}
int rzj_rotate(struct rzj_journal *j,uint64_t n,uint64_t *o){(void)j;(void)n;(void)o;return RZ_INVALID;}
int rzj_record(const struct rzj_journal *j,unsigned n,struct rzj_record *o){(void)j;(void)n;(void)o;return RZ_INVALID;}
int rzj_apply(struct rzj_journal *j,uint64_t n,struct rzj_request r,int *o,bool *d){(void)j;(void)n;(void)r;(void)o;(void)d;return RZ_INVALID;}
int rzj_mirror(const struct rzj_journal *j,struct rzj_io i){(void)j;(void)i;return RZ_INVALID;}
int rzj_replica_read(struct rzj_io i,unsigned w,struct rzj_replica *o){(void)i;(void)w;(void)o;return RZ_INVALID;}
