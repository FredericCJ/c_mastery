#ifndef BF_FACADE_H
#define BF_FACADE_H
struct bf_record {int value;};
typedef int (*bf_get_fn)(void *);
struct bf_record *bf_mutable(struct bf_record *);
const struct bf_record *bf_const(const struct bf_record *);
int bf_binary_int(int,int);
unsigned bf_binary_unsigned(unsigned,unsigned);
int bf_sequence(bf_get_fn,void *,bf_get_fn,void *,int *);
#define BF_BORROW(x) _Generic((x),struct bf_record*:bf_mutable,const struct bf_record*:bf_const)(x)
#define BF_BINARY(a,b) ((void)sizeof(char[_Generic((a),int:_Generic((b),int:1,default:-1),unsigned:_Generic((b),unsigned:1,default:-1),default:-1)]),_Generic((a),int:bf_binary_int,unsigned:bf_binary_unsigned)(a,b))
#define BF_CAT_RAW(a,b) a##b
#define BF_CAT(a,b) BF_CAT_RAW(a,b)
#endif
