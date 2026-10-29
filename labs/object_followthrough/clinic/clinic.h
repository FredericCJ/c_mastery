#ifndef MC_CLINIC_H
#define MC_CLINIC_H
#include "depth.h"
struct mc_allocator {void *context;void *(*allocate)(void *,size_t);void (*release)(void *,void *);};
struct mc_owner {int *value;struct mc_allocator allocation;unsigned releases;};
int mc_acquire(struct mc_owner *,struct mc_allocator,int);
int mc_read(const struct mc_owner *,int *);
int mc_snapshot(const struct mc_owner *,int *);
int mc_release(struct mc_owner *);
enum fd_status mc_move(unsigned char *,size_t,size_t,size_t,size_t);
struct mc_container {unsigned prefix;int member;};
struct mc_first {int first;unsigned tag;};
int mc_member(const struct mc_container *,const int *,int *);
int mc_initial(const struct mc_first *,const int *,int *);
#endif
