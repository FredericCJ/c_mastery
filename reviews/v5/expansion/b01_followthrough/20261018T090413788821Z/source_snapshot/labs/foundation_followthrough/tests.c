#include "follow.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"FF_CHECK:%d:%s\n",__LINE__,#x);return 1; } } while(0)
static int interval(void) {
    size_t cases=0;
    for(size_t o=0;o<16;++o)for(size_t n=0;n<16;++n)
    for(size_t w=0;w<16;++w)for(size_t limit=0;limit<16;++limit) {
        FFInterval out={711,913};size_t exact=o+n*w;FDStatus expected=exact<=limit?FD_OK:FD_RANGE;
        CHECK(ff_interval(o,n,w,limit,&out)==expected);
        CHECK(out.begin==(expected==FD_OK?o:711) && out.end==(expected==FD_OK?exact:913));++cases;
    }
    FFInterval out={711,913};CHECK(ff_interval(0,SIZE_MAX,2,SIZE_MAX,&out)==FD_RANGE);
    CHECK(out.begin==711 && out.end==913);
    CHECK(ff_interval(SIZE_MAX,0,0,SIZE_MAX,&out)==FD_OK && out.begin==SIZE_MAX && out.end==SIZE_MAX);
    CHECK(ff_interval(0,0,0,0,0)==FD_INVALID);
    printf("FF_OK:interval:%zu\n",cases);return 0;
}
typedef struct { unsigned frees, destroys; } Life;
static void *allocate(void *context,size_t n,size_t align) {
    (void)context;return n==sizeof(FDValue) && align==_Alignof(FDValue)?malloc(n):0;
}
static void release(void *context,void *p) { Life *life=context;++life->frees;free(p); }
static int typed(void *context,FDValue *p) { *p=*(const FDValue *)context;return 0; }
static int fail(void *context,FDValue *p) { (void)context;p->value=99;p->serial=88;return 1; }
static void destroy(void *context,FDValue *p) { Life *life=context;(void)p;++life->destroys; }
static int copy(void) {
    const FDValue values[]={{INT32_MIN,UINT32_MAX},{INT32_MAX,0},{-1,17},{0,42}};
    for(size_t i=0;i<sizeof values/sizeof values[0];++i) {
        FDSlot a=FD_SLOT_INITIALIZER,b=FD_SLOT_INITIALIZER;Life life={0,0};
        const FDValue *x=0,*y=0;FDValue source=values[i];
        CHECK(fd_slot_acquire(&a,(FDAllocator){&life,allocate,release})==FD_OK);
        CHECK(fd_slot_acquire(&b,(FDAllocator){&life,allocate,release})==FD_OK);
        CHECK(fd_slot_initialize(&a,typed,0,&source)==FD_OK);
        CHECK(fd_slot_initialize(&b,ff_copy_initialize,0,&source)==FD_OK);
        CHECK(fd_slot_publish(&a,&x)==FD_OK && fd_slot_publish(&b,&y)==FD_OK);
        CHECK(x->value==source.value && y->value==source.value && x->serial==source.serial && y->serial==source.serial);
        fd_slot_reset(&a);fd_slot_reset(&b);CHECK(life.frees==2);
    }
    puts("FF_OK:copy:4");return 0;
}
static int lifecycle(void) {
    FDSlot slot=FD_SLOT_INITIALIZER;Life life={0,0};const FDValue sentinel={77,88};const FDValue *out=&sentinel;
    CHECK(fd_slot_acquire(&slot,(FDAllocator){&life,allocate,release})==FD_OK);
    CHECK(fd_slot_publish(&slot,&out)==FD_STATE && out==&sentinel);
    CHECK(fd_slot_initialize(&slot,fail,destroy,&life)==FD_INIT_FAILED);
    CHECK(slot.state==FD_EMPTY && life.frees==1 && life.destroys==0);
    fd_slot_reset(&slot);CHECK(life.frees==1 && life.destroys==0);
    puts("FF_OK:lifecycle:2");return 0;
}
static int codec(int only_roundtrip) {
    for(unsigned count=0;count<256;++count) {
        FDRecord a={3,(uint16_t)(count*251),-2},b={0,0,0};unsigned char bytes[8];
        CHECK(ff_encode(a,bytes,8)==FD_OK && ff_decode(bytes,8,&b)==FD_OK);
        CHECK(a.flags==b.flags && a.count==b.count && a.value==b.value);
    }
    if(only_roundtrip) { puts("FF_OK:roundtrip:256");return 0; }
    const unsigned char golden[]={1,3,1,2,255,255,255,254};unsigned char bytes[8];FDRecord record={3,258,-2};
    CHECK(ff_encode(record,bytes,8)==FD_OK && memcmp(bytes,golden,8)==0);
    CHECK(ff_decode(golden,8,&record)==FD_OK && record.count==258 && record.value==-2);
    /* Inspect only a fully initialized live representation. Byte values/padding
     * are observations, never an equality condition for semantic records. */
    printf("FF_LAYOUT size=%zu align=%zu flags=%zu count=%zu value=%zu bytes=",sizeof record,_Alignof(FDRecord),offsetof(FDRecord,flags),offsetof(FDRecord,count),offsetof(FDRecord,value));
    const unsigned char *p=(const unsigned char *)&record;
    for(size_t i=0;i<sizeof record;++i)printf("%02x",p[i]);
    puts("\nFF_OK:codec:258");return 0;
}
static int partition(void) {
    const unsigned char bytes[]={1,2,3,4,5,6,7,8};size_t cases=0;
    for(size_t length=0;length<=8;++length)for(size_t at=0;at<=length;++at)
    for(size_t a=0;a<=length-at;++a)for(size_t b=0;b<=length-at-a;++b) {
        FDCursor one={bytes,length,at},two=one;const unsigned char *p=0,*q=0,*r=0;
        CHECK(fd_take(&one,a+b,&p)==FD_OK && fd_take(&two,a,&q)==FD_OK && fd_take(&two,b,&r)==FD_OK);
        CHECK(one.position==two.position && p==q && r==p+a);
        for(size_t i=0;i<a;++i)CHECK(q[i]==bytes[at+i]);
        for(size_t i=0;i<b;++i)CHECK(r[i]==bytes[at+a+i]);++cases;
    }
    printf("FF_OK:partition:%zu\n",cases);return 0;
}
int main(int argc,char **argv) {
    if(argc!=2)return 2;
    if(!strcmp(argv[1],"interval"))return interval();
    if(!strcmp(argv[1],"copy"))return copy();
    if(!strcmp(argv[1],"codec"))return codec(0);
    if(!strcmp(argv[1],"roundtrip"))return codec(1);
    if(!strcmp(argv[1],"partition"))return partition();
    if(!strcmp(argv[1],"lifecycle"))return lifecycle();return 2;
}
