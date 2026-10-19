#define _GNU_SOURCE
#include "map.h"
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#ifndef PM_MUTANT
#define PM_MUTANT 0
#endif
#define CHECK(x) do{if(!(x)){fprintf(stderr,"PL_CHECK:%d:%s\n",__LINE__,#x);return 1;}}while(0)
static int file(char name[64]){strcpy(name,"./pm-owned-XXXXXX");int f=mkostemp(name,O_CLOEXEC);return f;}
static int wait_child(pid_t p){int s;pid_t got;do{got=waitpid(p,&s,0);}while(got<0&&errno==EINTR);return got==p?s:-1;}
static int views(void){
 long page=sysconf(_SC_PAGESIZE);CHECK(page>0);char name[64];int fd=file(name);CHECK(fd>=0);CHECK(ftruncate(fd,2*page)==0);CHECK(pwrite(fd,"abcdef",6,0)==6);
 PMMap map={0};CHECK(pm_map(fd,0,(size_t)page,6,&map)==0);int closed=close(fd);CHECK(closed==0);
 char out[8]={0};CHECK(pm_copy(&map,(PMBorrow){0,6,map.generation},out,sizeof out)==0&&memcmp(out,"abcdef",6)==0);
 memset(out,0xa5,sizeof out);CHECK(pm_copy(&map,(PMBorrow){6,1,map.generation},out,sizeof out)==ERANGE&&out[0]==(char)0xa5);
 CHECK(pm_copy(&map,(PMBorrow){SIZE_MAX,1,map.generation},out,sizeof out)==ERANGE);
 CHECK(pm_copy(&map,(PMBorrow){6,0,map.generation},NULL,0)==0);
 fd=open(name,O_RDWR|O_CLOEXEC);CHECK(fd>=0);PMBorrow old={0,1,map.generation};uintptr_t address=(uintptr_t)map.base;
 CHECK(pm_remap(&map,fd,(size_t)(2*page),6)==0);CHECK((uintptr_t)map.base!=address);CHECK(pm_copy(&map,old,out,sizeof out)==ESTALE);
 CHECK(pm_copy(&map,(PMBorrow){0,1,map.generation},out,sizeof out)==0&&out[0]=='a');
 uint64_t saved=map.generation;map.generation=UINT64_MAX;CHECK(pm_remap(&map,fd,(size_t)page,6)==EOVERFLOW&&map.generation==UINT64_MAX);map.generation=saved;
 PMMap sentinel=map;CHECK(pm_map(fd,0,0,0,&map)==EINVAL&&map.base==sentinel.base);CHECK(pm_map(fd,1,16,1,&map)==EINVAL&&map.base==sentinel.base);
 CHECK(pm_map(fd,0,(size_t)page,(size_t)(2*page),&map)==EINVAL);CHECK(pm_map(fd,0,(size_t)(16*page),SIZE_MAX,&map)==EINVAL);
 char other[64];int wrong=file(other);CHECK(wrong>=0&&ftruncate(wrong,2*page)==0);CHECK(pm_remap(&map,wrong,(size_t)page,6)==EXDEV);CHECK(map.generation==sentinel.generation);
 CHECK(close(wrong)==0&&unlink(other)==0);CHECK(pm_unmap(&map)==0&&!map.active&&pm_unmap(&map)==0);CHECK(close(fd)==0&&unlink(name)==0);
 return 0;
}
static int codec(void){
 for(unsigned endian=1;endian<=2;++endian)for(unsigned n=0;n<=64;++n){
  PMRecord input={UINT64_C(0x0102030405060708),n,{0}},out={0};for(unsigned i=0;i<n;++i)input.payload[i]=(unsigned char)(i*7);
  unsigned char a[128],b[128];CHECK(pm_encode(a,&input,endian)==0);memcpy(b,a,128);CHECK(pm_decode(b,128,&out)==0);
  CHECK(out.session==input.session&&out.length==input.length&&memcmp(out.payload,input.payload,n)==0);
  /* Independent expected byte order, no native scalar alias. */
  for(unsigned i=0;i<8;++i)CHECK(a[8+i]==(endian==1?8-i:i+1));
  a[4]=0;out.session=99;CHECK(pm_decode(a,128,&out)==EAGAIN&&out.session==99);
  a[4]=99;CHECK(pm_decode(a,128,&out)==EPROTO);a[4]=1;a[5]=2;CHECK(pm_decode(a,128,&out)==EPROTO);a[5]=1;
  a[28]=1;CHECK(pm_decode(a,128,&out)==EPROTO);a[28]=0;
  memset(a+16,255,4);CHECK(pm_decode(a,128,&out)==EPROTO&&out.session==99);
  CHECK(pm_decode(b,31,&out)==EPROTO);CHECK(pm_decode(b,127,&out)==EPROTO);
  unsigned char oversized[129]={0};memcpy(oversized,b,128);CHECK(pm_decode(oversized,sizeof oversized,&out)==EPROTO);
  if(n){b[32]^=1;CHECK(pm_decode(b,128,&out)==EPROTO);}
 }
 return 0;
}
typedef struct {unsigned wanted;int ready,release;} Cut;
static void stop_at(unsigned stage,void *context){Cut *cut=context;if(stage==cut->wanted){char byte=(char)stage;if(write(cut->ready,&byte,1)!=1)_exit(120);if(read(cut->release,&byte,1)!=1)_exit(121);}}
static int publication(const char *self){
 for(unsigned cut=1;cut<=3;++cut){
  char name[64];int fd=file(name);CHECK(fd>=0);int ready[2],release[2];CHECK(pipe2(ready,O_CLOEXEC)==0&&pipe2(release,O_CLOEXEC)==0);
  pid_t child=fork();CHECK(child>=0);
  if(child==0){close(ready[0]);close(release[1]);Cut state={cut,ready[1],release[0]};int e=pm_publish(fd,42,"abc",3,stop_at,&state);_exit(e?122:0);}
  close(ready[1]);close(release[0]);char byte;ssize_t observed=read(ready[0],&byte,1);
  PMRecord record={999,0,{0}};int before=pm_read_record(fd,42,&record);
  /* Kill only the owned initializer after a confirmed transition. */
  int killed=kill(child,SIGKILL);int status=wait_child(child);close(ready[0]);close(release[1]);
  CHECK(observed==1&&(unsigned char)byte==cut&&killed==0&&WIFSIGNALED(status)&&WTERMSIG(status)==SIGKILL);
  CHECK((before==EAGAIN||before==EACCES)&&record.session==999);
  int after=pm_read_record(fd,42,&record);CHECK(cut==3?after==0:after==EAGAIN);
  if(cut==3){CHECK(record.session==42&&record.length==3&&memcmp(record.payload,"abc",3)==0);CHECK(pm_publish(fd,43,"xyz",3,NULL,NULL)==EEXIST);CHECK(pm_replace(fd,42,43,"xyz",3)==0);}
  else CHECK(pm_publish(fd,43,"xyz",3,NULL,NULL)==0);
  CHECK(pm_read_record(fd,42,&record)==ESTALE);CHECK(pm_read_record(fd,43,&record)==0&&record.session==43&&memcmp(record.payload,"xyz",3)==0);
  CHECK(pm_replace(fd,42,44,"bad",3)==ESTALE);CHECK(pm_replace(fd,43,43,"bad",3)==EINVAL);
  pid_t reader=fork();CHECK(reader>=0);if(reader==0){char *const args[]={(char *)self,"--reader",name,NULL};char *const env[]={NULL};execve(self,args,env);_exit(123);}
  status=wait_child(reader);CHECK(WIFEXITED(status)&&WEXITSTATUS(status)==0);
  CHECK(close(fd)==0&&unlink(name)==0);
 }
 /* Two original initializers compete for one publication; either may win. */
 char name[64];int fd=file(name),gate[2];CHECK(fd>=0&&pipe2(gate,O_CLOEXEC)==0);pid_t children[2];
 for(unsigned i=0;i<2;++i){children[i]=fork();CHECK(children[i]>=0);if(children[i]==0){close(gate[1]);char token;if(read(gate[0],&token,1)!=1)_exit(125);close(gate[0]);int e=pm_publish(fd,100,i?"red":"blu",3,NULL,NULL);_exit(e==0?0:e==EEXIST?3:124);}}
 close(gate[0]);CHECK(write(gate[1],"GG",2)==2);close(gate[1]);int a=wait_child(children[0]),b=wait_child(children[1]);
 CHECK(WIFEXITED(a)&&WIFEXITED(b));CHECK((WEXITSTATUS(a)==0&&WEXITSTATUS(b)==3)||(WEXITSTATUS(a)==3&&WEXITSTATUS(b)==0));
 PMRecord out;CHECK(pm_read_record(fd,100,&out)==0&&out.length==3);CHECK(!memcmp(out.payload,"red",3)||!memcmp(out.payload,"blu",3));CHECK(close(fd)==0&&unlink(name)==0);
 CHECK(waitpid(-1,NULL,WNOHANG)==-1&&errno==ECHILD);return 0;
}
static int residency(void){
 long page=sysconf(_SC_PAGESIZE);CHECK(page>0);
 for(unsigned pages=3;pages<=16;pages+=13){
  size_t length=(size_t)page*pages;unsigned char *p=mmap(NULL,length,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);CHECK(p!=MAP_FAILED);
  unsigned char before[16],after[16];struct rusage a,b;CHECK(mincore(p,length,before)==0&&getrusage(RUSAGE_SELF,&a)==0);p[(size_t)page*(pages/2)]=7;CHECK(getrusage(RUSAGE_SELF,&b)==0&&mincore(p,length,after)==0);
  unsigned prior=0,later=0;for(unsigned i=0;i<pages;++i){prior+=before[i]&1;later+=after[i]&1;}
  printf("RESIDENCY pages=%u mapped_bytes=%zu touched_pages=1 resident_before=%u resident_after=%u minor_fault_delta=%ld major_fault_delta=%ld\n",pages,length,prior,later,b.ru_minflt-a.ru_minflt,b.ru_majflt-a.ru_majflt);
  CHECK(p[(size_t)page*(pages/2)]==7&&munmap(p,length)==0);
 }
 return 0;
}
static int sharing(void){
 long page=sysconf(_SC_PAGESIZE);CHECK(page>0);
 for(unsigned shared=0;shared<2;++shared){
  int flag=shared?MAP_SHARED:MAP_PRIVATE;if(PM_MUTANT==7&&!shared)flag=MAP_SHARED;
  unsigned char *p=mmap(NULL,(size_t)page,PROT_READ|PROT_WRITE,flag|MAP_ANONYMOUS,-1,0);CHECK(p!=MAP_FAILED);p[0]=11;
  pid_t child=fork();CHECK(child>=0);if(child==0){if(PM_MUTANT!=8)p[0]=22;_exit(0);}
  int status=wait_child(child);unsigned observed=p[0];CHECK(munmap(p,(size_t)page)==0);
  CHECK(WIFEXITED(status)&&WEXITSTATUS(status)==0&&observed==(shared?22u:11u));
 }
 /* File-private write leaves both explicit file bytes and a second mapping unchanged. */
 char name[64];int fd=file(name);CHECK(fd>=0&&ftruncate(fd,page)==0&&pwrite(fd,"Q",1,0)==1);
 unsigned char *a=mmap(NULL,(size_t)page,PROT_READ|PROT_WRITE,MAP_PRIVATE,fd,0),*b=mmap(NULL,(size_t)page,PROT_READ,MAP_PRIVATE,fd,0);CHECK(a!=MAP_FAILED&&b!=MAP_FAILED);
 a[0]='R';char value;CHECK(pread(fd,&value,1,0)==1&&value=='Q'&&b[0]=='Q'&&a[0]=='R');
 CHECK(munmap(a,(size_t)page)==0&&munmap(b,(size_t)page)==0&&close(fd)==0&&unlink(name)==0);return 0;
}
static int faults(const char *helper){
 for(unsigned mode=0;mode<3;++mode){
  char name[64],text[2]={(char)('0'+mode),0},ready_number[24],release_number[24];int fd=file(name),ready[2],release[2];long page=sysconf(_SC_PAGESIZE);
  CHECK(fd>=0&&page>0&&ftruncate(fd,2*page)==0&&pipe(ready)==0&&pipe(release)==0);
  CHECK(snprintf(ready_number,sizeof ready_number,"%d",ready[1])>0&&snprintf(release_number,sizeof release_number,"%d",release[0])>0);
  pid_t p=fork();CHECK(p>=0);
  if(p==0){close(ready[0]);close(release[1]);char *const args[]={(char *)helper,text,name,ready_number,release_number,NULL};char *const env[]={NULL};execve(helper,args,env);_exit(123);}
  close(ready[1]);close(release[0]);char byte;ssize_t observed=read(ready[0],&byte,1);int trimmed=0;
  if(mode==2)trimmed=ftruncate(fd,page);
  ssize_t sent=write(release[1],"G",1);close(release[1]);close(ready[0]);int s=wait_child(p);CHECK(close(fd)==0&&unlink(name)==0);
  CHECK(observed==1&&byte=='R'&&trimmed==0&&sent==1);
  CHECK(mode==0?(WIFEXITED(s)&&WEXITSTATUS(s)==0):(WIFSIGNALED(s)&&WTERMSIG(s)==(mode==1?SIGSEGV:SIGBUS)));
 }
 return 0;
}
int main(int argc,char **argv){
 if(argc!=3)return 2;
 if(!strcmp(argv[1],"--reader")){int f=open(argv[2],O_RDWR|O_CLOEXEC);CHECK(f>=0);PMRecord r;CHECK(pm_read_record(f,43,&r)==0&&r.length==3&&!memcmp(r.payload,"xyz",3));CHECK(close(f)==0);puts("PLM_PASS unrelated-exec-reader");return 0;}
 int e;if(!strcmp(argv[1],"views"))e=views();else if(!strcmp(argv[1],"codec"))e=codec();else if(!strcmp(argv[1],"publication"))e=publication(argv[0]);else if(!strcmp(argv[1],"residency"))e=residency();else if(!strcmp(argv[1],"sharing"))e=sharing();else if(!strcmp(argv[1],"faults"))e=faults(argv[2]);else return 2;
 if(!e)printf("PLM_PASS %s\n",argv[1]);
 return e;
}
