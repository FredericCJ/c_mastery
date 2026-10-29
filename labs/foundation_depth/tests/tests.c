#include "depth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"FD_CHECK:%s:%d:%s\n",__FILE__,__LINE__,#x); return 1; } } while (0)
#define STATUS(expression, expected) CHECK((expression)==(expected))

static int semantic(void) {
#ifdef FD_ASSESSOR_TESTS
#include "semantic_answers.inc"
#else
    /* Public gate checks submission completeness only. Correct classification,
     * rationale and authority are separate assessor obligations. */
    for (unsigned i=0;i<FD_CASE_COUNT;i++) CHECK(fd_verdict((enum fd_case)i)!=FD_UNKNOWN);
#endif
    CHECK(fd_verdict((enum fd_case)-1)==FD_UNKNOWN);
    int value=5,old=99;
    STATUS(fd_repair_step(&value,&old),FD_OK); CHECK(value==6 && old==5);
    value=INT_MAX; old=99;
    STATUS(fd_repair_step(&value,&old),FD_RANGE); CHECK(value==INT_MAX && old==99);
    STATUS(fd_repair_step(&value,&value),FD_INVALID); CHECK(value==INT_MAX);
    value=INT_MIN;
    STATUS(fd_repair_step(&value,&old),FD_OK); CHECK(value==INT_MIN+1 && old==INT_MIN);
    return 0;
}
static int extent(void) {
    for (size_t offset=0;offset<18;offset++) for (size_t count=0;count<18;count++)
    for (size_t width=0;width<18;width++) for (size_t limit=0;limit<32;limit++) {
        size_t out=917, exact=offset+count*width;
        enum fd_status s=fd_extent(offset,count,width,limit,&out);
        CHECK(s==(exact<=limit?FD_OK:FD_RANGE));
        CHECK(out==(exact<=limit?exact:917));
    }
    size_t out=917;
    STATUS(fd_extent(SIZE_MAX,1,1,SIZE_MAX,&out),FD_RANGE); CHECK(out==917);
    STATUS(fd_extent(0,SIZE_MAX,2,SIZE_MAX,&out),FD_RANGE); CHECK(out==917);
    STATUS(fd_extent(SIZE_MAX,SIZE_MAX,0,SIZE_MAX,&out),FD_OK); CHECK(out==SIZE_MAX);
    STATUS(fd_extent(0,0,SIZE_MAX,0,&out),FD_OK); CHECK(out==0);
    STATUS(fd_extent(SIZE_MAX,0,0,SIZE_MAX-1,&out),FD_RANGE); CHECK(out==0);
    STATUS(fd_extent(SIZE_MAX-1,1,1,SIZE_MAX,&out),FD_OK); CHECK(out==SIZE_MAX);
    out=917; STATUS(fd_from_signed(-1,&out),FD_RANGE); CHECK(out==917);
    STATUS(fd_from_signed(INTMAX_MIN,&out),FD_RANGE); CHECK(out==917);
    STATUS(fd_from_signed(0,&out),FD_OK); CHECK(out==0);
    if ((uintmax_t)INTMAX_MAX>SIZE_MAX) {
        out=917; STATUS(fd_from_signed(INTMAX_MAX,&out),FD_RANGE); CHECK(out==917);
    } else { STATUS(fd_from_signed(INTMAX_MAX,&out),FD_OK); CHECK(out==(size_t)INTMAX_MAX); }
    STATUS(fd_extent(0,0,0,0,0),FD_INVALID);
    return 0;
}
static int cursor(void) {
    const unsigned char bytes[]={2,3,5,7,11,13,17};
    for (size_t split=0;split<=sizeof bytes;split++) {
        struct fd_cursor c={bytes,sizeof bytes,0},view={0,0,0};
        const unsigned char *p=0;
        STATUS(fd_take(&c,split,&p),FD_OK); CHECK(p==bytes && c.position==split);
        STATUS(fd_subview(&c,0,sizeof bytes-split,&view),FD_OK);
        CHECK(c.position==split && view.base==bytes+split && view.position==0);
        STATUS(fd_take(&view,sizeof bytes-split,&p),FD_OK); CHECK(p==bytes+split);
        STATUS(fd_take(&view,0,&p),FD_OK); CHECK(p==bytes+sizeof bytes);
    }
    struct fd_cursor c={bytes,sizeof bytes,1}; const unsigned char *p=bytes+4;
    STATUS(fd_take(&c,SIZE_MAX,&p),FD_RANGE); CHECK(c.position==1 && p==bytes+4);
    struct fd_cursor view={bytes,3,2};
    STATUS(fd_subview(&c,SIZE_MAX,0,&view),FD_RANGE); CHECK(view.base==bytes && view.length==3 && view.position==2);
    STATUS(fd_subview(&c,1,4,&view),FD_OK);
    STATUS(fd_subview(&view,1,2,&view),FD_OK); CHECK(view.base==bytes+3 && view.length==2 && view.position==0);
    c=(struct fd_cursor){0,0,0};
    STATUS(fd_take(&c,0,&p),FD_OK); CHECK(!p && c.position==0);
    STATUS(fd_subview(&c,0,0,&view),FD_OK); CHECK(!view.base && !view.length);
    STATUS(fd_take(&c,1,&p),FD_RANGE); CHECK(!p);
    c=(struct fd_cursor){0,1,0}; STATUS(fd_take(&c,0,&p),FD_INVALID);
    c=(struct fd_cursor){bytes,1,2}; STATUS(fd_take(&c,0,&p),FD_INVALID);
    return 0;
}
static int record_equal(struct fd_record a,struct fd_record b) { return a.flags==b.flags && a.count==b.count && a.value==b.value; }
static int codec(void) {
    const unsigned char known[][8]={
        {1,0,0,0,0,0,0,0}, {1,3,0x12,0x34,0xff,0xff,0xff,0xff},
        {1,1,0xff,0xff,0x80,0,0,0}, {1,2,0,1,0x7f,0xff,0xff,0xff}};
    const struct fd_record expected[]={{0,0,0},{3,0x1234,-1},{1,65535,INT32_MIN},{2,1,INT32_MAX}};
    for (size_t i=0;i<4;i++) {
        struct fd_record result={0,0,0}; unsigned char wire[10]; memset(wire,0xa5,sizeof wire);
        STATUS(fd_decode(known[i],8,&result),FD_OK); CHECK(record_equal(result,expected[i]));
        STATUS(fd_encode(expected[i],wire+1,8),FD_OK);
        CHECK(!memcmp(wire+1,known[i],8) && wire[0]==0xa5 && wire[9]==0xa5);
        result=(struct fd_record){0,0,0}; STATUS(fd_decode(wire+1,8,&result),FD_OK); CHECK(record_equal(result,expected[i]));
    }
    for (size_t n=0;n<=9;n++) if (n!=8) {
        unsigned char wire[9]={1,0}; struct fd_record result={3,999,42};
        STATUS(fd_decode(wire,n,&result),FD_RANGE); CHECK(record_equal(result,(struct fd_record){3,999,42}));
    }
    for (unsigned field=0;field<2;field++) for (unsigned value=0;value<=255;value++) {
        unsigned char wire[8]={1,0}; wire[field]=(unsigned char)value;
        struct fd_record result={3,999,42}; int valid=field==0?value==1:value<=3;
        STATUS(fd_decode(wire,8,&result),valid?FD_OK:FD_INVALID);
        if (!valid) CHECK(record_equal(result,(struct fd_record){3,999,42}));
    }
    unsigned char wire[8]; memset(wire,0xa5,sizeof wire);
    STATUS(fd_encode((struct fd_record){4,0,0},wire,8),FD_INVALID);
    for (size_t i=0;i<8;i++) CHECK(wire[i]==0xa5);
    STATUS(fd_encode((struct fd_record){0,0,0},wire,7),FD_RANGE);
    for (size_t i=0;i<8;i++) CHECK(wire[i]==0xa5);
    return 0;
}
struct fd_life { unsigned fail_alloc,fail_init,allocations,frees,initializations,destructions,live; struct fd_slot *slot; };
static void *allocate(void *ctx,size_t size,size_t align) {
    struct fd_life *l=ctx; l->allocations++;
    if (size!=sizeof(struct fd_value) || align!=_Alignof(struct fd_value) || l->fail_alloc) return 0;
    void *p=malloc(size); if (p) l->live++; return p;
}
static void deallocate(void *ctx,void *p) { struct fd_life *l=ctx; l->frees++; l->live--; free(p); }
static int initialize(void *ctx,struct fd_value *p) {
    struct fd_life *l=ctx; l->initializations++;
    if (l->slot->state!=FD_ACQUIRED) return 1;
    p->value=37; p->serial=123;
    return l->fail_init?1:0;
}
static void destroy(void *ctx,struct fd_value *p) { struct fd_life *l=ctx; if (p->value==37 && p->serial==123) l->destructions++; }
static int lifecycle(void) {
    for (unsigned failure=0;failure<3;failure++) {
        struct fd_slot s=FD_SLOT_INITIALIZER; struct fd_life l={0}; l.fail_alloc=failure==0; l.fail_init=failure==1; l.slot=&s;
        struct fd_allocator a={&l,allocate,deallocate}; const struct fd_value sentinel={99,99}; const struct fd_value *published=&sentinel;
        STATUS(fd_slot_publish(&s,&published),FD_STATE); CHECK(published==&sentinel);
        enum fd_status status=fd_slot_acquire(&s,a);
        if (l.fail_alloc) { CHECK(status==FD_NOMEM && s.state==FD_EMPTY && !l.live); fd_slot_reset(&s); CHECK(!l.frees); continue; }
        CHECK(status==FD_OK && s.state==FD_ACQUIRED && l.live==1);
        STATUS(fd_slot_publish(&s,&published),FD_STATE); CHECK(published==&sentinel);
        STATUS(fd_slot_acquire(&s,a),FD_STATE); CHECK(l.allocations==1);
        status=fd_slot_initialize(&s,initialize,destroy,&l);
        if (l.fail_init) {
            CHECK(status==FD_INIT_FAILED && s.state==FD_EMPTY && !l.live && l.frees==1 && !l.destructions);
        } else {
            CHECK(status==FD_OK && s.state==FD_INITIALIZED);
            STATUS(fd_slot_publish(&s,&published),FD_OK);
            CHECK(s.state==FD_PUBLISHED && published->value==37 && published->serial==123);
            STATUS(fd_slot_initialize(&s,initialize,destroy,&l),FD_STATE);
            fd_slot_reset(&s); CHECK(s.state==FD_EMPTY && l.destructions==1 && l.frees==1 && !l.live);
        }
        fd_slot_reset(&s); CHECK(l.frees==1 && l.initializations==1);
    }
    struct fd_slot s=FD_SLOT_INITIALIZER; struct fd_life l={0}; l.slot=&s;
    STATUS(fd_slot_acquire(&s,(struct fd_allocator){&l,allocate,deallocate}),FD_OK);
    fd_slot_reset(&s); CHECK(l.frees==1 && !l.initializations && !l.destructions);
    return 0;
}
static int overlap(void) {
    for (size_t dst=0;dst<=12;dst++) for (size_t src=0;src<=12;src++) for (size_t n=0;n<=12;n++) {
        unsigned char a[12],snapshot[12],expected[12];
        for (size_t i=0;i<12;i++) a[i]=snapshot[i]=expected[i]=(unsigned char)(i*17+3);
        int valid=n<=12-dst && n<=12-src;
        if (valid) for (size_t i=0;i<n;i++) expected[dst+i]=snapshot[src+i];
        STATUS(fd_move(a,12,dst,src,n),valid?FD_OK:FD_RANGE); CHECK(!memcmp(a,expected,12));
    }
    STATUS(fd_move(0,0,0,0,0),FD_OK);
    STATUS(fd_move(0,1,0,0,0),FD_INVALID);
    unsigned char a[12]={0}; STATUS(fd_move(a,12,SIZE_MAX,0,0),FD_RANGE);
    for (unsigned mask=0;mask<=255;mask++) {
        unsigned char src[256],dst[258];
        for (unsigned i=0;i<256;i++) src[i]=(unsigned char)i;
        dst[0]=77;dst[257]=88; fd_transform(dst+1,src,256,(unsigned char)mask);
        CHECK(dst[0]==77 && dst[257]==88);
        for (unsigned i=0;i<256;i++) CHECK(dst[i+1]==(unsigned char)(i^mask));
    }
    fd_transform(0,0,0,0);
    return 0;
}
static int handle_equal(struct fd_handle a,struct fd_handle b) { return a.owner==b.owner && a.index==b.index && a.generation==b.generation; }
static int handles(void) {
    struct fd_pool p={0},other={0}; fd_pool_init(&p); fd_pool_init(&other);
    struct fd_handle history[FD_POOL_CAPACITY*FD_GENERATION_MAX],h={&p,77,77};
    size_t count=0; struct fd_value out={99,99};
    for (size_t slot=0;slot<FD_POOL_CAPACITY;slot++) for (unsigned generation=1;generation<=FD_GENERATION_MAX;generation++) {
        STATUS(fd_pool_acquire(&p,(struct fd_value){(int32_t)count,(uint32_t)count+1},&h),FD_OK);
        CHECK(h.index==slot && h.generation==generation && h.owner==&p);
        STATUS(fd_pool_get(&p,h,&out),FD_OK); CHECK(out.value==(int32_t)count && out.serial==count+1);
        for (size_t i=0;i<count;i++) {
            out=(struct fd_value){99,99}; STATUS(fd_pool_get(&p,history[i],&out),FD_STALE); CHECK(out.value==99 && out.serial==99);
        }
        STATUS(fd_pool_get(&other,h,&out),FD_STALE);
        history[count++]=h;
        STATUS(fd_pool_release(&p,h),FD_OK); STATUS(fd_pool_release(&p,h),FD_STALE);
    }
    struct fd_handle before=h;
    STATUS(fd_pool_acquire(&p,(struct fd_value){0,0},&h),FD_FULL); CHECK(handle_equal(h,before));
    for (size_t i=0;i<FD_POOL_CAPACITY;i++) CHECK(p.entries[i].retired && !p.entries[i].live);
    STATUS(fd_pool_get(&p,(struct fd_handle){&p,SIZE_MAX,1},&out),FD_STALE);
    out=(struct fd_value){99,99};
    STATUS(fd_pool_get(0,h,&out),FD_INVALID); CHECK(out.value==99 && out.serial==99);
    STATUS(fd_pool_release(0,h),FD_INVALID);
    STATUS(fd_pool_get(&p,h,0),FD_INVALID);
    struct fd_handle active[FD_POOL_CAPACITY];
    for (size_t i=0;i<FD_POOL_CAPACITY;i++) STATUS(fd_pool_acquire(&other,(struct fd_value){0,0},&active[i]),FD_OK);
    STATUS(fd_pool_acquire(&other,(struct fd_value){0,0},&h),FD_FULL);
    for (size_t i=0;i<FD_POOL_CAPACITY;i++) STATUS(fd_pool_release(&other,active[i]),FD_OK);
    return 0;
}
static int defense(void) {
    const unsigned char valid[]={1,0,0,2,0,0,0,3};
    int32_t counter=10;
    STATUS(fd_apply(valid,8,0,&counter),FD_DENIED); CHECK(counter==10);
    STATUS(fd_apply(valid,8,1,&counter),FD_OK); CHECK(counter==16);
    for (size_t n=0;n<=9;n++) if (n!=8) {
        unsigned char supplied[9]={1,0,0,2,0,0,0,3,0}; counter=10;
        STATUS(fd_apply(supplied,n,1,&counter),FD_RANGE); CHECK(counter==10);
    }
    unsigned char invalid[8]={2,0,0,2,0,0,0,3}; counter=10;
    STATUS(fd_apply(invalid,8,1,&counter),FD_INVALID); CHECK(counter==10);
    invalid[0]=1; invalid[1]=0x80;
    STATUS(fd_apply(invalid,8,1,&counter),FD_INVALID); CHECK(counter==10);
    const unsigned char overflow[]={1,0,0,2,0x7f,0xff,0xff,0xff};
    STATUS(fd_apply(overflow,8,1,&counter),FD_RANGE); CHECK(counter==10);
    const unsigned char zero[]={1,3,0,0,0x80,0,0,0};
    STATUS(fd_apply(zero,8,0,&counter),FD_DENIED); CHECK(counter==10);
    STATUS(fd_apply(zero,8,1,&counter),FD_OK); CHECK(counter==10);
    return 0;
}
int main(int argc,char **argv) {
    const struct { const char *name; int (*test)(void); } cases[]={
        {"semantic",semantic},{"extent",extent},{"cursor",cursor},{"codec",codec},
        {"lifecycle",lifecycle},{"overlap",overlap},{"handles",handles},{"defense",defense}};
    if (argc!=2) return 2;
    for (size_t i=0;i<sizeof cases/sizeof cases[0];i++) if (!strcmp(argv[1],cases[i].name)) return cases[i].test();
    return 2;
}
