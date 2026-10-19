#ifndef CM_DEFENSE_TRANSFER_H
#define CM_DEFENSE_TRANSFER_H
#include <stddef.h>
#include <stdint.h>
int dt_extent(uint64_t,uint64_t,uint64_t,uint64_t,uint64_t,uint64_t,uint64_t*);
typedef struct {const unsigned char *p;size_t n;unsigned tag;} DTView;
typedef struct {const unsigned char *p;size_t n;unsigned tag;} DTOwner;
int dt_read(const DTOwner*,DTView,size_t,unsigned char*);
int dt_equal(const unsigned char*,size_t,const unsigned char*,size_t,size_t*);
#endif
