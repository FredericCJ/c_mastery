#ifndef CMASTERY_EFFECT_FOLLOW_H
#define CMASTERY_EFFECT_FOLLOW_H
#include "coroutines.h"
typedef int(*ch_answer_fn)(void*,unsigned,uint32_t*);
struct ch_handler{unsigned questions;ch_answer_fn answer;void*context;const struct ch_handler*parent;};
struct ch_effect{
 struct co_effect core;struct co_continuation token;struct co_allocator allocator;
 const struct ch_handler*lexical;unsigned dynamic,active;unsigned char*resource;
};
int ch_create(struct co_allocator,const struct ch_handler*,unsigned,const unsigned char[4],struct ch_effect**);
int ch_step(struct ch_effect*,const struct ch_handler*,uint32_t*);
int ch_clone(const struct ch_effect*,struct ch_effect**);
int ch_abort(struct ch_effect*);
int ch_destroy(struct ch_effect**);
#endif
