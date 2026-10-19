#define _GNU_SOURCE
#include <errno.h>
#include <signal.h>
#include <stddef.h>
#include <sys/types.h>
#include <unistd.h>
int plx_original_mode;
int __real_execve(const char *,char *const [],char *const []);
ssize_t __real_write(int,const void *,size_t);
static unsigned written;
int __wrap_execve(const char *p,char *const a[],char *const e[]){
  if(plx_original_mode==1)_exit(126);
  if(plx_original_mode==2){kill(getpid(),SIGTERM);_exit(125);}
  written=0;return __real_execve(p,a,e);
}
/* A complete native-int record may require retries; the short mode deliberately
   reports one byte then makes no progress. It is an original protocol defect. */
ssize_t __wrap_write(int fd,const void *p,size_t n){
  if(!written++){errno=EINTR;return -1;}
  if(plx_original_mode==3&&written>2)return 0;
  return __real_write(fd,p,n>1?1:n);
}
