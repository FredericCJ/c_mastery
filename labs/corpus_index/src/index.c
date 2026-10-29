#include "index.h"
/* Complete each mechanism from the public contract. */
enum ci_status ci_fenwick_init(struct ci_fenwick * arg0,const uint32_t * arg1,unsigned arg2){(void)arg0;(void)arg1;(void)arg2;return CI_INVALID;}
enum ci_status ci_fenwick_add(struct ci_fenwick * arg0,unsigned arg1,int64_t arg2){(void)arg0;(void)arg1;(void)arg2;return CI_INVALID;}
enum ci_status ci_fenwick_prefix(const struct ci_fenwick * arg0,unsigned arg1,uint32_t * arg2){(void)arg0;(void)arg1;(void)arg2;return CI_INVALID;}
enum ci_status ci_fenwick_select(const struct ci_fenwick * arg0,uint32_t arg1,unsigned * arg2){(void)arg0;(void)arg1;(void)arg2;return CI_INVALID;}
enum ci_status ci_fenwick_rescale(struct ci_fenwick * arg0,unsigned arg1){(void)arg0;(void)arg1;return CI_INVALID;}
enum ci_status ci_segment_init(struct ci_segment * arg0,const uint32_t * arg1,unsigned arg2){(void)arg0;(void)arg1;(void)arg2;return CI_INVALID;}
enum ci_status ci_segment_apply(struct ci_segment * arg0,unsigned arg1,unsigned arg2,uint32_t arg3,uint32_t arg4){(void)arg0;(void)arg1;(void)arg2;(void)arg3;(void)arg4;return CI_INVALID;}
enum ci_status ci_segment_query(const struct ci_segment * arg0,unsigned arg1,unsigned arg2,struct ci_aggregate * arg3){(void)arg0;(void)arg1;(void)arg2;(void)arg3;return CI_INVALID;}
enum ci_status ci_intervals_init(struct ci_intervals * arg0,unsigned arg1){(void)arg0;(void)arg1;return CI_INVALID;}
enum ci_status ci_intervals_insert(struct ci_intervals * arg0,struct ci_interval arg1){(void)arg0;(void)arg1;return CI_INVALID;}
enum ci_status ci_intervals_remove(struct ci_intervals * arg0,int32_t arg1,uint32_t arg2){(void)arg0;(void)arg1;(void)arg2;return CI_INVALID;}
enum ci_status ci_intervals_find(const struct ci_intervals * arg0,int32_t arg1,int32_t arg2,struct ci_interval * arg3){(void)arg0;(void)arg1;(void)arg2;(void)arg3;return CI_INVALID;}
enum ci_status ci_intervals_all(const struct ci_intervals * arg0,int32_t arg1,int32_t arg2,struct ci_interval * arg3,unsigned arg4,unsigned * arg5){(void)arg0;(void)arg1;(void)arg2;(void)arg3;(void)arg4;(void)arg5;return CI_INVALID;}
enum ci_status ci_suffix_build(struct ci_suffix * arg0,const unsigned char * arg1,unsigned arg2){(void)arg0;(void)arg1;(void)arg2;return CI_INVALID;}
enum ci_status ci_suffix_range(const struct ci_suffix * arg0,const unsigned char * arg1,unsigned arg2,unsigned * arg3,unsigned * arg4){(void)arg0;(void)arg1;(void)arg2;(void)arg3;(void)arg4;return CI_INVALID;}
enum ci_status ci_suffix_lce(const struct ci_suffix * arg0,unsigned arg1,unsigned arg2,unsigned * arg3){(void)arg0;(void)arg1;(void)arg2;(void)arg3;return CI_INVALID;}
enum ci_status ci_suffix_longest(const struct ci_suffix * arg0,unsigned arg1,unsigned * arg2,unsigned * arg3,unsigned * arg4){(void)arg0;(void)arg1;(void)arg2;(void)arg3;(void)arg4;return CI_INVALID;}
