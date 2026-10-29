#ifndef CMASTERY_ABSTRACTION_DISPATCH_H
#define CMASTERY_ABSTRACTION_DISPATCH_H
#include "stream.h"
struct ad_sink { unsigned char bytes[66];size_t length,calls,offered;unsigned mode; };
typedef ptrdiff_t (*ad_write_fn)(struct ad_sink *,const unsigned char *,size_t);
struct ad_ops { unsigned version,kind;ad_write_fn write; };
struct ad_direct { struct ad_sink sink;unsigned kind; };
struct ad_erased { struct ad_sink sink;const struct ad_ops *ops; };
ptrdiff_t ad_one(struct ad_sink *,const unsigned char *,size_t);
ptrdiff_t ad_four(struct ad_sink *,const unsigned char *,size_t);
ptrdiff_t ad_all(struct ad_sink *,const unsigned char *,size_t);
const struct ad_ops *ad_operations(unsigned);
int ad_direct_init(struct ad_direct *,unsigned);
int ad_erased_init(struct ad_erased *,const struct ad_ops *);
ptrdiff_t ad_direct_write(void *,const unsigned char *,size_t);
ptrdiff_t ad_erased_write(void *,const unsigned char *,size_t);
ptrdiff_t ad_fixed_write(void *,const unsigned char *,size_t);
#endif
