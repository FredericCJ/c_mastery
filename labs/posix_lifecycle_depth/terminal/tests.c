#define _GNU_SOURCE
#include "terminal.h"
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#ifndef PT_MUTANT
#define PT_MUTANT 0
#endif
#define CHECK(x) do{if(!(x)){fprintf(stderr,"PL_CHECK:%d:%s errno=%d\n",__LINE__,#x,errno);exit(1);}}while(0)
static unsigned get_calls,set_calls,fail_get,fail_set,partial_set,fork_calls,fail_fork,pipe_calls,fail_pipe,clock_calls;
static bool jump_clock;
static pid_t active_reader;
static void cleanup_reader(void){if(active_reader>0){int status;pid_t p=waitpid(active_reader,&status,WNOHANG);if(p==0){(void)kill(active_reader,SIGKILL);while(waitpid(active_reader,&status,0)<0&&errno==EINTR){}}active_reader=0;}}
int __real_tcgetattr(int,struct termios*);int __real_tcsetattr(int,int,const struct termios*);pid_t __real_fork(void);int __real_pipe2(int[2],int);int __real_clock_gettime(clockid_t,struct timespec*);
int __wrap_tcgetattr(int fd,struct termios *t){if(++get_calls==fail_get){errno=EIO;return -1;}return __real_tcgetattr(fd,t);}
int __wrap_tcsetattr(int fd,int action,const struct termios *t){++set_calls;if(set_calls==fail_set){errno=EIO;return -1;}if(set_calls==partial_set){struct termios partial=*t;partial.c_lflag^=ICANON;return __real_tcsetattr(fd,action,&partial);}return __real_tcsetattr(fd,action,t);}
pid_t __wrap_fork(void){if(++fork_calls==fail_fork){errno=EAGAIN;return -1;}return __real_fork();}
int __wrap_pipe2(int p[2],int flags){if(++pipe_calls==fail_pipe){errno=EMFILE;return -1;}return __real_pipe2(p,flags);}
int __wrap_clock_gettime(clockid_t c,struct timespec *t){int result=__real_clock_gettime(c,t);if(result==0&&jump_clock&&++clock_calls>1)t->tv_sec+=20;return result;}
static void reset(void){get_calls=set_calls=fail_get=fail_set=partial_set=fork_calls=fail_fork=pipe_calls=fail_pipe=clock_calls=0;jump_clock=false;}
static long long millis(void){struct timespec t;CHECK(__real_clock_gettime(CLOCK_MONOTONIC,&t)==0);return (long long)t.tv_sec*1000+t.tv_nsec/1000000;}
static bool equal(const struct termios *a,const struct termios *b){if(a->c_iflag!=b->c_iflag||a->c_oflag!=b->c_oflag||a->c_cflag!=b->c_cflag||a->c_lflag!=b->c_lflag||cfgetispeed(a)!=cfgetispeed(b)||cfgetospeed(a)!=cfgetospeed(b))return false;for(size_t i=0;i<NCCS;++i){if(a->c_cc[i]!=b->c_cc[i])return false;}return true;}
static int descriptors(void){int count=0;DIR *d=opendir("/proc/self/fd");struct dirent *e;CHECK(d);while((e=readdir(d)))if(strcmp(e->d_name,".")&&strcmp(e->d_name,".."))++count;CHECK(closedir(d)==0);return count;}
static void pair(int *master,int *slave){char name[128];*master=posix_openpt(O_RDWR|O_NOCTTY|O_CLOEXEC);CHECK(*master>=0&&grantpt(*master)==0&&unlockpt(*master)==0&&ptsname_r(*master,name,sizeof name)==0);*slave=open(name,O_RDWR|O_NOCTTY|O_CLOEXEC);CHECK(*slave>=0);}
static void wait_child(pid_t p){int status;pid_t actual;do{actual=waitpid(p,&status,0);}while(actual<0&&errno==EINTR);CHECK(actual==p&&WIFEXITED(status)&&WEXITSTATUS(status)==0);}
static int ready(int fd,int ms){struct pollfd p={fd,POLLIN,0};int r;do{r=poll(&p,1,ms);}while(r<0&&errno==EINTR);CHECK(r>=0);return r;}
static void guard(void){
    int master,slave;PTGuard g;struct termios original,actual;pair(&master,&slave);CHECK(__real_tcgetattr(slave,&original)==0);pt_guard_init(&g);
    CHECK(pt_enter(&g,slave,PT_RAW,1,0)==0&&g.restore_pending);CHECK(__real_tcgetattr(slave,&actual)==0&&!(actual.c_lflag&ICANON));CHECK((actual.c_cflag&(CSIZE|PARENB|CSTOPB))==CS8&&cfgetospeed(&actual)==B9600);
    CHECK(pt_restore(&g)==0&&!g.restore_pending&&__real_tcgetattr(slave,&actual)==0&&equal(&actual,&original));CHECK(pt_restore(&g)==0);
    reset();partial_set=1;pt_guard_init(&g);CHECK(pt_enter(&g,slave,PT_RAW,1,0)==EIO&&g.restore_pending);CHECK(pt_restore(&g)==0&&__real_tcgetattr(slave,&actual)==0&&equal(&actual,&original));
    reset();fail_get=1;pt_guard_init(&g);CHECK(pt_enter(&g,slave,PT_RAW,1,0)==EIO&&!g.restore_pending);CHECK(__real_tcgetattr(slave,&actual)==0&&equal(&actual,&original));
    reset();fail_set=1;pt_guard_init(&g);CHECK(pt_enter(&g,slave,PT_RAW,1,0)==EIO&&g.restore_pending);CHECK(pt_restore(&g)==0);
    reset();fail_get=2;pt_guard_init(&g);CHECK(pt_enter(&g,slave,PT_RAW,1,0)==EIO&&g.restore_pending);CHECK(pt_restore(&g)==0);
    reset();fail_set=2;pt_guard_init(&g);CHECK(pt_enter(&g,slave,PT_RAW,1,0)==0);CHECK(pt_restore(&g)==EIO&&g.restore_pending);CHECK(pt_restore(&g)==0&&!g.restore_pending&&__real_tcgetattr(slave,&actual)==0&&equal(&actual,&original));
    reset();pt_guard_init(&g);CHECK(pt_enter(&g,slave,PT_RAW,256,0)==EINVAL&&!g.restore_pending);CHECK(close(slave)==0&&close(master)==0);
}
static void bytes(void){
    int master,slave;PTGuard g;unsigned char b[8];struct termios original,actual;pair(&master,&slave);CHECK(tcgetattr(slave,&original)==0);pt_guard_init(&g);CHECK(pt_enter(&g,slave,PT_CANONICAL,1,0)==0);
    CHECK(write(master,"A",1)==1&&ready(slave,20)==0);CHECK(write(master,"\n",1)==1&&ready(slave,2000)==1&&read(slave,b,sizeof b)==2&&b[0]=='A'&&b[1]=='\n');
    CHECK(write(master,"\0\n",2)==2&&ready(slave,2000)==1&&read(slave,b,sizeof b)==2&&b[0]==0&&b[1]=='\n');CHECK(pt_restore(&g)==0);
    CHECK(pt_enter(&g,slave,PT_RAW,1,0)==0&&write(master,"A\n\0",3)==3);size_t got=0;while(got<3){CHECK(ready(slave,2000)==1);ssize_t n=read(slave,b+got,3-got);CHECK(n>0);got+=(size_t)n;}CHECK(memcmp(b,"A\n\0",3)==0);CHECK(pt_restore(&g)==0&&tcgetattr(slave,&actual)==0&&equal(&original,&actual));CHECK(close(slave)==0&&close(master)==0);
}
/* Observe only our exact child's syscall, so READY-before-read scheduling is
   not mistaken for enrollment in a blocking read. Access denial is a failure
   with evidence, never grounds for changing ptrace/host security settings. */
static void await_read(pid_t child,int fd){char path[80];CHECK(snprintf(path,sizeof path,"/proc/%ld/syscall",(long)child)>0);long long end=millis()+2000;for(;;){FILE *f=fopen(path,"r");long number=-1;unsigned long first=0;CHECK(f);int n=fscanf(f,"%ld %lx",&number,&first);CHECK(fclose(f)==0);if(n==2&&number==SYS_read&&first==(unsigned long)fd)return;CHECK(millis()<end);(void)poll(NULL,0,1);}}
typedef struct{ssize_t count;unsigned char bytes[8];int error;}ReadResult;
static pid_t reader(int slave,int result[2]){CHECK(pipe2(result,O_CLOEXEC)==0);pid_t child=fork();CHECK(child>=0);if(child==0){close(result[0]);ReadResult r={0};r.count=read(slave,r.bytes,sizeof r.bytes);r.error=r.count<0?errno:0;CHECK(write(result[1],&r,sizeof r)==sizeof r);_exit(0);}active_reader=child;CHECK(close(result[1])==0);return child;}
static ReadResult result_of(pid_t child,int fd){ReadResult r={0};CHECK(ready(fd,2500)==1&&read(fd,&r,sizeof r)==sizeof r);wait_child(child);active_reader=0;CHECK(close(fd)==0);return r;}
static void timers(void){
    int master,slave,p[2];PTGuard g;unsigned char b[8];ReadResult r;pid_t child;long long start;pair(&master,&slave);pt_guard_init(&g);
    CHECK(pt_enter(&g,slave,PT_RAW,0,0)==0&&read(slave,b,sizeof b)==0);CHECK(pt_restore(&g)==0);
    CHECK(pt_enter(&g,slave,PT_RAW,2,0)==0);child=reader(slave,p);await_read(child,slave);CHECK(write(master,"A",1)==1&&ready(p[0],100)==0&&write(master,"B",1)==1);r=result_of(child,p[0]);CHECK(r.count==2&&!r.error&&memcmp(r.bytes,"AB",2)==0);CHECK(pt_restore(&g)==0);
    CHECK(pt_enter(&g,slave,PT_RAW,0,1)==0);start=millis();child=reader(slave,p);await_read(child,slave);r=result_of(child,p[0]);CHECK(r.count==0&&!r.error&&millis()-start>=50);CHECK(pt_restore(&g)==0);
    CHECK(pt_enter(&g,slave,PT_RAW,3,1)==0);child=reader(slave,p);await_read(child,slave);CHECK(ready(p[0],150)==0);CHECK(write(master,"A",1)==1&&ready(p[0],60)==0);start=millis();CHECK(write(master,"B",1)==1);r=result_of(child,p[0]);CHECK(r.count==2&&!r.error&&memcmp(r.bytes,"AB",2)==0&&millis()-start>=50);CHECK(pt_restore(&g)==0);
    printf("PT_TIMERS four_cases actual_blocking_read_enrollment interbyte_elapsed_ms=%lld\n",millis()-start);CHECK(close(slave)==0&&close(master)==0);
}
static unsigned unreaped(PTPipeline *r){unsigned unexpected=0;for(unsigned i=0;i<3;++i)if(r->children[i]>0){int status;pid_t p=waitpid(r->children[i],&status,WNOHANG);if(p==r->children[i])++unexpected;else CHECK(p<0&&errno==ECHILD);}return unexpected;}
static uint32_t sum(size_t bytes){uint32_t value=0;for(size_t i=0;i<bytes;++i)value+=(uint32_t)(i%251);return value;}
static void pipeline(void){
    PTPipeline r;int p[2];CHECK(pipe2(p,O_CLOEXEC)==0);int capacity=fcntl(p[0],F_GETPIPE_SZ);CHECK(capacity>0&&close(p[0])==0&&close(p[1])==0);size_t bytes=(size_t)capacity*4+17;CHECK(bytes<=1024*1024);
    CHECK(pt_pipeline(bytes,false,2000,&r)==0);unsigned zombies=unreaped(&r);CHECK(!zombies&&r.reaped_mask==7&&r.bytes==bytes&&r.sum==sum(bytes));for(unsigned i=0;i<3;++i)CHECK(WIFEXITED(r.status[i])&&WEXITSTATUS(r.status[i])==0);
    CHECK(pt_pipeline(bytes,true,2000,&r)==0);zombies=unreaped(&r);CHECK(!zombies&&r.reaped_mask==7&&r.bytes>0&&r.bytes<=512&&r.sum==sum(r.bytes));for(unsigned i=0;i<3;++i)CHECK(WIFEXITED(r.status[i])&&(WEXITSTATUS(r.status[i])==0||WEXITSTATUS(r.status[i])==20));
    CHECK(pt_pipeline(0,false,2000,&r)==0&&!unreaped(&r)&&r.bytes==0&&r.sum==0);
    for(unsigned i=1;i<=3;++i){reset();fail_pipe=i;int before=descriptors();CHECK(pt_pipeline(bytes,false,2000,&r)==EMFILE&&!unreaped(&r)&&descriptors()==before);}
    for(unsigned i=1;i<=3;++i){reset();fail_fork=i;int before=descriptors();CHECK(pt_pipeline(bytes,false,2000,&r)==EAGAIN&&!unreaped(&r)&&descriptors()==before);}
    reset();jump_clock=true;CHECK(pt_pipeline(1024*1024,false,1,&r)==ETIMEDOUT);jump_clock=false;CHECK(!unreaped(&r)&&r.reaped_mask==7);reset();printf("PT_PIPELINE discovered_capacity=%d transfer=%zu fault_ordinals=6\n",capacity,bytes);
}
static void disconnect(void){
    for(unsigned ignore=0;ignore<2;++ignore){int p[2];CHECK(pipe(p)==0&&close(p[0])==0);pid_t child=fork();CHECK(child>=0);if(child==0){struct sigaction action={0};action.sa_handler=ignore?SIG_IGN:SIG_DFL;sigemptyset(&action.sa_mask);CHECK(sigaction(SIGPIPE,&action,NULL)==0);ssize_t n=write(p[1],"X",1);CHECK(ignore&&n<0&&errno==EPIPE);_exit(0);}CHECK(close(p[1])==0);int status;CHECK(waitpid(child,&status,0)==child);CHECK(ignore?(WIFEXITED(status)&&WEXITSTATUS(status)==0):(WIFSIGNALED(status)&&WTERMSIG(status)==SIGPIPE));}
    int master,slave;PTGuard g;unsigned char b;pair(&master,&slave);pt_guard_init(&g);CHECK(pt_enter(&g,slave,PT_RAW,1,0)==0&&close(master)==0);ssize_t n=read(slave,&b,1);int read_error=n<0?errno:0;CHECK(n==0||(n<0&&read_error==EIO));int restore=pt_restore(&g);CHECK(restore!=0&&g.restore_pending);printf("PT_DISCONNECT read=%zd error=%d restore=%d restoration_unavailable_after_peer_loss\n",n,read_error,restore);CHECK(close(slave)==0);
    int sockets[2];CHECK(socketpair(AF_UNIX,SOCK_STREAM|SOCK_CLOEXEC,0,sockets)==0);CHECK(write(sockets[0],"Q",1)==1&&shutdown(sockets[0],SHUT_WR)==0);CHECK(read(sockets[1],&b,1)==1&&b=='Q'&&read(sockets[1],&b,1)==0);
    CHECK(send(sockets[1],"R",1,MSG_NOSIGNAL)==1&&read(sockets[0],&b,1)==1&&b=='R');CHECK(close(sockets[0])==0&&close(sockets[1])==0);
}
int main(int argc,char **argv){int before=descriptors();CHECK(atexit(cleanup_reader)==0);CHECK(argc==2);reset();if(!strcmp(argv[1],"guard"))guard();else if(!strcmp(argv[1],"bytes"))bytes();else if(!strcmp(argv[1],"timers"))timers();else if(!strcmp(argv[1],"pipeline"))pipeline();else if(!strcmp(argv[1],"disconnect"))disconnect();else CHECK(0);CHECK(descriptors()==before);puts("PT_PASS");return 0;}
