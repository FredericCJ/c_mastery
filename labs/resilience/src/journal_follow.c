#include "journal.h"
int rzj_open(RzjIO i,unsigned w,RzjJournal *o){(void)i;(void)w;(void)o;return RZ_INVALID;}
int rzj_issue(RzjJournal *j,uint64_t n,uint64_t *o){(void)j;(void)n;(void)o;return RZ_INVALID;}
int rzj_rotate(RzjJournal *j,uint64_t n,uint64_t *o){(void)j;(void)n;(void)o;return RZ_INVALID;}
int rzj_record(const RzjJournal *j,unsigned n,RzjRecord *o){(void)j;(void)n;(void)o;return RZ_INVALID;}
int rzj_apply(RzjJournal *j,uint64_t n,RzjRequest r,int *o,bool *d){(void)j;(void)n;(void)r;(void)o;(void)d;return RZ_INVALID;}
int rzj_mirror(const RzjJournal *j,RzjIO i){(void)j;(void)i;return RZ_INVALID;}
int rzj_replica_read(RzjIO i,unsigned w,RzjReplica *o){(void)i;(void)w;(void)o;return RZ_INVALID;}
