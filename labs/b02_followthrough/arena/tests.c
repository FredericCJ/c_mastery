#include "arena.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"BF_CHECK:%d:%s\n",__LINE__,#x);exit(1);}}while(0)
static void marks(void){
 B2Arena a,foreign;b2_arena_init(&a);b2_arena_init(&foreign);B2Mark outer,inner;size_t at=77;
 CHECK(b2_mark(&a,&outer)==B2_OK&&b2_reserve(&a,3,1,&at)==B2_OK&&at==0&&b2_mark(&a,&inner)==B2_OK&&b2_reserve(&a,5,8,&at)==B2_OK&&at==8&&a.cursor==13);
 CHECK(b2_rollback(&a,outer)==B2_STALE&&a.cursor==13&&a.depth==2&&b2_rollback(&foreign,inner)==B2_STALE);
 CHECK(b2_rollback(&a,inner)==B2_OK&&a.cursor==3&&b2_rollback(&a,inner)==B2_STALE);
 B2Mark replacement;CHECK(b2_mark(&a,&replacement)==B2_OK&&replacement.depth==inner.depth&&replacement.id!=inner.id);
 CHECK(b2_rollback(&a,inner)==B2_STALE&&a.depth==2&&a.cursor==3&&b2_rollback(&a,replacement)==B2_OK&&b2_rollback(&a,outer)==B2_OK&&a.cursor==0);
 BfArena guarded;bf_arena_init(&guarded);B2Mark gm;CHECK(bf_arena_mark(&guarded,&gm)==B2_OK&&bf_arena_rollback(&guarded,gm)==B2_OK);
 b2_arena_init(&a);B2Mark stack[4];for(unsigned i=0;i<4;++i)CHECK(b2_mark(&a,&stack[i])==B2_OK);
 B2Mark before=stack[0];CHECK(b2_mark(&a,&before)==B2_FULL&&before.id==stack[0].id&&a.next_id==5);
 for(unsigned i=4;i>0;--i)CHECK(b2_rollback(&a,stack[i-1])==B2_OK);
 for(unsigned i=5;i<=15;++i){CHECK(b2_mark(&a,&inner)==B2_OK&&inner.id==i&&b2_rollback(&a,inner)==B2_OK);}
 CHECK(b2_mark(&a,&inner)==B2_FULL&&a.depth==0&&a.next_id==16);
 puts("BF marks validated actual-B02 depth4 lifetime15 stale-foreign repeated refusal");
}
typedef struct{BfArena*arena;BfBorrow borrow;B2Mark mark;unsigned id,fail,closed,calls;unsigned*trace,*used;} Resource;
typedef struct{unsigned id,calls;unsigned*trace,*used;} Count;
static int close_count(void*v){Count*c=v;CHECK(!c->calls);++c->calls;c->trace[(*c->used)++]=c->id;return 0;}
static int close_resource(void*v){Resource*r=v;r->trace[(*r->used)++]=r->id;++r->calls;CHECK(!r->closed);unsigned char byte=99;
 CHECK(bf_arena_get(r->arena,r->borrow,0,&byte)==B2_OK&&byte==42);
 BfBorrow b;B2Mark m;CHECK(bf_arena_reserve(r->arena,1,1,&b)==BF_ARENA_BUSY&&bf_arena_mark(r->arena,&m)==BF_ARENA_BUSY&&bf_arena_register(r->arena,close_resource,r)==BF_ARENA_BUSY&&bf_arena_rollback(r->arena,r->mark)==BF_ARENA_BUSY&&bf_arena_set(r->arena,r->borrow,0,1)==BF_ARENA_BUSY);
 if(r->fail){--r->fail;return 1;}r->closed=1;return 0;
}
static void cleanup(void){
 BfArena a;bf_arena_init(&a);B2Mark mark;BfBorrow b;CHECK(bf_arena_mark(&a,&mark)==B2_OK&&bf_arena_reserve(&a,8,8,&b)==B2_OK&&bf_arena_set(&a,b,0,42)==B2_OK);
 unsigned trace[20],used=0;Resource r[3];for(unsigned i=0;i<3;++i){r[i]=(Resource){&a,b,mark,i+1,i==1,0,0,trace,&used};CHECK(bf_arena_register(&a,close_resource,&r[i])==B2_OK);}
 CHECK(bf_arena_rollback(&a,mark)==BF_ARENA_CLEANUP&&a.raw.cursor==8&&a.count==1&&used==3&&trace[0]==3&&trace[1]==2&&trace[2]==1);
 BfBorrow untouched={NULL,77};CHECK(bf_arena_reserve(&a,1,1,&untouched)==BF_ARENA_BUSY&&untouched.id==77&&bf_arena_commit(&a,mark)==BF_ARENA_BUSY);
 CHECK(r[0].closed&&r[2].closed&&!r[1].closed&&bf_arena_rollback(&a,mark)==B2_OK&&used==4&&trace[3]==2&&a.raw.cursor==0&&!a.count);
 CHECK(r[0].calls==1&&r[1].calls==2&&r[2].calls==1);unsigned char byte=77;CHECK(bf_arena_get(&a,b,0,&byte)==B2_STALE&&byte==77&&bf_arena_rollback(&a,mark)==B2_STALE);
 BfArena full;bf_arena_init(&full);Count counts[9];used=0;CHECK(bf_arena_mark(&full,&mark)==B2_OK);
 for(unsigned i=0;i<9;++i){counts[i]=(Count){i+1,0,trace,&used};CHECK(bf_arena_register(&full,close_count,&counts[i])==(i<8?B2_OK:B2_FULL));}
 CHECK(full.cleanup_count==8&&!counts[8].calls);CHECK(close_count(&counts[8])==0);
 CHECK(bf_arena_rollback(&full,mark)==B2_OK&&used==9&&trace[0]==9);
 for(unsigned i=1;i<9;++i)CHECK(trace[i]==9-i&&counts[i-1].calls==1);
 puts("BF cleanup validated reverse3-2-1 retry2 no-double-close retained-byte-authority busy-reentry");
}
static void commit(void){
 BfArena a;bf_arena_init(&a);B2Mark outer,inner;BfBorrow parent,child,empty;unsigned char byte=77;
 CHECK(bf_arena_mark(&a,&outer)==B2_OK&&bf_arena_reserve(&a,3,1,&parent)==B2_OK&&bf_arena_set(&a,parent,0,9)==B2_OK);
 CHECK(bf_arena_mark(&a,&inner)==B2_OK&&bf_arena_reserve(&a,5,8,&child)==B2_OK&&bf_arena_set(&a,child,0,42)==B2_OK&&bf_arena_reserve(&a,0,8,&empty)==B2_OK&&a.raw.cursor==16);
 CHECK(bf_arena_commit(&a,outer)==B2_STALE&&bf_arena_commit(&a,inner)==B2_OK&&a.raw.depth==1&&a.raw.cursor==16);
 CHECK(bf_arena_get(&a,child,0,&byte)==B2_OK&&byte==42&&bf_arena_get(&a,parent,0,&byte)==B2_OK&&byte==9&&bf_arena_get(&a,empty,0,&byte)==B2_RANGE);
 CHECK(bf_arena_commit(&a,inner)==B2_STALE&&bf_arena_commit(&a,outer)==B2_INVALID&&bf_arena_rollback(&a,outer)==B2_OK);
 CHECK(bf_arena_get(&a,parent,0,&byte)==B2_STALE&&bf_arena_get(&a,child,0,&byte)==B2_STALE&&bf_arena_get(&a,empty,0,&byte)==B2_STALE);
 unsigned trace[2],used=0;Count r[2]={{1,0,trace,&used},{2,0,trace,&used}};
 CHECK(bf_arena_mark(&a,&outer)==B2_OK&&bf_arena_register(&a,close_count,&r[0])==B2_OK);
 CHECK(bf_arena_mark(&a,&inner)==B2_OK&&bf_arena_register(&a,close_count,&r[1])==B2_OK&&bf_arena_commit(&a,inner)==B2_OK&&!used);
 CHECK(bf_arena_rollback(&a,outer)==B2_OK&&used==2&&trace[0]==2&&trace[1]==1);
 puts("BF commit validated inner-retains-allocation outer-owns-reclamation zero-length-identity");
}
static void borrow(void){
 BfArena a,b;bf_arena_init(&a);bf_arena_init(&b);B2Mark mark;BfBorrow old,latest;unsigned char out=77;
 BfBorrow untouched={NULL,77};CHECK(bf_arena_reserve(&a,SIZE_MAX,8,&untouched)==B2_FULL&&untouched.id==77&&!a.count&&a.next_object==1&&!a.raw.cursor);
 CHECK(bf_arena_mark(&a,&mark)==B2_OK&&bf_arena_reserve(&a,1,1,&old)==B2_OK&&bf_arena_get(&b,old,0,&out)==B2_STALE);
 CHECK(bf_arena_rollback(&a,mark)==B2_OK&&bf_arena_mark(&a,&mark)==B2_OK&&bf_arena_reserve(&a,1,1,&latest)==B2_OK&&old.id!=latest.id&&bf_arena_get(&a,old,0,&out)==B2_STALE);
 for(unsigned id=3;id<=17;++id)CHECK(bf_arena_reserve(&a,0,1,&latest)==B2_OK&&latest.id==id);
 CHECK(bf_arena_reserve(&a,0,1,&latest)==B2_FULL&&bf_arena_rollback(&a,mark)==B2_OK&&bf_arena_mark(&a,&mark)==B2_OK);
 for(unsigned id=18;id<=31;++id)CHECK(bf_arena_reserve(&a,0,1,&latest)==B2_OK&&latest.id==id);
 CHECK(bf_arena_reserve(&a,0,1,&latest)==B2_FULL&&bf_arena_rollback(&a,mark)==B2_OK&&bf_arena_reserve(&a,0,1,&latest)==B2_FULL);
 puts("BF borrow validated owner-plus-nonreused31 IDs byte-api no-raw-revocation-claim");
}
static int extents(void){size_t cursor,n,align;while(scanf("%zu %zu %zu",&cursor,&n,&align)==3){B2Arena a;b2_arena_init(&a);size_t unused,out=77;CHECK(b2_reserve(&a,cursor,1,&unused)==B2_OK);int rc=b2_reserve(&a,n,align,&out);printf("%d %zu %zu\n",rc,out,a.cursor);}return 0;}
static int history(void){unsigned n;char ops[9];while(scanf("%u %8s",&n,ops)==2){CHECK(n<=6);BfArena a;bf_arena_init(&a);B2Mark stack[4];printf("%u",n);
 for(unsigned i=0;i<n;++i){unsigned op=(unsigned)(ops[i]-'0');CHECK(op<5);size_t depth=a.raw.depth;int rc;B2Mark token={&a.raw,1,0};if(depth)token=stack[op==3?0:depth-1];
 if(op==0){B2Mark m;rc=bf_arena_mark(&a,&m);if(rc==B2_OK)stack[depth]=m;}
 else if(op==1){BfBorrow b;rc=bf_arena_reserve(&a,1,2,&b);}
 else if(op==4)rc=bf_arena_commit(&a,token);else rc=bf_arena_rollback(&a,token);
 printf(" %d %zu %zu %u %zu",rc,a.raw.cursor,a.raw.depth,a.raw.next_id,a.count);
 }putchar('\n');}return 0;}
int main(int argc,char**argv){CHECK(argc==2);if(!strcmp(argv[1],"marks"))marks();else if(!strcmp(argv[1],"cleanup"))cleanup();else if(!strcmp(argv[1],"commit"))commit();else if(!strcmp(argv[1],"borrow"))borrow();else if(!strcmp(argv[1],"extents"))return extents();else if(!strcmp(argv[1],"oracle"))return history();else if(!strcmp(argv[1],"metrics"))printf("{\"arena_bytes\":%zu,\"backing_bytes\":64,\"object_capacity\":16,\"cleanup_capacity\":8,\"mark_capacity\":4,\"mark_ids\":15,\"object_ids\":31}\n",sizeof(BfArena));else if(!strcmp(argv[1],"limits"))printf("%zu\n",SIZE_MAX);else return 2;return 0;}
