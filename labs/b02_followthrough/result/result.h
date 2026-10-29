#ifndef BF_RESULT_H
#define BF_RESULT_H
#include "depth.h"
typedef void *(*bf_alloc_fn)(void *,size_t);
typedef void (*bf_free_fn)(void *,void *);
struct bf_bytes {unsigned char *data;size_t length;bf_free_fn release;void *context;};
enum {BF_INTEGER,BF_BORROWED,BF_ERROR,BF_OWNED,BF_EMPTY};
struct bf_value {unsigned tag;union {int number;struct {const unsigned char *data;size_t length;} borrowed;int error;struct bf_bytes owned;} value;};
int bf_number(int,unsigned,struct b2_result *);
int bf_text(const unsigned char *,size_t,struct b2_result *);
void bf_value_init(struct bf_value *);
int bf_clone(struct bf_value *,const unsigned char *,size_t,bf_alloc_fn,bf_free_fn,void *);
int bf_move(struct bf_value *,struct bf_value *);
int bf_take(struct bf_value *,struct bf_bytes *);
void bf_bytes_drop(struct bf_bytes *);
void bf_reset(struct bf_value *);
int bf_value_render(const struct bf_value *,unsigned char *,size_t,size_t *);
struct bf_wire {unsigned version,tag;int number;unsigned char bytes[256];size_t length;unsigned boolean;};
int bf_encode(const struct bf_wire *,unsigned char *,size_t,size_t *);
int bf_decode(const unsigned char *,size_t,struct bf_wire *);
#endif
