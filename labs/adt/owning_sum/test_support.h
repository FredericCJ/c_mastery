#ifndef CMASTERY_ADO_TEST_SUPPORT_H
#define CMASTERY_ADO_TEST_SUPPORT_H
#include "owning_sum.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"ADO_CHECK %s:%d %s\n",__FILE__,__LINE__,#x);return 1;}}while(0)
typedef struct {size_t calls,drops,live,fail,partial,partial_cleanups;} Counter;
static unsigned char *allocations[8];
static void cleanup_allocations(void){for(unsigned i=0;i<8;++i){free(allocations[i]);allocations[i]=NULL;}}
static void remember(unsigned char *p){if(!p)return;for(unsigned i=0;i<8;++i)if(!allocations[i]){allocations[i]=p;return;}abort();}
static void forget(unsigned char *p){if(!p)return;for(unsigned i=0;i<8;++i)if(allocations[i]==p){allocations[i]=NULL;return;}abort();}
static int clone_bytes(void *context,const unsigned char *p,size_t n,unsigned char **out) {
    Counter *c=context;++c->calls;
    if(c->fail==c->calls&&!c->partial)return 0;
    unsigned char *q=n?malloc(n):NULL;
    if(n&&!q)return 0;
    if(c->fail==c->calls){free(q);++c->partial_cleanups;return 0;}
    for(size_t i=0;i<n;++i)q[i]=p[i];
    remember(q);*out=q;++c->live;return 1;
}
static void drop_bytes(void *context,unsigned char *p,size_t n) {
    Counter *c=context;(void)n;++c->drops;--c->live;forget(p);free(p);
}
static AdoOps operations(Counter *c){return(AdoOps){clone_bytes,drop_bytes,c};}
#endif
