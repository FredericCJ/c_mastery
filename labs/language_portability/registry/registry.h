#ifndef LR_REGISTRY_H
#define LR_REGISTRY_H
#include <stddef.h>
#include <stdint.h>
int lr_dispatch(uint32_t,int,int *);
int lr_name(uint32_t,const char **);
int lr_id(const char *,uint32_t *);
int lr_at(size_t,uint32_t *,const char **);
size_t lr_count(void);
size_t lr_probes(uint32_t);
struct lr_generic {int selection,value;double real;unsigned evaluated;};
int lr_generic(unsigned,struct lr_generic *);
#endif
