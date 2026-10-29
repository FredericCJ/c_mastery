#ifndef CMASTERY_ALGORITHM_LOOKUP_H
#define CMASTERY_ALGORITHM_LOOKUP_H
#include "containers.h"
#include "algorithms.h"
enum al_status {AL_OK,AL_INVALID,AL_SPACE,AL_MISSING,AL_OVERFLOW,AL_FAILED,AL_CANCELLED};
struct al_request {uint32_t key;unsigned start;};
struct al_probe {int index;unsigned used,visits[16];};
struct al_match_work {uint64_t comparisons;size_t required;};
struct al_stream {struct ad_matcher prefix;uint64_t consumed;unsigned empty,started;};
typedef int (*al_publish_fn)(void*,uint64_t); /* 0 accepted, 1 failed, 2 cancelled */
struct al_publication {size_t attempted,accepted;uint64_t comparisons;};
/* Trusted, live, disjoint arguments: the checked wrapper establishes bounds. */
void al_lookup_kernel(const struct cd_bucket*,unsigned,const struct al_request*,struct al_probe*);
int al_lookup_abi(const struct cd_bucket*,unsigned,const struct al_request*,struct al_probe*);
enum al_status al_lookup(const struct cd_hash*,uint32_t,int*,struct al_probe*);
enum al_status al_match(const unsigned char*,size_t,const unsigned char*,size_t,uint64_t*,size_t,size_t*,struct al_match_work*);
enum al_status al_publish(const unsigned char*,size_t,const unsigned char*,size_t,al_publish_fn,void*,struct al_publication*);
enum al_status al_stream_init(struct al_stream*,const unsigned char*,size_t);
enum al_status al_stream_feed(struct al_stream*,const unsigned char*,size_t,uint64_t*,size_t,size_t*);
enum al_status al_quadratic(unsigned,unsigned,struct al_probe*);
int al_failure(const char*,unsigned);
void al_emit_lookup(unsigned,uint32_t,const struct al_probe*);
void al_emit_match(unsigned,size_t,unsigned,size_t,const uint64_t*,size_t,uint64_t);
void al_emit_stream(unsigned,size_t,unsigned,size_t,size_t,const uint64_t*,size_t);
int al_test(unsigned);
#endif
