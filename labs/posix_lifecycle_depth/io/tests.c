#define _GNU_SOURCE
#include "io.h"
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#ifndef PC_MUTANT
#define PC_MUTANT 0
#endif
#define CHECK(x) do{if(!(x)){fprintf(stderr,"PL_CHECK:%d:%s errno=%d\n",__LINE__,#x,errno);exit(1);}}while(0)
static unsigned write_calls,write_fail_call;static int write_error;static size_t write_limit;
#if defined(__i386__) && defined(_FILE_OFFSET_BITS) && _FILE_OFFSET_BITS == 64
#define __real_pwrite __real_pwrite64
#define __wrap_pwrite __wrap_pwrite64
#endif
ssize_t __real_pwrite(int,const void *,size_t,off_t);
ssize_t __wrap_pwrite(int fd,const void *data,size_t count,off_t offset){
    ++write_calls;if(write_calls==write_fail_call){errno=write_error;return -1;}
    if(write_limit&&count>write_limit)count=write_limit;
    return __real_pwrite(fd,data,count,offset);
}
typedef struct{char op;int result;}Event;
typedef struct{const Event *events;size_t total,next,source,used;unsigned char output[64];}Script;
static int event(Script *s,char op){CHECK(s->next<s->total&&s->events[s->next].op==op);return s->events[s->next++].result;}
static ssize_t scripted_read(void *context,unsigned char *b,size_t n){Script *s=context;int r=event(s,'r');if(r<0){errno=-r;return -1;}CHECK((size_t)r<=n&&s->source+(size_t)r<=7);memcpy(b,&"abcdefg"[s->source],(size_t)r);s->source+=(size_t)r;return r;}
static ssize_t scripted_write(void *context,const unsigned char *b,size_t n){Script *s=context;int r=event(s,'w');if(r<0){errno=-r;return -1;}CHECK((size_t)r<=n&&s->used+(size_t)r<=sizeof s->output);memcpy(s->output+s->used,b,(size_t)r);s->used+=(size_t)r;return r;}
typedef struct{int read_fd,write_fd;}Descriptors;
static ssize_t actual_read(void *p,unsigned char *b,size_t n){return read(((Descriptors*)p)->read_fd,b,n);}
static ssize_t actual_write(void *p,const unsigned char *b,size_t n){return write(((Descriptors*)p)->write_fd,b,n);}
static void stream(void){
    const Event events[]={{'r',3},{'w',1},{'w',-EINTR},{'w',2},{'r',4},{'w',2},{'w',-EAGAIN},{'w',2},{'r',0}};
    const Event failed[]={{'r',3},{'w',1},{'w',2},{'r',4},{'w',2},{'w',-EIO}};
    const Event waiting[]={{'r',-EAGAIN},{'r',-EINTR},{'r',0}};
    const Event zero[]={{'r',3},{'w',0}};
    Script s={events,sizeof events/sizeof *events,0,0,0,{0}};PCCopy c;PCOps ops={&s,scripted_read,scripted_write};enum PCStatus status;unsigned calls=0;
    pc_init(&c);CHECK(pc_resume(&c,ops,0)==PC_YIELD&&s.next==0);
    do{status=pc_resume(&c,ops,2);CHECK(++calls<20);}while(status==PC_YIELD);
    CHECK(status==PC_WAIT_WRITE&&c.committed==5&&c.end-c.next==2&&memcmp(c.bytes+c.next,"fg",2)==0);
    do{status=pc_resume(&c,ops,1);CHECK(++calls<40);}while(status==PC_YIELD);
    CHECK(status==PC_DONE&&c.committed==7&&s.used==7&&memcmp(s.output,"abcdefg",7)==0&&s.next==s.total);
    s=(Script){failed,sizeof failed/sizeof *failed,0,0,0,{0}};pc_init(&c);CHECK(pc_resume(&c,ops,20)==PC_ERROR&&c.error==EIO&&c.committed==5&&c.end-c.next==2);CHECK(memcmp(c.bytes+c.next,"fg",2)==0);size_t consumed=s.next;CHECK(pc_resume(&c,ops,20)==PC_ERROR&&s.next==consumed);
    s=(Script){waiting,sizeof waiting/sizeof *waiting,0,0,0,{0}};pc_init(&c);CHECK(pc_resume(&c,ops,20)==PC_WAIT_READ);CHECK(pc_resume(&c,ops,1)==PC_YIELD);CHECK(pc_resume(&c,ops,1)==PC_DONE);
    s=(Script){zero,sizeof zero/sizeof *zero,0,0,0,{0}};pc_init(&c);CHECK(pc_resume(&c,ops,20)==PC_ERROR&&c.error==EIO);
    pc_init(&c);c.end=2;c.committed=UINTMAX_MAX-1;consumed=s.next;CHECK(pc_resume(&c,ops,1)==PC_ERROR&&c.error==EOVERFLOW&&s.next==consumed);
    int source[2],sink[2];unsigned char scratch[512]={0};size_t filled=0;ssize_t n;CHECK(pipe2(source,O_NONBLOCK|O_CLOEXEC)==0&&pipe2(sink,O_NONBLOCK|O_CLOEXEC)==0);
    Descriptors descriptors={source[0],sink[1]};ops=(PCOps){&descriptors,actual_read,actual_write};pc_init(&c);CHECK(pc_resume(&c,ops,10)==PC_WAIT_READ);
    while((n=write(sink[1],scratch,sizeof scratch))>0){filled+=(size_t)n;CHECK(filled<=4*1024*1024);}CHECK(n<0&&(errno==EAGAIN||errno==EWOULDBLOCK));
    CHECK(write(source[1],"abcdefg",7)==7&&close(source[1])==0);CHECK(pc_resume(&c,ops,10)==PC_WAIT_WRITE&&c.committed==0&&c.end==7);
    size_t drained=0;while((n=read(sink[0],scratch,sizeof scratch))>0)drained+=(size_t)n;CHECK(drained==filled&&n<0&&(errno==EAGAIN||errno==EWOULDBLOCK));
    CHECK(pc_resume(&c,ops,10)==PC_DONE&&c.committed==7);CHECK(read(sink[0],scratch,sizeof scratch)==7&&memcmp(scratch,"abcdefg",7)==0);CHECK(close(source[0])==0&&close(sink[0])==0&&close(sink[1])==0);
    puts("PC_STREAM actual_nonblocking_pipes_and_scripted_errors");
}
static void extent(void){
    uintmax_t maximum=pc_off_max();off_t out=123;
    CHECK(sizeof(off_t)==8||sizeof(off_t)==4);CHECK(maximum==(sizeof(off_t)==8?(uintmax_t)INT64_MAX:(uintmax_t)INT32_MAX));
    CHECK(pc_extent(0,0,0,&out)==0&&out==0);CHECK(pc_extent(maximum,0,maximum,&out)==0&&(uintmax_t)out==maximum);
    out=123;CHECK(pc_extent(maximum,1,UINTMAX_MAX,&out)==EOVERFLOW&&out==123);CHECK(pc_extent(UINTMAX_MAX,1,UINTMAX_MAX,&out)==EOVERFLOW&&out==123);
    CHECK(pc_extent(5,3,7,&out)==ERANGE&&out==123);CHECK(pc_extent(8,0,7,&out)==ERANGE&&out==123);CHECK(pc_extent(7,0,7,&out)==0&&out==7);
    printf("PC_ABI off_t_bytes=%zu max=%ju size_t_bytes=%zu\n",sizeof(off_t),maximum,sizeof(size_t));
}
static void byte(int fd,char expected){char c='?';CHECK(read(fd,&c,1)==1&&c==expected);}
static void wait_owned(pid_t p){int status;CHECK(waitpid(p,&status,0)==p&&WIFEXITED(status)&&WEXITSTATUS(status)==0);}
static int new_file(const char *name){int fd=open(name,O_RDWR|O_CREAT|O_EXCL|O_CLOEXEC,0600);CHECK(fd>=0);return fd;}
static void sparse(void){
    int fd=new_file("sparse"),alias=dup(fd);unsigned char block[4096]={0},readback[4096];struct stat hole,zeros,large;size_t done=0;off_t end=1024*1024;
    CHECK(alias>=0&&ftruncate(fd,end+1)==0);CHECK(pc_write_record(fd,0,(const unsigned char*)"A",1,(uintmax_t)end+1,&done)==0&&done==1);CHECK(pc_write_record(fd,(uintmax_t)end,(const unsigned char*)"Z",1,(uintmax_t)end+1,&done)==0);
#if PC_MUTANT == 10
    CHECK(pwrite(fd,"!",1,4096)==1);
#endif
    CHECK(pread(fd,readback,sizeof readback,4096)==sizeof readback&&memcmp(block,readback,sizeof block)==0);CHECK(lseek(fd,0,SEEK_SET)==0);byte(alias,'A');CHECK(lseek(fd,0,SEEK_CUR)==1);
#if PC_MUTANT == 9
    CHECK(lseek(alias,end,SEEK_SET)==end);CHECK(read(alias,readback,1)==1);
#else
    CHECK(pread(alias,readback,1,end)==1);
#endif
    CHECK(readback[0]=='Z'&&lseek(fd,0,SEEK_CUR)==1);byte(fd,0);CHECK(lseek(alias,0,SEEK_CUR)==2);
    CHECK(fstat(fd,&hole)==0);for(off_t o=4096;o<end;o+=(off_t)sizeof block)CHECK(pwrite(fd,block,sizeof block,o)==sizeof block);CHECK(fstat(fd,&zeros)==0);CHECK(pread(fd,readback,sizeof readback,4096)==sizeof readback&&memcmp(block,readback,sizeof block)==0);
    off_t far=0;CHECK(pc_extent(UINTMAX_C(5)*1024*1024*1024,1,pc_off_max(),&far)==0);CHECK(ftruncate(fd,far+1)==0&&pwrite(fd,"L",1,far)==1&&pread(fd,readback,1,far)==1&&readback[0]=='L');CHECK(fstat(fd,&large)==0&&large.st_size==far+1);
    printf("PC_SPARSE hole_bytes=%jd hole_blocks512=%jd explicit_zero_blocks512=%jd large_bytes=%jd large_blocks512=%jd\n",(intmax_t)hole.st_size,(intmax_t)hole.st_blocks,(intmax_t)zeros.st_blocks,(intmax_t)large.st_size,(intmax_t)large.st_blocks);
    CHECK(close(alias)==0&&close(fd)==0);
}
static void position(void){
    int fd=new_file("records"),go[2],ready[2];pid_t children[2];size_t done=123;unsigned char record[8],got[8];CHECK(ftruncate(fd,4096)==0&&lseek(fd,512,SEEK_SET)==512);
    CHECK(pc_write_record(fd,16,(const unsigned char*)"TEST",4,4096,&done)==0&&done==4&&lseek(fd,0,SEEK_CUR)==512);CHECK(pread(fd,got,4,16)==4&&memcmp(got,"TEST",4)==0);
    write_calls=0;write_limit=2;write_fail_call=2;write_error=EAGAIN;
    CHECK(pc_write_record(fd,24,(const unsigned char*)"PART",4,4096,&done)==EAGAIN&&done==2&&pread(fd,got,4,24)==4&&got[0]=='P'&&got[1]=='A'&&got[2]==0);
    write_calls=0;write_fail_call=1;write_error=EINTR;
    CHECK(pc_write_record(fd,24,(const unsigned char*)"PART",4,4096,&done)==0&&done==4&&write_calls==3);write_limit=write_fail_call=0;
    int appended=open("records",O_WRONLY|O_APPEND|O_CLOEXEC);CHECK(appended>=0);done=123;CHECK(pc_write_record(appended,0,(const unsigned char*)"NO",2,4096,&done)==EINVAL&&done==123);CHECK(close(appended)==0);
    for(unsigned worker=0;worker<2;++worker){children[worker]=fork();CHECK(children[worker]>=0);if(children[worker]==0){for(unsigned i=0;i<32;++i){memset(record,(int)('A'+worker),sizeof record);CHECK(pc_write_record(fd,(worker*32u+i)*8u,record,sizeof record,4096,&done)==0&&done==sizeof record);}_exit(0);}}
    for(unsigned worker=0;worker<2;++worker){wait_owned(children[worker]);}
    for(unsigned i=0;i<64;++i){CHECK(pread(fd,got,sizeof got,i*8)==sizeof got);memset(record,i<32?'A':'B',sizeof record);CHECK(memcmp(got,record,sizeof got)==0);}
    /* Deterministic lseek/write interference on one shared open description. */
    CHECK(pipe(go)==0&&pipe(ready)==0);pid_t child=fork();CHECK(child>=0);if(child==0){close(go[1]);close(ready[0]);byte(go[0],'G');CHECK(lseek(fd,800,SEEK_SET)==800&&write(ready[1],"R",1)==1);_exit(0);}
    CHECK(close(go[0])==0&&close(ready[1])==0&&lseek(fd,700,SEEK_SET)==700&&write(go[1],"G",1)==1);byte(ready[0],'R');wait_owned(child);CHECK(write(fd,"WRONG",5)==5&&pread(fd,got,5,800)==5&&memcmp(got,"WRONG",5)==0);CHECK(close(go[1])==0&&close(ready[0])==0);
    /* A two-part overlapping record is not automatically atomic. Publish an
       invalid intermediate pair, then demonstrate cooperative lock exclusion. */
    unsigned char first[8]={1,1,1,1,254,254,254,254},next[8]={2,2,2,2,253,253,253,253};CHECK(pwrite(fd,first,8,1000)==8&&pwrite(fd,next,4,1000)==4&&pread(fd,got,8,1000)==8);CHECK(got[0]==2&&got[4]==254);CHECK(pwrite(fd,next+4,4,1004)==4);
    struct flock lock={.l_type=F_WRLCK,.l_whence=SEEK_SET,.l_start=1000,.l_len=8};CHECK(fcntl(fd,F_SETLK,&lock)==0&&pwrite(fd,first,4,1000)==4);child=fork();CHECK(child>=0);if(child==0){lock.l_type=F_RDLCK;CHECK(fcntl(fd,F_SETLK,&lock)<0&&(errno==EACCES||errno==EAGAIN));_exit(0);}wait_owned(child);
    CHECK(pwrite(fd,first+4,4,1004)==4);lock.l_type=F_UNLCK;CHECK(fcntl(fd,F_SETLK,&lock)==0&&pread(fd,got,8,1000)==8&&memcmp(got,first,8)==0);CHECK(close(fd)==0);
    puts("PC_POSITION disjoint_workers_shared_offset_counterexample_overlapping_lock_policy");
}
static void close_policy(void){
    int fd=new_file("original"),other=new_file("replacement");bool owned=true;CHECK(write(other,"R",1)==1);
    /* Inject an error after actual release, then deliberately reuse the number. */
    CHECK(close(fd)==0&&dup2(other,fd)==fd);CHECK(pc_close_transition(&owned,EINTR,PC_LINUX_RELEASES)==EINTR&&!owned);CHECK(fcntl(fd,F_GETFD)>=0);
    /* Contrasting model returns EINTR before releasing the same owned number. */
    owned=true;CHECK(pc_close_transition(&owned,EINTR,PC_EINTR_RETAINS)==EINTR&&owned);CHECK(fcntl(fd,F_GETFD)>=0&&close(fd)==0);CHECK(pc_close_transition(&owned,0,PC_EINTR_RETAINS)==0&&!owned);
    owned=true;CHECK(pc_close_transition(&owned,EIO,PC_LINUX_RELEASES)==EIO&&!owned);CHECK(close(other)==0);puts("PC_CLOSE injected_linux_release_and_alternative_retention_models");
}
int main(int argc,char **argv){char directory[]="./pc-owned-XXXXXX";CHECK(argc==2&&mkdtemp(directory)&&chdir(directory)==0);
    if(!strcmp(argv[1],"stream"))stream();else if(!strcmp(argv[1],"extent"))extent();else if(!strcmp(argv[1],"position"))position();else if(!strcmp(argv[1],"close_policy"))close_policy();else if(!strcmp(argv[1],"sparse"))sparse();else CHECK(0);puts("PC_PASS");return 0;}
