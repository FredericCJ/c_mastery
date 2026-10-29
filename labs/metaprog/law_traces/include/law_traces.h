#ifndef CMASTERY_MPT_LAW_TRACES_H
#define CMASTERY_MPT_LAW_TRACES_H
#include <stddef.h>
#include <stdint.h>
enum { MPT_OK,MPT_INVALID,MPT_NOMEM,MPT_RANGE,MPT_NO_FAILURE };
enum { MPT_BYTES,MPT_ASCII_TEXT };
struct mpt_view {const unsigned char *bytes;size_t length;unsigned cache;};
typedef void *(*mpt_allocate_fn)(void *,size_t);
typedef void (*mpt_free_fn)(void *,void *);
struct mpt_allocator {mpt_allocate_fn allocate;mpt_free_fn release;void *context;};
struct mpt_owner {unsigned char *bytes;size_t length;unsigned cache;struct mpt_allocator allocator;};
int mpt_assign(struct mpt_owner *,struct mpt_view,struct mpt_allocator);
void mpt_drop(struct mpt_owner *);
struct mpt_view mpt_view(const struct mpt_owner *);
int mpt_equal(struct mpt_view,struct mpt_view,unsigned,unsigned *);
int mpt_hash(struct mpt_view,unsigned,uint64_t *);
#endif
