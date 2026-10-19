/* Deterministic syscall-boundary negative schedule, separate from real signals. */
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
ssize_t __real_read(int,void *,size_t);
ssize_t __real___read_chk(int,void *,size_t,size_t);
ssize_t __real_write(int,const void *,size_t);
pid_t __real_waitpid(pid_t,int *,int);
static pid_t owner;
static unsigned reads,writes,waits;
static void refresh(void){pid_t p=getpid();if(owner!=p){owner=p;reads=writes=waits=0;}}
ssize_t __wrap_read(int fd,void *p,size_t n){refresh();if(!reads++){errno=EINTR;return -1;}return __real_read(fd,p,n>1?1:n);}
/* Optimized glibc fortified calls preserve their destination extent check. */
ssize_t __wrap___read_chk(int fd,void *p,size_t n,size_t extent){refresh();if(!reads++){errno=EINTR;return -1;}return __real___read_chk(fd,p,n>1?1:n,extent);}
#ifndef PLX_ORIGINAL
ssize_t __wrap_write(int fd,const void *p,size_t n){refresh();if(!writes++){errno=EINTR;return -1;}return __real_write(fd,p,n>1?1:n);}
#endif
pid_t __wrap_waitpid(pid_t p,int *s,int flags){refresh();if(!waits++){errno=EINTR;return -1;}return __real_waitpid(p,s,flags);}
int plx_injection_observed(void){printf("INJECT parent read_calls=%u wait_calls=%u\n",reads,waits);return reads>1&&waits>1;}
