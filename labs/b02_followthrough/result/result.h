#ifndef BF_RESULT_H
#define BF_RESULT_H
#include "depth.h"
typedef void *(*BfAlloc)(void *,size_t);
typedef void (*BfFree)(void *,void *);
typedef struct {unsigned char *data;size_t length;BfFree release;void *context;} BfBytes;
enum {BF_INTEGER,BF_BORROWED,BF_ERROR,BF_OWNED,BF_EMPTY};
typedef struct {unsigned tag;union {int number;struct {const unsigned char *data;size_t length;} borrowed;int error;BfBytes owned;} value;} BfValue;
int bf_number(int,unsigned,B2Result *);
int bf_text(const unsigned char *,size_t,B2Result *);
void bf_value_init(BfValue *);
int bf_clone(BfValue *,const unsigned char *,size_t,BfAlloc,BfFree,void *);
int bf_move(BfValue *,BfValue *);
int bf_take(BfValue *,BfBytes *);
void bf_bytes_drop(BfBytes *);
void bf_reset(BfValue *);
int bf_value_render(const BfValue *,unsigned char *,size_t,size_t *);
typedef struct {unsigned version,tag;int number;unsigned char bytes[256];size_t length;unsigned boolean;} BfWire;
int bf_encode(const BfWire *,unsigned char *,size_t,size_t *);
int bf_decode(const unsigned char *,size_t,BfWire *);
#endif
