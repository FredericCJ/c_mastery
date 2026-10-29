#ifndef CMASTERY_EFFECTS_DEPTH_H
#define CMASTERY_EFFECTS_DEPTH_H
#include <stddef.h>
#include <stdint.h>
enum ef_status {EF_OK,EF_INVALID,EF_FULL,EF_BUSY,EF_NOMEM,EF_INCOMPLETE,EF_FAILED};
struct ef_allocator {void*context;unsigned char*(*take)(void*,size_t);void(*release)(void*,unsigned char*);};
struct ef_cache {struct ef_allocator allocator;unsigned char*scratch;uint32_t generation,borrows;unsigned failed,permanent;};
/* Initialise generation1, zero scratch/borrows/failed. Byte storage32. */
enum ef_status ef_checksum(const unsigned char*,size_t,uint32_t*);
enum ef_status ef_cached(struct ef_cache*,const unsigned char*,size_t,uint32_t*);
enum ef_status ef_cache_reset(struct ef_cache*);
enum ef_status ef_cache_borrow(struct ef_cache*,const unsigned char**);
enum ef_status ef_cache_return(struct ef_cache*);
struct ef_pool {unsigned char bytes[16];uint32_t used,requests;};
enum ef_status ef_pool_take(struct ef_pool*,unsigned char**);
enum ef_status ef_pool_return(struct ef_pool*,unsigned);
struct ef_parser {unsigned char scratch[32];size_t cursor;uint32_t seed,generation;unsigned borrowed;};
struct ef_view {struct ef_parser*owner;uint32_t generation;size_t length;const unsigned char*data;};
enum ef_status ef_parse(struct ef_parser*,const unsigned char*,size_t,struct ef_view*);
enum ef_status ef_view_release(struct ef_view*);
enum ef_status ef_parser_reset(struct ef_parser*,uint32_t);
uint32_t ef_random(struct ef_parser*);
typedef int(*ef_callback_fn)(void*,unsigned char*,size_t);
enum ef_status ef_attempt(ef_callback_fn,void*,unsigned nonblocking,unsigned attempts,unsigned*used);
struct ef_record {int key;unsigned identity;unsigned char payload[32];};
enum ef_status ef_sort(int*,size_t);
enum ef_status ef_sort_records(struct ef_record*,size_t);
#endif
