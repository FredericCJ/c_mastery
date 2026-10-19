#include "shared.h"
#include <limits.h>
int mc_shared_init(McShared *s,McAllocator a)
{
    unsigned *p;
    if(!s || s->initialized || !a.allocate || !a.release)return -1;
    p=a.allocate(a.context,sizeof *p);if(!p)return -1;*p=0;
    if(pthread_mutex_init(&s->mutex,NULL)){a.release(a.context,p);return -1;}
    s->value=p;s->allocation=a;s->references=1;s->releases=0;s->initialized=1;return 0;
}
int mc_shared_retain(McShared *s,McShared **out)
{
    int status=-1;
    if(!s || !out || !s->initialized || pthread_mutex_lock(&s->mutex))return -1;
    if(s->value && s->references<3){
#ifndef MC_MUTANT_RETAIN
        ++s->references;
#endif
        *out=s;status=0;}
    if(pthread_mutex_unlock(&s->mutex))return -1;
    return status;
}
int mc_shared_add(McShared *s,unsigned amount)
{
    int status=-1;
    if(!s || !s->initialized)return -1;
#ifndef MC_MUTANT_RACE
    if(pthread_mutex_lock(&s->mutex))return -1;
#endif
    if(s->value && amount<=UINT_MAX-*s->value){*s->value+=amount;status=0;}
#ifndef MC_MUTANT_RACE
    if(pthread_mutex_unlock(&s->mutex))return -1;
#endif
    return status;
}
int mc_shared_read(McShared *s,unsigned *out)
{
    int status=-1;
    if(!s || !out || !s->initialized || pthread_mutex_lock(&s->mutex))return -1;
    if(s->value){*out=*s->value;status=0;}
    if(pthread_mutex_unlock(&s->mutex))return -1;
    return status;
}
int mc_shared_drop(McShared **owner)
{
    McShared *s;
    if(!owner || !(s=*owner) || !s->initialized || pthread_mutex_lock(&s->mutex))return -1;
    if(!s->references){(void)pthread_mutex_unlock(&s->mutex);return -1;}
    *owner=NULL;
    if(--s->references==0){void *p=s->value;s->value=NULL;++s->releases;s->allocation.release(s->allocation.context,p);}
    return pthread_mutex_unlock(&s->mutex)?-1:0;
}
int mc_shared_finish(McShared *s)
{
    if(!s || !s->initialized || pthread_mutex_lock(&s->mutex))return -1;
    if(s->references){(void)pthread_mutex_unlock(&s->mutex);return -1;}
    if(pthread_mutex_unlock(&s->mutex) || pthread_mutex_destroy(&s->mutex))return -1;
    s->initialized=0;return 0;
}
