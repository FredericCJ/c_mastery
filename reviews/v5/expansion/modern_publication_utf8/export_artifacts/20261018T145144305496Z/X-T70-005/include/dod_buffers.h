#ifndef CMASTERY_DOD_BUFFERS_H
#define CMASTERY_DOD_BUFFERS_H
#include "dod.h"
typedef struct {unsigned char bytes[16];size_t length;} DInline;
int d_inline_append(DInline *,const unsigned char *,size_t);
int d_inline_truncate(DInline *,size_t);
typedef struct {unsigned char rows[4][128];size_t used[4],live,peak,attempts,releases,fail_at;} DQuota;
typedef struct {DQuota *owner;unsigned char *data;size_t length;} DZero;
void d_zero_init(DZero *,DQuota *);
int d_zero_append(DZero *,const unsigned char *,size_t);
int d_zero_pair_append(DZero *,const unsigned char *,size_t,DZero *,const unsigned char *,size_t);
int d_zero_move(DZero *,DZero *);
void d_zero_reset(DZero *);
uint32_t d_swar16_signed(uint32_t,uint32_t);
#endif
