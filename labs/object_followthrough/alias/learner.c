#include "alias.h"
int os_acquire(struct fd_slot *s,struct os_allocator a){(void)s;(void)a;return -1;}
int os_wide_acquire(struct os_wide_slot *s,struct os_allocator a,int v,os_drop_fn d,void *c){(void)s;(void)a;(void)v;(void)d;(void)c;return -1;}
int os_wide_borrow(struct os_wide_slot *s,const struct os_wide **p){(void)s;(void)p;return -1;}
int os_wide_return(struct os_wide_slot *s,const struct os_wide **p){(void)s;(void)p;return -1;}
int os_wide_reset(struct os_wide_slot *s){(void)s;return -1;}
int os_clone(void *d,const struct fd_value *s){(void)d;(void)s;return -1;}
int os_wire(const unsigned char *b,size_t n,struct fd_value *o){(void)b;(void)n;(void)o;return -1;}
uint32_t os_wire_integer(const unsigned char *p){(void)p;return 0;}
uint32_t os_host_representation(uint32_t n){(void)n;return 0;}
void os_transform(unsigned char *restrict d,const unsigned char *restrict s,size_t n,unsigned char m){(void)d;(void)s;(void)n;(void)m;}
void os_ordinary(unsigned char *d,const unsigned char *s,size_t n,unsigned char m){(void)d;(void)s;(void)n;(void)m;}
void os_nested(unsigned char *restrict d,const unsigned char *restrict s,size_t n,unsigned char m){(void)d;(void)s;(void)n;(void)m;}
void os_shared(unsigned char *restrict d,const unsigned char *restrict a,const unsigned char *restrict b,size_t n){(void)d;(void)a;(void)b;(void)n;}
int os_partition(unsigned char *b,size_t l,size_t d,size_t s,size_t n,unsigned char m,int i){(void)b;(void)l;(void)d;(void)s;(void)n;(void)m;(void)i;return -1;}
enum fd_status os_move(unsigned char *b,size_t l,size_t d,size_t s,size_t n){(void)b;(void)l;(void)d;(void)s;(void)n;return FD_INVALID;}
int os_member(struct os_container *o,int *m,int *v){(void)o;(void)m;(void)v;return -1;}
#ifdef UINTPTR_MAX
int os_tag_roundtrip(struct os_container *o,unsigned t,uintptr_t *r,int *v){(void)o;(void)t;(void)r;(void)v;return -1;}
#endif
