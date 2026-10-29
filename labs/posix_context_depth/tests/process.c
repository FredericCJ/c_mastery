#define _GNU_SOURCE
#include "context.h"
#include "test.h"
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <unistd.h>
struct pc_identity {pid_t pid,tid;uid_t uid;gid_t gid;int private_value;uintptr_t private_address,shared_address;};
static int private_integer=7;
static unsigned char *shared_byte;
static pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond=PTHREAD_COND_INITIALIZER;
static int ready,release_workers;
static void *worker(void *p){struct pc_identity *o=p;o->pid=getpid();o->tid=(pid_t)syscall(SYS_gettid);o->uid=geteuid();o->gid=getegid();o->private_address=(uintptr_t)&private_integer;o->shared_address=(uintptr_t)shared_byte;CHECK(pthread_mutex_lock(&mutex)==0);++ready;CHECK(pthread_cond_broadcast(&cond)==0);while(!release_workers)CHECK(pthread_cond_wait(&cond,&mutex)==0);CHECK(pthread_mutex_unlock(&mutex)==0);return NULL;}
static void read_exact(int f,void *p,size_t n){size_t used=0;while(used<n){ssize_t r=read(f,(unsigned char *)p+used,n-used);if(r<0&&errno==EINTR)continue;CHECK(r>0);used+=(size_t)r;}}
static void identity(void){
 pthread_t threads[2];struct pc_identity ids[2],child;int channel[2],status;
 unsigned char *shared=mmap(NULL,1,PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);CHECK(shared!=MAP_FAILED);*shared=1;
 shared_byte=shared;CHECK(pipe2(channel,O_CLOEXEC)==0);for(int i=0;i<2;++i)CHECK(pthread_create(&threads[i],NULL,worker,&ids[i])==0);
 CHECK(pthread_mutex_lock(&mutex)==0);while(ready<2)CHECK(pthread_cond_wait(&cond,&mutex)==0);
 /* The mutex remains locked across fork: child must never call pthread APIs. */
 pid_t pid=fork();CHECK(pid>=0);
 if(pid==0){close(channel[0]);private_integer=9;*shared=5;struct pc_identity x={getpid(),0,geteuid(),getegid(),private_integer,(uintptr_t)&private_integer,(uintptr_t)shared};const unsigned char *p=(const unsigned char *)&x;size_t used=0;while(used<sizeof x){ssize_t n=write(channel[1],p+used,sizeof x-used);if(n>0)used+=(size_t)n;else if(n<0&&errno==EINTR)continue;else _exit(99);}_exit(0);}
 close(channel[1]);read_exact(channel[0],&child,sizeof child);close(channel[0]);CHECK(waitpid(pid,&status,0)==pid&&WIFEXITED(status)&&WEXITSTATUS(status)==0);
 CHECK(private_integer==7&&child.private_value==9&&*shared==5&&child.pid==pid&&pid!=getpid());CHECK(child.uid==geteuid()&&child.gid==getegid());
 release_workers=1;CHECK(pthread_cond_broadcast(&cond)==0);CHECK(pthread_mutex_unlock(&mutex)==0);for(int i=0;i<2;++i)CHECK(pthread_join(threads[i],NULL)==0);
 CHECK(ids[0].pid==getpid()&&ids[1].pid==getpid()&&ids[0].tid!=ids[1].tid);CHECK(ids[0].uid==geteuid()&&ids[1].gid==getegid());
 CHECK(child.private_address==(uintptr_t)&private_integer&&child.shared_address==(uintptr_t)shared&&ids[0].private_address==child.private_address&&ids[1].shared_address==child.shared_address);
 printf("PC_IDENTITY parent=%ld child=%ld thread0=%ld thread1=%ld private_parent=%d private_child=%d shared=%u private_address=%p shared_address=%p child_private_address=%ju child_shared_address=%ju\n",(long)getpid(),(long)pid,(long)ids[0].tid,(long)ids[1].tid,private_integer,child.private_value,*shared,(void *)&private_integer,(void *)shared,(uintmax_t)child.private_address,(uintmax_t)child.shared_address);CHECK(munmap(shared,1)==0);shared_byte=NULL;
}
static void launch_case(const char *helper,int spawn,int missing,int bad,const char *dir){
 char parent[4096],after[4096],output[8192]={0};int fd[2],cwd;struct pc_launch r={0};struct rlimit before,end;
 char *args[]={(char *)helper,"","with spaces",NULL};char *env[]={"PC_CONFIG=synthetic=alpha","PC_SECOND=beta",NULL};char *empty[]={"PC_SECOND=beta",NULL};
 CHECK(getcwd(parent,sizeof parent)&&getrlimit(RLIMIT_NOFILE,&before)==0);cwd=open(dir,O_RDONLY|O_DIRECTORY|O_CLOEXEC);CHECK(cwd>=0&&pipe2(fd,O_CLOEXEC)==0);
 CHECK((spawn?pc_spawn:pc_launch)(bad?"/cmastery/no/such/original-helper":helper,args,missing?empty:env,cwd,fd[1],&r)==0);close(fd[1]);size_t used=0;for(;;){ssize_t n=read(fd[0],output+used,sizeof output-1-used);if(n<0&&errno==EINTR)continue;CHECK(n>=0);if(!n)break;used+=(size_t)n;CHECK(used<sizeof output-1);}close(fd[0]);close(cwd);
 if(bad)CHECK(r.kind==2&&r.value==ENOENT);else if(missing)CHECK(r.kind==1&&r.value==23&&strstr(output,"PC_MISSING_CONFIG"));else {CHECK(r.kind==1&&r.value==0&&strstr(output,"PC_HELPER")&&strstr(output,dir));char expected[128];snprintf(expected,sizeof expected,"soft=%llu hard=%llu",(unsigned long long)before.rlim_cur,(unsigned long long)before.rlim_max);CHECK(strstr(output,expected));snprintf(expected,sizeof expected,"pid=%ld euid=%lu egid=%lu",(long)r.pid,(unsigned long)geteuid(),(unsigned long)getegid());CHECK(strstr(output,expected));}
 CHECK(getcwd(after,sizeof after)&&!strcmp(parent,after)&&getrlimit(RLIMIT_NOFILE,&end)==0&&before.rlim_cur==end.rlim_cur&&before.rlim_max==end.rlim_max);
 printf("PC_LAUNCH backend=%s missing=%d bad=%d kind=%d value=%d %s",spawn?"spawn":"fork",missing,bad,r.kind,r.value,output);if(!used)putchar('\n');
}
static void offsets(const char *dir){char p[4096],a,b,c,e;snprintf(p,sizeof p,"%s/offsets",dir);int f=open(p,O_CREAT|O_EXCL|O_RDWR|O_CLOEXEC,0600);CHECK(f>=0&&write(f,"abcdef",6)==6&&lseek(f,0,SEEK_SET)==0);int d=dup(f),other=open(p,O_RDONLY|O_CLOEXEC);CHECK(d>=0&&other>=0);CHECK(read(f,&a,1)==1&&a=='a'&&lseek(d,0,SEEK_CUR)==1);CHECK(read(d,&b,1)==1&&b=='b'&&lseek(f,0,SEEK_CUR)==2);CHECK(pread(d,&e,1,4)==1&&e=='e'&&lseek(f,0,SEEK_CUR)==2);CHECK(read(f,&c,1)==1&&c=='c'&&lseek(d,0,SEEK_CUR)==3);CHECK(read(other,&a,1)==1&&a=='a'&&lseek(d,0,SEEK_CUR)==3);CHECK(fcntl(f,F_GETFD)&FD_CLOEXEC);CHECK(!(fcntl(d,F_GETFD)&FD_CLOEXEC));int flags=fcntl(d,F_GETFL);CHECK(flags>=0&&fcntl(d,F_SETFL,flags|O_NONBLOCK)==0&&(fcntl(f,F_GETFL)&O_NONBLOCK));CHECK(!(fcntl(other,F_GETFL)&O_NONBLOCK));close(f);CHECK(lseek(d,0,SEEK_CUR)==3);close(d);close(other);CHECK(unlink(p)==0);puts("PC_OFFSETS abc positioned=e duplicate=3 independent=1 cloexec_separate status_shared");}
int main(int argc,char **argv){CHECK(argc==2);char dir[]="/tmp/cmastery-pc-process-XXXXXX";CHECK(mkdtemp(dir));identity();
 pthread_t threads[2];struct pc_identity ids[2];ready=0;release_workers=0;for(int i=0;i<2;++i)CHECK(pthread_create(&threads[i],NULL,worker,&ids[i])==0);
 CHECK(pthread_mutex_lock(&mutex)==0);while(ready<2)CHECK(pthread_cond_wait(&cond,&mutex)==0);
 for(int s=0;s<2;++s){launch_case(argv[1],s,0,0,dir);launch_case(argv[1],s,1,0,dir);launch_case(argv[1],s,0,1,dir);}
 release_workers=1;CHECK(pthread_cond_broadcast(&cond)==0&&pthread_mutex_unlock(&mutex)==0);for(int i=0;i<2;++i)CHECK(pthread_join(threads[i],NULL)==0);
 offsets(dir);CHECK(rmdir(dir)==0);puts("PC_PROCESS_PASS");return 0;}
