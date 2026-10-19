#ifndef CMASTERY_EFFECT_FOLLOW_H
#define CMASTERY_EFFECT_FOLLOW_H
#include "coroutines.h"
typedef int(*CHAnswer)(void*,unsigned,uint32_t*);
typedef struct CHHandler{unsigned questions;CHAnswer answer;void*context;const struct CHHandler*parent;} CHHandler;
typedef struct{
 COEffect core;COContinuation token;COAllocator allocator;
 const CHHandler*lexical;unsigned dynamic,active;unsigned char*resource;
} CHEffect;
int ch_create(COAllocator,const CHHandler*,unsigned,const unsigned char[4],CHEffect**);
int ch_step(CHEffect*,const CHHandler*,uint32_t*);
int ch_clone(const CHEffect*,CHEffect**);
int ch_abort(CHEffect*);
int ch_destroy(CHEffect**);
#endif
