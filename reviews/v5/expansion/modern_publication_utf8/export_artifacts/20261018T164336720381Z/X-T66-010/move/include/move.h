#ifndef CMASTERY_OWNERSHIP_MOVE_DEPTH_H
#define CMASTERY_OWNERSHIP_MOVE_DEPTH_H
#include "ownership.h"
typedef struct OmmValue OmmValue;
struct OmmValue { int32_t small[4];int32_t *data;size_t length;V4Allocator allocator;const OmmValue *self;unsigned live; };
OwStatus omm_init(OmmValue *,V4Allocator,const int32_t *,size_t);
OwStatus omm_move(OmmValue *,OmmValue *);
OwStatus omm_read(const OmmValue *,size_t,int32_t *);
OwStatus omm_consume(OmmValue *);
/* The registry representation is incomplete; ticket values remain copyable. */
typedef struct OmmRegistry OmmRegistry;
typedef struct {OmmRegistry *owner;unsigned slot;uint32_t generation;} OmmToken;
typedef struct {void *context;void *(*take)(void *,size_t);void (*release)(void *,void *);} OmmMemory;
OwStatus omm_registry_create(OmmMemory,V4Allocator,uint32_t,OmmRegistry **);
OwStatus omm_registry_destroy(OmmRegistry **);
OwStatus omm_token_open(OmmRegistry *,const int32_t *,size_t,OmmToken *);
OwStatus omm_token_move(OmmRegistry *,OmmToken *,OmmToken *);
OwStatus omm_token_read(OmmRegistry *,OmmToken,size_t,int32_t *);
OwStatus omm_token_consume(OmmRegistry *,OmmToken *);
#endif
