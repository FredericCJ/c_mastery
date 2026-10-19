#include "index.h"
/* Complete each mechanism from the public contract. */
CiStatus ci_fenwick_init(CiFenwick * arg0,const uint32_t * arg1,unsigned arg2){(void)arg0;(void)arg1;(void)arg2;return CI_INVALID;}
CiStatus ci_fenwick_add(CiFenwick * arg0,unsigned arg1,int64_t arg2){(void)arg0;(void)arg1;(void)arg2;return CI_INVALID;}
CiStatus ci_fenwick_prefix(const CiFenwick * arg0,unsigned arg1,uint32_t * arg2){(void)arg0;(void)arg1;(void)arg2;return CI_INVALID;}
CiStatus ci_fenwick_select(const CiFenwick * arg0,uint32_t arg1,unsigned * arg2){(void)arg0;(void)arg1;(void)arg2;return CI_INVALID;}
CiStatus ci_fenwick_rescale(CiFenwick * arg0,unsigned arg1){(void)arg0;(void)arg1;return CI_INVALID;}
CiStatus ci_segment_init(CiSegment * arg0,const uint32_t * arg1,unsigned arg2){(void)arg0;(void)arg1;(void)arg2;return CI_INVALID;}
CiStatus ci_segment_apply(CiSegment * arg0,unsigned arg1,unsigned arg2,uint32_t arg3,uint32_t arg4){(void)arg0;(void)arg1;(void)arg2;(void)arg3;(void)arg4;return CI_INVALID;}
CiStatus ci_segment_query(const CiSegment * arg0,unsigned arg1,unsigned arg2,CiAggregate * arg3){(void)arg0;(void)arg1;(void)arg2;(void)arg3;return CI_INVALID;}
CiStatus ci_intervals_init(CiIntervals * arg0,unsigned arg1){(void)arg0;(void)arg1;return CI_INVALID;}
CiStatus ci_intervals_insert(CiIntervals * arg0,CiInterval arg1){(void)arg0;(void)arg1;return CI_INVALID;}
CiStatus ci_intervals_remove(CiIntervals * arg0,int32_t arg1,uint32_t arg2){(void)arg0;(void)arg1;(void)arg2;return CI_INVALID;}
CiStatus ci_intervals_find(const CiIntervals * arg0,int32_t arg1,int32_t arg2,CiInterval * arg3){(void)arg0;(void)arg1;(void)arg2;(void)arg3;return CI_INVALID;}
CiStatus ci_intervals_all(const CiIntervals * arg0,int32_t arg1,int32_t arg2,CiInterval * arg3,unsigned arg4,unsigned * arg5){(void)arg0;(void)arg1;(void)arg2;(void)arg3;(void)arg4;(void)arg5;return CI_INVALID;}
CiStatus ci_suffix_build(CiSuffix * arg0,const unsigned char * arg1,unsigned arg2){(void)arg0;(void)arg1;(void)arg2;return CI_INVALID;}
CiStatus ci_suffix_range(const CiSuffix * arg0,const unsigned char * arg1,unsigned arg2,unsigned * arg3,unsigned * arg4){(void)arg0;(void)arg1;(void)arg2;(void)arg3;(void)arg4;return CI_INVALID;}
CiStatus ci_suffix_lce(const CiSuffix * arg0,unsigned arg1,unsigned arg2,unsigned * arg3){(void)arg0;(void)arg1;(void)arg2;(void)arg3;return CI_INVALID;}
CiStatus ci_suffix_longest(const CiSuffix * arg0,unsigned arg1,unsigned * arg2,unsigned * arg3,unsigned * arg4){(void)arg0;(void)arg1;(void)arg2;(void)arg3;(void)arg4;return CI_INVALID;}
