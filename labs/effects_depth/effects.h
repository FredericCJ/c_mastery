#ifndef CMASTERY_EFFECTS_DEPTH_H
#define CMASTERY_EFFECTS_DEPTH_H
#include <stddef.h>
#include <stdint.h>
enum EFStatus {EF_OK,EF_INVALID,EF_FULL,EF_BUSY,EF_NOMEM,EF_INCOMPLETE,EF_FAILED};
typedef struct {void*context;unsigned char*(*take)(void*,size_t);void(*release)(void*,unsigned char*);} EFAllocator;
typedef struct {EFAllocator allocator;unsigned char*scratch;uint32_t generation,borrows;unsigned failed,permanent;} EFCache;
/* Initialise generation1, zero scratch/borrows/failed. Byte storage32. */
enum EFStatus ef_checksum(const unsigned char*,size_t,uint32_t*);
enum EFStatus ef_cached(EFCache*,const unsigned char*,size_t,uint32_t*);
enum EFStatus ef_cache_reset(EFCache*);
enum EFStatus ef_cache_borrow(EFCache*,const unsigned char**);
enum EFStatus ef_cache_return(EFCache*);
typedef struct {unsigned char bytes[16];uint32_t used,requests;} EFPool;
enum EFStatus ef_pool_take(EFPool*,unsigned char**);
enum EFStatus ef_pool_return(EFPool*,unsigned);
typedef struct {unsigned char scratch[32];size_t cursor;uint32_t seed,generation;unsigned borrowed;} EFParser;
typedef struct {EFParser*owner;uint32_t generation;size_t length;const unsigned char*data;} EFView;
enum EFStatus ef_parse(EFParser*,const unsigned char*,size_t,EFView*);
enum EFStatus ef_view_release(EFView*);
enum EFStatus ef_parser_reset(EFParser*,uint32_t);
uint32_t ef_random(EFParser*);
typedef int(*EFCallback)(void*,unsigned char*,size_t);
enum EFStatus ef_attempt(EFCallback,void*,unsigned nonblocking,unsigned attempts,unsigned*used);
typedef struct {int key;unsigned identity;unsigned char payload[32];} EFRecord;
enum EFStatus ef_sort(int*,size_t);
enum EFStatus ef_sort_records(EFRecord*,size_t);
#endif
