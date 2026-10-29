#include "storage.h"
/* Deliberately incomplete: compilation is not semantic acceptance. */
void ss_skip_init(struct ss_skip * arg0){(void)arg0;*arg0=(struct ss_skip){0};}
unsigned ss_height(uint32_t arg0,unsigned arg1){(void)arg0;(void)arg1;return 0;}
int ss_skip_put(struct ss_skip * arg0,uint32_t arg1,int32_t arg2,uint32_t arg3,unsigned arg4){(void)arg0;(void)arg1;(void)arg2;(void)arg3;(void)arg4;return SS_INVALID;}
int ss_skip_get(const struct ss_skip * arg0,uint32_t arg1,int32_t * arg2,unsigned * arg3){(void)arg0;(void)arg1;(void)arg2;(void)arg3;return SS_INVALID;}
int ss_skip_erase(struct ss_skip * arg0,uint32_t arg1){(void)arg0;(void)arg1;return SS_INVALID;}
int ss_skip_scan(const struct ss_skip * arg0,uint32_t arg1,uint32_t arg2,uint32_t * arg3,int32_t * arg4,size_t arg5,size_t * arg6){(void)arg0;(void)arg1;(void)arg2;(void)arg3;(void)arg4;(void)arg5;(void)arg6;return SS_INVALID;}
int ss_random(uint32_t * arg0,uint32_t arg1,unsigned arg2,uint32_t * arg3){(void)arg0;(void)arg1;(void)arg2;(void)arg3;return SS_INVALID;}
uint32_t ss_random_advance(uint32_t arg0,uint32_t arg1){(void)arg0;(void)arg1;return 0;}
void ss_tree_init(struct ss_tree * arg0){(void)arg0;*arg0=(struct ss_tree){0};}
int ss_tree_snapshot(struct ss_tree * arg0,struct ss_version * arg1){(void)arg0;(void)arg1;return SS_INVALID;}
int ss_tree_release(struct ss_tree * arg0,struct ss_version arg1){(void)arg0;(void)arg1;return SS_INVALID;}
int ss_tree_get(const struct ss_tree * arg0,const struct ss_version * arg1,uint32_t arg2,int32_t * arg3){(void)arg0;(void)arg1;(void)arg2;(void)arg3;return SS_INVALID;}
int ss_tree_put(struct ss_tree * arg0,uint32_t arg1,int32_t arg2,unsigned arg3){(void)arg0;(void)arg1;(void)arg2;(void)arg3;return SS_INVALID;}
int ss_tree_erase(struct ss_tree * arg0,uint32_t arg1,unsigned arg2){(void)arg0;(void)arg1;(void)arg2;return SS_INVALID;}
int ss_tree_collect(struct ss_tree * arg0,unsigned * arg1){(void)arg0;(void)arg1;return SS_INVALID;}
int ss_tree_vacuum(struct ss_tree * arg0,unsigned arg1){(void)arg0;(void)arg1;return SS_INVALID;}
void ss_lsm_init(struct ss_lsm * arg0){(void)arg0;*arg0=(struct ss_lsm){0};}
int ss_lsm_write(struct ss_lsm * arg0,uint32_t arg1,int32_t arg2,unsigned arg3){(void)arg0;(void)arg1;(void)arg2;(void)arg3;return SS_INVALID;}
int ss_lsm_get(const struct ss_lsm * arg0,uint32_t arg1,int32_t * arg2,unsigned * arg3){(void)arg0;(void)arg1;(void)arg2;(void)arg3;return SS_INVALID;}
int ss_lsm_flush(struct ss_lsm * arg0,unsigned arg1){(void)arg0;(void)arg1;return SS_INVALID;}
int ss_lsm_compact(struct ss_lsm * arg0,unsigned arg1,unsigned arg2,unsigned arg3){(void)arg0;(void)arg1;(void)arg2;(void)arg3;return SS_INVALID;}
