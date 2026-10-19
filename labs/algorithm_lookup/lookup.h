#ifndef CMASTERY_ALGORITHM_LOOKUP_H
#define CMASTERY_ALGORITHM_LOOKUP_H
#include "containers.h"
#include "algorithms.h"
enum ALStatus {AL_OK,AL_INVALID,AL_SPACE,AL_MISSING,AL_OVERFLOW,AL_FAILED,AL_CANCELLED};
typedef struct {uint32_t key;unsigned start;} ALRequest;
typedef struct {int index;unsigned used,visits[16];} ALProbe;
typedef struct {uint64_t comparisons;size_t required;} ALMatchWork;
typedef struct {ADMatcher prefix;uint64_t consumed;unsigned empty,started;} ALStream;
typedef int (*ALPublish)(void*,uint64_t); /* 0 accepted, 1 failed, 2 cancelled */
typedef struct {size_t attempted,accepted;uint64_t comparisons;} ALPublication;
/* Trusted, live, disjoint arguments: the checked wrapper establishes bounds. */
void al_lookup_kernel(const CDBucket*,unsigned,const ALRequest*,ALProbe*);
int al_lookup_abi(const CDBucket*,unsigned,const ALRequest*,ALProbe*);
enum ALStatus al_lookup(const CDHash*,uint32_t,int*,ALProbe*);
enum ALStatus al_match(const unsigned char*,size_t,const unsigned char*,size_t,uint64_t*,size_t,size_t*,ALMatchWork*);
enum ALStatus al_publish(const unsigned char*,size_t,const unsigned char*,size_t,ALPublish,void*,ALPublication*);
enum ALStatus al_stream_init(ALStream*,const unsigned char*,size_t);
enum ALStatus al_stream_feed(ALStream*,const unsigned char*,size_t,uint64_t*,size_t,size_t*);
enum ALStatus al_quadratic(unsigned,unsigned,ALProbe*);
int al_failure(const char*,unsigned);
void al_emit_lookup(unsigned,uint32_t,const ALProbe*);
void al_emit_match(unsigned,size_t,unsigned,size_t,const uint64_t*,size_t,uint64_t);
void al_emit_stream(unsigned,size_t,unsigned,size_t,size_t,const uint64_t*,size_t);
int al_test(unsigned);
#endif
