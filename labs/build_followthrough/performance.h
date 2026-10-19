#ifndef BUILD_FOLLOWTHROUGH_PERFORMANCE_H
#define BUILD_FOLLOWTHROUGH_PERFORMANCE_H
#include <stddef.h>
#include <stdint.h>
/* Live uint32 extents n; disjoint outputs unless bf_add_inplace; n<=SIZE_MAX/4. */
void bf_add(const uint32_t *a,const uint32_t *b,uint32_t *out,size_t n);
void bf_add_disjoint(const uint32_t *restrict a,const uint32_t *restrict b,uint32_t *restrict out,size_t n);
void bf_add_inplace(uint32_t *a,const uint32_t *b,size_t n);
void bf_add_unroll(const uint32_t *a,const uint32_t *b,uint32_t *out,size_t n,unsigned lanes);
void bf_sse2(const uint32_t *a,const uint32_t *b,uint32_t *out,size_t n);
int bf_select(int feature,int os_support,int force_scalar);
void bf_dispatch(const uint32_t *a,const uint32_t *b,uint32_t *out,size_t n,int feature,int os_support,int force_scalar);
int bf_clamp_branch(int x);
int bf_clamp_select(int x);
uint32_t bf_recurrence(const uint32_t *a,size_t n);
float bf_reduce(const float *a,size_t n);
struct bf_record{uint32_t id,value;char label[32];};
void bf_to_soa(const struct bf_record *in,size_t n,uint32_t *id,uint32_t *value);
int bf_delete_soa(uint32_t *id,uint32_t *value,size_t *n,uint32_t remove_id);
void bf_blocked(const uint32_t *in,size_t rows,size_t cols,size_t stride,uint32_t *out,size_t out_stride,size_t block);
#endif
