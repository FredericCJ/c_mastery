#include "alias.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
static void *allocations[16];
static void cleanup(void){for(size_t i=0;i<16;++i){free(allocations[i]);allocations[i]=NULL;}}
static void *get_memory(size_t size,size_t alignment)
{
    void *p=alignment>_Alignof(max_align_t)?aligned_alloc(alignment,size):malloc(size);
    if(p){for(size_t i=0;i<16;++i)if(!allocations[i]){allocations[i]=p;return p;}free(p);}
    return NULL;
}
static void release_memory(void *p){for(size_t i=0;i<16;++i)if(allocations[i]==p){free(p);allocations[i]=NULL;return;}}
#define REQUIRE(x) do{if(!(x)){fprintf(stderr,"OS_ASSERT:%s\n",#x);cleanup();return 41;}}while(0)
typedef struct { int fail_alloc,fail_init,fail_drop;unsigned allocations,releases,initializations,destructions,rollbacks;unsigned char *child;size_t requested_size,requested_alignment; } Trace;
static void *allocate(void *context,size_t size,size_t alignment)
{Trace *t=context;t->requested_size=size;t->requested_alignment=alignment;if(t->fail_alloc)return NULL;void *p=get_memory(size,alignment);if(p)++t->allocations;return p;}
static void deallocate(void *context,void *p){Trace *t=context;++t->releases;release_memory(p);}
static int initialize(void *context,FDValue *p)
{
    Trace *t=context;++t->initializations;t->child=get_memory(8,1);if(!t->child)return -1;t->child[0]=19;
    if(t->fail_init){release_memory(t->child);t->child=NULL;++t->rollbacks;return -1;}
    p->value=7;p->serial=11;return 0;
}
static void destroy(void *context,FDValue *p){Trace *t=context;(void)p;++t->destructions;release_memory(t->child);t->child=NULL;}
static OsAllocator allocator(Trace *t,size_t capacity,size_t alignment)
{OsAllocator a={{t,allocate,deallocate},capacity,alignment};return a;}
static int aggregate_touch(int *member,OsContainer *owner)
{owner->member=23;return *member;}
static int lifecycle(void)
{
    Trace t={0};FDSlot s=FD_SLOT_INITIALIZER;const FDValue *view=NULL;OsAllocator a=allocator(&t,sizeof(FDValue),_Alignof(FDValue));
    t.fail_alloc=1;REQUIRE(os_acquire(&s,a)==-1 && s.state==FD_EMPTY && !t.allocations);
    t.fail_alloc=0;REQUIRE(os_acquire(&s,a)==0 && s.state==FD_ACQUIRED);REQUIRE(t.requested_size==sizeof(FDValue) && t.requested_alignment==_Alignof(FDValue));
    t.fail_init=1;REQUIRE(fd_slot_initialize(&s,initialize,destroy,&t)==FD_INIT_FAILED && s.state==FD_EMPTY && !t.child && t.rollbacks==1 && t.releases==1 && !t.destructions);
    t.fail_init=0;REQUIRE(os_acquire(&s,a)==0);REQUIRE(fd_slot_initialize(&s,initialize,destroy,&t)==FD_OK && s.state==FD_INITIALIZED);
    REQUIRE(fd_slot_publish(&s,&view)==FD_OK && view->value==7 && t.child[0]==19);view=NULL;fd_slot_reset(&s);
    REQUIRE(s.state==FD_EMPTY && !t.child && t.destructions==1 && t.releases==2);fd_slot_reset(&s);REQUIRE(t.releases==2);
    a.capacity=sizeof(FDValue)-1;REQUIRE(os_acquire(&s,a)==-1 && t.allocations==2);cleanup();puts("OS_LIFECYCLE_PASS");return 0;
}
static int wide_drop(void *context,OsWide *p)
{Trace *t=context;++t->destructions;if(t->fail_drop){--t->fail_drop;return -1;}release_memory(p->resource);p->resource=NULL;return 0;}
static int wide(void)
{
    Trace t={0};OsWideSlot s={0};const OsWide *view=NULL;OsAllocator a=allocator(&t,sizeof(OsWide),_Alignof(OsWide));
    OsAllocator weak=a;weak.alignment=_Alignof(max_align_t);
    REQUIRE(_Alignof(OsWide)>_Alignof(max_align_t) && os_wide_acquire(&s,weak,17,wide_drop,&t)==-1 && !t.allocations);
    REQUIRE(os_wide_acquire(&s,a,17,wide_drop,&t)==0 && s.state==OS_LIVE && t.requested_alignment==64);s.value->resource=get_memory(8,1);REQUIRE(s.value->resource!=NULL);s.value->resource[0]=23;
    REQUIRE(os_wide_borrow(&s,&view)==0 && view->value==17 && view->resource[0]==23);REQUIRE(os_wide_reset(&s)==-1 && !t.destructions && !t.releases);
    REQUIRE(os_wide_return(&s,&view)==0 && !view);t.fail_drop=1;
    REQUIRE(os_wide_reset(&s)==-1 && s.state==OS_CLEANUP && s.value->resource[0]==23 && !t.releases);
    REQUIRE(os_wide_borrow(&s,&view)==-1 && !view);REQUIRE(os_wide_reset(&s)==0 && s.state==OS_EMPTY && t.releases==1 && t.destructions==2);
    REQUIRE(os_wide_reset(&s)==0 && t.releases==1);cleanup();puts("OS_WIDE_PASS");return 0;
}
static int representations(void)
{
    FDValue source={7,11},out={99,88};void *raw=get_memory(sizeof source,_Alignof(FDValue));unsigned char declared[sizeof source],wire[8]={1,2,3,4,0x89,0xab,0xcd,0xef};
    REQUIRE(raw!=NULL);REQUIRE(os_clone(raw,&source)==0);FDValue *copy=raw;REQUIRE(copy->value==7 && copy->serial==11);
    for(size_t i=0;i<sizeof source;++i)declared[i]=((unsigned char *)&source)[i];
    REQUIRE(!memcmp(declared,&source,sizeof source)); /* Declared bytes never accessed as FDValue. */
    REQUIRE(os_wire(wire,8,&out)==0 && out.value==0x01020304 && out.serial==UINT32_C(0x89abcdef));wire[0]=0xff;REQUIRE(os_wire(wire,8,&out)==-1 && out.value==0x01020304);REQUIRE(os_host_representation(UINT32_C(0x89abcdef))==UINT32_C(0x89abcdef));
    {union {uint32_t value;unsigned char bytes[4];}u={UINT32_C(0x01020304)},other;unsigned char independent[4];uint32_t copied=0;for(size_t i=0;i<4;++i)independent[i]=u.bytes[i];for(size_t i=0;i<4;++i){((unsigned char *)&copied)[i]=independent[i];other.bytes[i]=independent[i];}REQUIRE(copied==u.value && other.value==copied);printf("OS_REPRESENTATION:%u,%u,%u,%u\n",u.bytes[0],u.bytes[1],u.bytes[2],u.bytes[3]);}
    {int x=17;unsigned int *unsigned_view=(unsigned int *)&x;const int *qualified=&x;unsigned char *bytes=(unsigned char *)&x;REQUIRE(*unsigned_view==17u && *qualified==17 && bytes!=NULL);}
    {OsContainer object={1,17};REQUIRE(aggregate_touch(&object.member,&object)==23);}
    release_memory(raw);cleanup();puts("OS_REPRESENTATIONS_PASS");return 0;
}
static int restricted(void)
{
    unsigned char source[16],a[16],b[16],c[16],inplace[16],snapshot[16];
    for(size_t i=0;i<16;++i)source[i]=(unsigned char)i;
    os_transform(a,source,16,0x55);os_ordinary(b,source,16,0x55);os_nested(c,source,16,0x55);
    REQUIRE(!memcmp(a,b,16) && !memcmp(b,c,16) && a[7]==(7^0x55));os_shared(c,source,source,16);for(size_t i=0;i<16;++i)REQUIRE(c[i]==0);
    memcpy(inplace,source,16);memcpy(snapshot,inplace,16);REQUIRE(os_partition(inplace,16,2,0,4,0x55,1)==-1 && !memcmp(inplace,snapshot,16));
    REQUIRE(os_partition(inplace,16,0,0,4,0x55,0)==-1 && !memcmp(inplace,snapshot,16));REQUIRE(os_partition(inplace,16,0,0,4,0x55,1)==0 && inplace[3]==(3^0x55));
    REQUIRE(os_partition(inplace,16,8,4,4,0x55,0)==0 && inplace[8]==(4^0x55));REQUIRE(os_partition(NULL,0,0,0,0,7,0)==0);
    memcpy(inplace,source,16);REQUIRE(os_move(inplace,16,2,0,8)==FD_OK);for(size_t i=0;i<8;++i)REQUIRE(inplace[2+i]==i);
    memcpy(inplace,source,16);REQUIRE(os_move(inplace,16,0,2,8)==FD_OK);for(size_t i=0;i<8;++i)REQUIRE(inplace[i]==2+i);
    puts("OS_RESTRICTED_PASS");return 0;
}
static int identity(void)
{
    FDPool p;FDHandle first={0},h={0};FDValue out={77,88};fd_pool_init(&p);
    REQUIRE(fd_pool_acquire(&p,(FDValue){1,11},&first)==FD_OK);REQUIRE(fd_pool_release(&p,first)==FD_OK);REQUIRE(fd_pool_get(&p,first,&out)==FD_STALE && out.value==77);
    for(int i=2;i<=3;++i){REQUIRE(fd_pool_acquire(&p,(FDValue){i,11},&h)==FD_OK && h.index==0);REQUIRE(fd_pool_release(&p,h)==FD_OK);}
    REQUIRE(fd_pool_acquire(&p,(FDValue){4,11},&h)==FD_OK && h.index==1 && p.entries[0].retired);REQUIRE(fd_pool_get(&p,first,&out)==FD_STALE);
    {OsContainer owner={9,17},foreign={8,22};int value=77;REQUIRE(os_member(&owner,&owner.member,&value)==0 && value==17);REQUIRE(os_member(&owner,&foreign.member,&value)==-1 && value==17);
#ifdef UINTPTR_MAX
    uintptr_t tag=0;REQUIRE(os_tag_roundtrip(&owner,3,&tag,&value)==0 && (tag&3u)==3 && value==17);
#endif
    printf("OS_COSTS:pointer=%zu,handle=%zu,owner=%zu,member_offset=%zu\n",sizeof(void *),sizeof(FDHandle),sizeof owner,offsetof(OsContainer,member));}
    puts("OS_IDENTITY_PASS");return 0;
}
int main(int argc,char **argv)
{switch(argc==2?atoi(argv[1]):-1){case 0:return lifecycle();case 1:return wide();case 2:return representations();case 3:return restricted();case 4:return identity();default:return 2;}}
