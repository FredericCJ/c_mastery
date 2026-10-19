#ifndef LS_GENERATED_CODEC_H
#define LS_GENERATED_CODEC_H
#include "fields.h"
static int lsg_pack(const LsFields *v,uint16_t *out){uint32_t w;
if(!v || !out)return -1;
if(v->flags>LS_FLAGS_MAX)return -1;
if(v->length>LS_LENGTH_MAX)return -1;
if(v->tag>LS_TAG_MAX)return -1;
w=(v->flags*UINT32_C(256)) | (v->length*UINT32_C(8)) | (v->tag*UINT32_C(1));
*out=(uint16_t)w;return 0;}
static int lsg_unpack(uint16_t w,LsFields *out){LsFields v;
if(!out || ((uint32_t)w & ~LS_ALLOWED))return -1;
v.flags=((uint32_t)w & LS_FLAGS_MASK)>>LS_FLAGS_SHIFT;
v.length=((uint32_t)w & LS_LENGTH_MASK)>>LS_LENGTH_SHIFT;
v.tag=((uint32_t)w & LS_TAG_MASK)>>LS_TAG_SHIFT;
*out=v;return 0;}
#endif
