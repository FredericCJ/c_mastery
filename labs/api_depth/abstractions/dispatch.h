#ifndef CMASTERY_ABSTRACTION_DISPATCH_H
#define CMASTERY_ABSTRACTION_DISPATCH_H
#include "stream.h"
typedef struct { unsigned char bytes[66];size_t length,calls,offered;unsigned mode; } AdSink;
typedef ptrdiff_t (*AdWrite)(AdSink *,const unsigned char *,size_t);
typedef struct { unsigned version,kind;AdWrite write; } AdOps;
typedef struct { AdSink sink;unsigned kind; } AdDirect;
typedef struct { AdSink sink;const AdOps *ops; } AdErased;
ptrdiff_t ad_one(AdSink *,const unsigned char *,size_t);
ptrdiff_t ad_four(AdSink *,const unsigned char *,size_t);
ptrdiff_t ad_all(AdSink *,const unsigned char *,size_t);
const AdOps *ad_operations(unsigned);
int ad_direct_init(AdDirect *,unsigned);
int ad_erased_init(AdErased *,const AdOps *);
ptrdiff_t ad_direct_write(void *,const unsigned char *,size_t);
ptrdiff_t ad_erased_write(void *,const unsigned char *,size_t);
ptrdiff_t ad_fixed_write(void *,const unsigned char *,size_t);
#endif
