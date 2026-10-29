#ifndef CM_DEFENSE_TRANSFER_H
#define CM_DEFENSE_TRANSFER_H
#include <stddef.h>
#include <stdint.h>
int dt_extent(uint64_t,uint64_t,uint64_t,uint64_t,uint64_t,uint64_t,uint64_t*);
struct dt_view {const unsigned char *p;size_t n;unsigned tag;};
struct dt_owner {const unsigned char *p;size_t n;unsigned tag;};
int dt_read(const struct dt_owner*,struct dt_view,size_t,unsigned char*);
int dt_equal(const unsigned char*,size_t,const unsigned char*,size_t,size_t*);
#endif
