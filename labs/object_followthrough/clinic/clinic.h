#ifndef MC_CLINIC_H
#define MC_CLINIC_H
#include "depth.h"
typedef struct {void *context;void *(*allocate)(void *,size_t);void (*release)(void *,void *);} McAllocator;
typedef struct {int *value;McAllocator allocation;unsigned releases;} McOwner;
int mc_acquire(McOwner *,McAllocator,int);
int mc_read(const McOwner *,int *);
int mc_snapshot(const McOwner *,int *);
int mc_release(McOwner *);
FDStatus mc_move(unsigned char *,size_t,size_t,size_t,size_t);
typedef struct {unsigned prefix;int member;} McContainer;
typedef struct {int first;unsigned tag;} McFirst;
int mc_member(const McContainer *,const int *,int *);
int mc_initial(const McFirst *,const int *,int *);
#endif
