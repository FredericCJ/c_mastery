#ifndef BUILD_FOLLOWTHROUGH_FORMAL_H
#define BUILD_FOLLOWTHROUGH_FORMAL_H
#include <stddef.h>
#include <stdint.h>
enum bf_status { BF_OK, BF_INVALID, BF_RANGE, BF_EMPTY, BF_IO, BF_PENDING, BF_STUB };
enum bf_status bf_sum(const unsigned *a,size_t extent,size_t n,unsigned *out);
enum bf_status bf_sum_pairs(const unsigned *a,size_t extent,size_t n,unsigned *out);
enum bf_status bf_signed(const int *a,size_t extent,size_t n,int *out);
enum bf_status bf_max(const unsigned *a,size_t extent,size_t n,unsigned *out);
typedef ptrdiff_t (*bf_writer)(void *,const unsigned char *,size_t);
enum bf_status bf_write(bf_writer writer,void *context,const unsigned char *a,size_t n,unsigned budget,size_t *committed);
unsigned bf_read_clear(unsigned *status);
#endif
