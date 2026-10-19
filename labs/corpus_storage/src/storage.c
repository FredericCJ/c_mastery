#include "storage.h"
/* Deliberately incomplete: compilation is not semantic acceptance. */
void ss_skip_init(SsSkip * arg0){(void)arg0;*arg0=(SsSkip){0};}
unsigned ss_height(uint32_t arg0,unsigned arg1){(void)arg0;(void)arg1;return 0;}
int ss_skip_put(SsSkip * arg0,uint32_t arg1,int32_t arg2,uint32_t arg3,unsigned arg4){(void)arg0;(void)arg1;(void)arg2;(void)arg3;(void)arg4;return SS_INVALID;}
int ss_skip_get(const SsSkip * arg0,uint32_t arg1,int32_t * arg2,unsigned * arg3){(void)arg0;(void)arg1;(void)arg2;(void)arg3;return SS_INVALID;}
int ss_skip_erase(SsSkip * arg0,uint32_t arg1){(void)arg0;(void)arg1;return SS_INVALID;}
int ss_skip_scan(const SsSkip * arg0,uint32_t arg1,uint32_t arg2,uint32_t * arg3,int32_t * arg4,size_t arg5,size_t * arg6){(void)arg0;(void)arg1;(void)arg2;(void)arg3;(void)arg4;(void)arg5;(void)arg6;return SS_INVALID;}
int ss_random(uint32_t * arg0,uint32_t arg1,unsigned arg2,uint32_t * arg3){(void)arg0;(void)arg1;(void)arg2;(void)arg3;return SS_INVALID;}
uint32_t ss_random_advance(uint32_t arg0,uint32_t arg1){(void)arg0;(void)arg1;return 0;}
void ss_tree_init(SsTree * arg0){(void)arg0;*arg0=(SsTree){0};}
int ss_tree_snapshot(SsTree * arg0,SsVersion * arg1){(void)arg0;(void)arg1;return SS_INVALID;}
int ss_tree_release(SsTree * arg0,SsVersion arg1){(void)arg0;(void)arg1;return SS_INVALID;}
int ss_tree_get(const SsTree * arg0,const SsVersion * arg1,uint32_t arg2,int32_t * arg3){(void)arg0;(void)arg1;(void)arg2;(void)arg3;return SS_INVALID;}
int ss_tree_put(SsTree * arg0,uint32_t arg1,int32_t arg2,unsigned arg3){(void)arg0;(void)arg1;(void)arg2;(void)arg3;return SS_INVALID;}
int ss_tree_erase(SsTree * arg0,uint32_t arg1,unsigned arg2){(void)arg0;(void)arg1;(void)arg2;return SS_INVALID;}
int ss_tree_collect(SsTree * arg0,unsigned * arg1){(void)arg0;(void)arg1;return SS_INVALID;}
int ss_tree_vacuum(SsTree * arg0,unsigned arg1){(void)arg0;(void)arg1;return SS_INVALID;}
void ss_lsm_init(SsLsm * arg0){(void)arg0;*arg0=(SsLsm){0};}
int ss_lsm_write(SsLsm * arg0,uint32_t arg1,int32_t arg2,unsigned arg3){(void)arg0;(void)arg1;(void)arg2;(void)arg3;return SS_INVALID;}
int ss_lsm_get(const SsLsm * arg0,uint32_t arg1,int32_t * arg2,unsigned * arg3){(void)arg0;(void)arg1;(void)arg2;(void)arg3;return SS_INVALID;}
int ss_lsm_flush(SsLsm * arg0,unsigned arg1){(void)arg0;(void)arg1;return SS_INVALID;}
int ss_lsm_compact(SsLsm * arg0,unsigned arg1,unsigned arg2,unsigned arg3){(void)arg0;(void)arg1;(void)arg2;(void)arg3;return SS_INVALID;}
