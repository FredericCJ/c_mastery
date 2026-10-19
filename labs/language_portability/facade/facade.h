#ifndef BF_FACADE_H
#define BF_FACADE_H
typedef struct {int value;} BfRecord;
typedef int (*BfGet)(void *);
BfRecord *bf_mutable(BfRecord *);
const BfRecord *bf_const(const BfRecord *);
int bf_binary_int(int,int);
unsigned bf_binary_unsigned(unsigned,unsigned);
int bf_sequence(BfGet,void *,BfGet,void *,int *);
#define BF_BORROW(x) _Generic((x),BfRecord*:bf_mutable,const BfRecord*:bf_const)(x)
#define BF_BINARY(a,b) ((void)sizeof(char[_Generic((a),int:_Generic((b),int:1,default:-1),unsigned:_Generic((b),unsigned:1,default:-1),default:-1)]),_Generic((a),int:bf_binary_int,unsigned:bf_binary_unsigned)(a,b))
#define BF_CAT_RAW(a,b) a##b
#define BF_CAT(a,b) BF_CAT_RAW(a,b)
#endif
