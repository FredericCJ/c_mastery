#include "journal.h"
#include "journal_posix.h"
#include <inttypes.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(c) do{if(!(c)){fprintf(stderr,"RZ_CHECK: %s:%d %s\n",__FILE__,__LINE__,#c);exit(1);}}while(0)
struct rz_memory {
    unsigned char cache[RZJ_MAX_RECORDS*RZJ_RECORD_BYTES],durable[RZJ_MAX_RECORDS*RZJ_RECORD_BYTES];
    size_t size,durable_size,chunk;
    unsigned calls,cut,mode;
    bool failed;
};
static void persist(struct rz_memory *m) {for(size_t i=0;i<m->size;++i)m->durable[i]=m->cache[i];m->durable_size=m->size;}
static void crash(struct rz_memory *m) {
    if(m->mode==1)persist(m);
    if(m->mode==2 && m->size && !(m->size%64) && m->size>m->durable_size) {
        for(size_t i=m->size-8;i<m->size;++i)m->durable[i]=m->cache[i];
        m->durable_size=m->size;
    }
    m->size=m->durable_size;for(size_t i=0;i<m->size;++i)m->cache[i]=m->durable[i];
}
static int event(struct rz_memory *m) {
    ++m->calls;if(m->cut==m->calls){crash(m);m->failed=true;return RZJ_UNCERTAIN;}return RZ_OK;
}
static int size(void *p,size_t *out){*out=((struct rz_memory *)p)->size;return RZ_OK;}
static int read_bytes(void *p,size_t offset,unsigned char *out,size_t n,size_t *done) {
    struct rz_memory *m=p;if(offset>m->size)return RZ_INVALID;
    if(n>m->size-offset)n=m->size-offset;
    if(m->chunk && n>m->chunk)n=m->chunk;
    for(size_t i=0;i<n;++i)out[i]=m->cache[offset+i];
    *done=n;return RZ_OK;
}
static int write_bytes(void *p,size_t offset,const unsigned char *in,size_t n,size_t *done) {
    struct rz_memory *m=p;if(m->failed || offset>sizeof m->cache || n>sizeof m->cache-offset)return RZJ_UNCERTAIN;
    if(m->chunk && n>m->chunk)n=m->chunk;
    for(size_t i=0;i<n;++i)m->cache[offset+i]=in[i];
    if(offset+n>m->size)m->size=offset+n;
    *done=n;return event(m);
}
static int truncate_bytes(void *p,size_t n){struct rz_memory *m=p;if(n>m->size)return RZ_INVALID;m->size=n;return event(m);}
static int flush(void *p){struct rz_memory *m=p;if(m->failed)return RZJ_UNCERTAIN;int status=event(m);if(status==RZ_OK)persist(m);return status;}
static struct rzj_io io(struct rz_memory *m){return (struct rzj_io){m,size,read_bytes,write_bytes,truncate_bytes,flush};}
static void restart(struct rz_memory *m){m->mode=0;crash(m);m->failed=false;m->cut=m->calls=0;}
static void fresh(struct rz_memory *m,struct rzj_journal *j,unsigned window) {
    *m=(struct rz_memory){.chunk=7};CHECK(rzj_open(io(m),window,j)==RZ_OK);uint64_t token=0;
    CHECK(rzj_issue(j,0,&token)==RZ_OK && token==1);
}
static void nominal(void) {
    struct rz_memory memory;struct rzj_journal j;fresh(&memory,&j,4);int value=99;bool duplicate=true;
    CHECK(rzj_apply(&j,0,(struct rzj_request){0,1,1,3},&value,&duplicate)==RZ_OK && value==3 && !duplicate);
    CHECK(rzj_apply(&j,1,(struct rzj_request){1,2,1,2},&value,&duplicate)==RZ_OK && value==5 && !duplicate);
    CHECK(rzj_apply(&j,2,(struct rzj_request){0,1,1,3},&value,&duplicate)==RZ_OK && value==3 && duplicate && j.receiver.total==5);
    unsigned count=j.records;value=99;duplicate=false;
    CHECK(rzj_apply(&j,2,(struct rzj_request){0,1,1,4},&value,&duplicate)==RZ_CONFLICT && value==99 && !duplicate && j.records==count);
    uint64_t token=0;CHECK(rzj_issue(&j,2,&token)==RZ_OK && token==2);
    CHECK(rzj_apply(&j,3,(struct rzj_request){0,3,1,7},&value,&duplicate)==RZ_STALE && value==99 && j.receiver.total==5);
    CHECK(rzj_apply(&j,4,(struct rzj_request){0,1,2,3},&value,&duplicate)==RZ_OK && value==8 && !duplicate);
    restart(&memory);CHECK(rzj_open(io(&memory),4,&j)==RZ_OK && j.receiver.total==8 && j.issued==2);
    CHECK(rzj_issue(&j,4,&token)==RZ_OK && token==3);
    CHECK(rzj_apply(&j,4,(struct rzj_request){1,3,2,1},&value,&duplicate)==RZ_STALE);
    CHECK(rzj_apply(&j,4,(struct rzj_request){1,3,4,1},&value,&duplicate)==RZ_DENIED);
    struct rzj_journal before=j;memory.cache[64+32]^=1;
    CHECK(rzj_open(io(&memory),4,&j)==RZJ_CORRUPT && j.records==before.records && j.receiver.total==before.receiver.total);
    memory.cache[64+32]^=1;CHECK(rzj_open(io(&memory),3,&j)==RZJ_CORRUPT);

    fresh(&memory,&j,16);
    for(unsigned i=1;i<=4;++i)CHECK(rzj_apply(&j,0,(struct rzj_request){i%2,i,1,1},&value,&duplicate)==RZ_OK);
    count=j.records;CHECK(rzj_apply(&j,0,(struct rzj_request){0,5,1,1},&value,&duplicate)==RZ_FULL && j.records==count && j.receiver.total==4);
    for(unsigned i=j.records;i<RZJ_MAX_RECORDS;++i)CHECK(rzj_apply(&j,0,(struct rzj_request){1,1,1,1},&value,&duplicate)==RZ_OK && duplicate);
    CHECK(rzj_issue(&j,0,&token)==RZ_FULL && rzj_apply(&j,0,(struct rzj_request){1,1,1,1},&value,&duplicate)==RZ_FULL);
}
static void failure_cuts(void) {
    for(unsigned operation=0;operation<2;++operation)for(unsigned mode=0;mode<3;++mode)for(unsigned cut=1;cut<=68;++cut) {
        struct rz_memory memory;struct rzj_journal j;fresh(&memory,&j,4);memory.chunk=1;memory.calls=0;memory.cut=cut;memory.mode=mode;
        int result=99;bool duplicate=true;uint64_t token=99;
        int status=operation?rzj_issue(&j,1,&token):rzj_apply(&j,1,(struct rzj_request){0,1,1,3},&result,&duplicate);
        CHECK(status==RZ_OK || status==RZJ_UNCERTAIN);
        if(status==RZJ_UNCERTAIN) {
            CHECK(j.poisoned && j.receiver.total==0 && j.issued==1 && result==99 && duplicate && token==99);
            CHECK(rzj_issue(&j,1,&token)==RZJ_UNCERTAIN && token==99);
        }
        restart(&memory);struct rzj_journal recovered;CHECK(rzj_open(io(&memory),4,&recovered)==RZ_OK);
        if(operation)CHECK(recovered.issued==1 || recovered.issued==2);
        else CHECK(recovered.receiver.total==0 || recovered.receiver.total==3);
        if(status==RZ_OK)CHECK(operation?recovered.issued==2:recovered.receiver.total==3);
        if(operation)CHECK(rzj_issue(&recovered,1,&token)==RZ_OK && token==recovered.issued);
        else {
            bool was_committed=recovered.receiver.total==3;
            CHECK(rzj_apply(&recovered,1,(struct rzj_request){0,1,1,3},&result,&duplicate)==RZ_OK && result==3 && duplicate==was_committed);
        }
        CHECK(!(memory.size%64));
    }
}
static void replica(void) {
    struct rz_memory primary,secondary;struct rzj_journal a,b;fresh(&primary,&a,16);uint64_t token=0;
    for(unsigned i=2;i<=4;++i)CHECK(rzj_issue(&a,0,&token)==RZ_OK && token==i);
    secondary=primary;CHECK(rzj_open(io(&secondary),16,&b)==RZ_OK);
    CHECK(rzj_issue(&a,0,&token)==RZ_OK && token==5);int value;bool duplicate;
    CHECK(rzj_apply(&a,0,(struct rzj_request){0,1,5,3},&value,&duplicate)==RZ_OK && value==3);
    CHECK(rzj_apply(&b,0,(struct rzj_request){1,2,4,7},&value,&duplicate)==RZ_OK && value==7);
    CHECK(a.receiver.total==3 && b.receiver.total==7); /* Deliberately flawed independent writers. */
    CHECK(rzj_mirror(&a,io(&secondary))==RZJ_CORRUPT); /* Do not silently erase a divergent effect. */
    secondary=(struct rz_memory){.chunk=3};CHECK(rzj_mirror(&a,io(&secondary))==RZ_OK);
    struct rzj_replica replica;CHECK(rzj_replica_read(io(&secondary),16,&replica)==RZ_OK && replica.total==3 && replica.issued==5);
    value=99;CHECK(rzj_apply(&a,1,(struct rzj_request){1,2,4,7},&value,&duplicate)==RZ_STALE && value==99);
    /* The repaired follower returns a read snapshot with no effect-writer API. */
    for(unsigned mode=0;mode<3;++mode)for(unsigned cut=1;cut<=70;++cut) {
        secondary=(struct rz_memory){.chunk=1,.mode=mode,.cut=cut};int status=rzj_mirror(&a,io(&secondary));
        CHECK(status==RZ_OK || status==RZJ_UNCERTAIN);restart(&secondary);
        CHECK(rzj_open(io(&secondary),16,&b)==RZ_OK);CHECK(rzj_mirror(&a,io(&secondary))==RZ_OK);
        CHECK(rzj_open(io(&secondary),16,&b)==RZ_OK && b.receiver.total==3 && b.issued==5);
    }
}
static void recovery_cuts(void) {
    for(unsigned cut=1;cut<=2;++cut)for(unsigned mode=0;mode<3;++mode) {
        struct rz_memory m;struct rzj_journal j;fresh(&m,&j,4);m.cache[m.size++]=0xab;persist(&m);
        m.calls=0;m.cut=cut;m.mode=mode;unsigned count=j.records;
        CHECK(rzj_open(io(&m),4,&j)==RZJ_UNCERTAIN && j.records==count);
        restart(&m);CHECK(rzj_open(io(&m),4,&j)==RZ_OK && j.records==1 && m.size==64 && m.durable_size==64);
    }
}
static void native_file(void) {
    char path[]="/tmp/cmastery-rzj-XXXXXX";int fd=mkstemp(path);CHECK(fd>=0 && !unlink(path));
    struct rzj_posix file;struct rzj_io file_io;CHECK(rzj_posix(fd,3,&file,&file_io)==RZ_OK);
    int flags=fcntl(fd,F_GETFL);CHECK(flags>=0 && !fcntl(fd,F_SETFL,flags|O_APPEND));
    CHECK(rzj_posix(fd,3,&file,&file_io)==RZ_INVALID);CHECK(!fcntl(fd,F_SETFL,flags));
    struct rzj_journal j;CHECK(rzj_open(file_io,4,&j)==RZ_OK);uint64_t token;CHECK(rzj_issue(&j,0,&token)==RZ_OK && token==1);
    pid_t child=fork();CHECK(child>=0);
    if(!child) {
        int value=99;bool duplicate;int status=rzj_apply(&j,0,(struct rzj_request){0,1,1,3},&value,&duplicate);
        _exit(status==RZ_OK && value==3 && !duplicate?0:1); /* Reply deliberately never delivered. */
    }
    int status;CHECK(waitpid(child,&status,0)==child && WIFEXITED(status) && !WEXITSTATUS(status));
    CHECK(rzj_open(file_io,4,&j)==RZ_OK && j.receiver.total==3);
    int value=99;bool duplicate=false;CHECK(rzj_apply(&j,1,(struct rzj_request){0,1,1,3},&value,&duplicate)==RZ_OK && value==3 && duplicate);
    CHECK(rzj_issue(&j,1,&token)==RZ_OK && token==2);
    const unsigned char partial[]={1,2,3};CHECK(pwrite(fd,partial,sizeof partial,(off_t)j.records*64)==(ssize_t)sizeof partial && !fdatasync(fd));
    CHECK(rzj_open(file_io,4,&j)==RZ_OK && j.issued==2 && j.receiver.total==3);
    CHECK(!close(fd));
}
static int zero_read(void *p,size_t o,unsigned char *b,size_t n,size_t *done){(void)p;(void)o;(void)b;(void)n;*done=0;return RZ_OK;}
static int zero_write(void *p,size_t o,const unsigned char *b,size_t n,size_t *done){(void)p;(void)o;(void)b;(void)n;*done=0;return RZ_OK;}
static int excess_write(void *p,size_t o,const unsigned char *b,size_t n,size_t *done){(void)p;(void)o;(void)b;*done=n+1;return RZ_OK;}
static void invalid_progress(void) {
    struct rz_memory m;struct rzj_journal j;fresh(&m,&j,4);struct rzj_io bad=io(&m);bad.read=zero_read;
    CHECK(rzj_open(bad,4,&j)==RZJ_UNCERTAIN && j.records==1 && j.issued==1);
    for(unsigned mode=0;mode<2;++mode) {
        fresh(&m,&j,4);bad=io(&m);bad.write=mode?excess_write:zero_write;
        CHECK(rzj_open(bad,4,&j)==RZ_OK);int value=99;bool duplicate=true;
        CHECK(rzj_apply(&j,1,(struct rzj_request){0,1,1,3},&value,&duplicate)==RZJ_UNCERTAIN && j.poisoned && j.records==1 && value==99 && duplicate);
        CHECK(m.size==64 && m.durable_size==64);
    }
}
static void print_bytes(const struct rz_memory *m){for(size_t i=0;i<m->durable_size;++i)printf("%02x",m->durable[i]);}
static void oracle(void) {
    unsigned window,issued,client,id,fence,time;int delta;
    while(scanf("%u %u %u %u %d %u %u",&window,&issued,&client,&id,&delta,&fence,&time)==7) {
        struct rz_memory m;struct rzj_journal j;fresh(&m,&j,window);uint64_t token;
        for(unsigned i=2;i<=issued;++i)CHECK(rzj_issue(&j,0,&token)==RZ_OK && token==i);
        int value;bool duplicate;CHECK(rzj_apply(&j,0,(struct rzj_request){0,1,issued,1},&value,&duplicate)==RZ_OK);
        value=99;duplicate=true;int status=rzj_apply(&j,time,(struct rzj_request){client,id,fence,delta},&value,&duplicate);
        printf("%d %d %u %d %u %"PRIu64" ",status,value,duplicate,j.receiver.total,j.records,j.issued);print_bytes(&m);putchar('\n');
        restart(&m);CHECK(rzj_open(io(&m),window,&j)==RZ_OK);
    }
}
static void cut_oracle(void) {
    unsigned operation,mode,cut;
    while(scanf("%u %u %u",&operation,&mode,&cut)==3) {
        CHECK(operation<=1 && mode<=2 && cut<=68);
        struct rz_memory m;struct rzj_journal j;fresh(&m,&j,4);m.chunk=1;m.calls=0;m.cut=cut;m.mode=mode;
        int value=99;bool duplicate=true;uint64_t token=99;
        int status=operation?rzj_issue(&j,1,&token):rzj_apply(&j,1,(struct rzj_request){0,1,1,3},&value,&duplicate);
        printf("%d %d %u %"PRIu64" %u ",status,value,duplicate,token,j.poisoned);
        restart(&m);CHECK(rzj_open(io(&m),4,&j)==RZ_OK);
        printf("%d %u %"PRIu64" ",j.receiver.total,j.records,j.issued);print_bytes(&m);putchar('\n');
    }
}
int main(int argc,char **argv) {
    if(argc==2 && !strcmp(argv[1],"oracle")){oracle();return 0;}
    if(argc==2 && !strcmp(argv[1],"cuts")){cut_oracle();return 0;}
    nominal();failure_cuts();replica();recovery_cuts();native_file();invalid_progress();puts("journal tests passed");return 0;
}
