#define _GNU_SOURCE
#include "launch.h"
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdint.h>
#include <sys/wait.h>
#include <unistd.h>
#ifndef PLX_MUTANT
#define PLX_MUTANT 0
#endif
int plx_read_all(int fd,void *buffer,size_t length,size_t *used,unsigned *interrupts){
  if(!used||!interrupts||length>8||(!buffer&&length))return EINVAL;
  *used=0;*interrupts=0;
  while(*used<length){
    ssize_t n=read(fd,(unsigned char *)buffer+*used,length-*used);
    if(n>0)*used+=(size_t)n;
    else if(n==0)return EPROTO;
    else if(errno==EINTR&&PLX_MUTANT!=9)++*interrupts;
    else return errno;
  }
  return 0;
}
int plx_write_all(int fd,const void *buffer,size_t length,size_t *used,unsigned *interrupts){
  if(!used||!interrupts||length>8||(!buffer&&length))return EINVAL;
  *used=0;*interrupts=0;
  while(*used<length){
    ssize_t n=write(fd,(const unsigned char *)buffer+*used,length-*used);
    if(n>0)*used+=(size_t)n;
    else if(n<0&&errno==EINTR)++*interrupts;
    else return n==0?EIO:errno;
  }
  return 0;
}
int plx_wait(pid_t child,int *status,unsigned *interrupts){
  if(child<=0||!status||!interrupts)return EINVAL;
  *interrupts=0;
  for(;;){
    pid_t p=waitpid(child,status,0);
    if(p==child)return 0;
    if(p<0&&errno==EINTR&&PLX_MUTANT!=10){++*interrupts;continue;}
    return p<0?errno:EPROTO;
  }
}
static void put(unsigned char *p,unsigned value){for(unsigned i=0;i<4;++i)p[i]=(unsigned char)(value>>(8*i));}
static uint32_t get(const unsigned char *p){uint32_t v=0;for(unsigned i=0;i<4;++i)v|=(uint32_t)p[i]<<(8*i);return v;}
int plx_launch(const char *path,char *const args[],struct plx_options options,struct plx_result *out){
  if(!path||!args||!args[0]||!out||options.mode>PLX_EARLY_SIGNAL||options.chunk>8)return EINVAL;
  int pipefd[2];if(pipe2(pipefd,O_CLOEXEC)<0)return errno;
  pid_t child=fork();
  if(child<0){int e=errno;close(pipefd[0]);close(pipefd[1]);return e;}
  if(child==0){
    close(pipefd[0]);
    if(options.mode==PLX_EARLY_EXIT)_exit(126);
    if(options.mode==PLX_EARLY_SIGNAL){kill(getpid(),SIGTERM);_exit(125);}
    unsigned stage=1,error=EACCES;
    if(options.mode==PLX_NORMAL){char *const env[]={NULL};execve(path,args,env);stage=2;error=(unsigned)errno;}
    if(options.mode==PLX_UNKNOWN_STAGE)stage=99;
    unsigned char record[8];put(record,stage);put(record+4,error);
    size_t sent=0,length=options.mode==PLX_SHORT_RECORD?3:8;
    while(sent<length){
      size_t n=length-sent;if(options.chunk&&n>options.chunk)n=options.chunk;
      ssize_t count=write(pipefd[1],record+sent,n);
      if(count>0){sent+=(size_t)count;if(PLX_MUTANT==1)break;}
      else if(count<0&&errno==EINTR)continue;
      else break;
    }
    _exit(127);
  }
  close(pipefd[1]);
  unsigned char record[9];size_t used=0;int error=0;
  while(used<sizeof record){
    size_t n=sizeof record-used;if(options.chunk&&n>options.chunk)n=options.chunk;
    ssize_t count=read(pipefd[0],record+used,n);
    if(count>0)used+=(size_t)count;
    else if(count==0)break;
    else if(errno!=EINTR){error=errno;break;}
  }
  close(pipefd[0]);int status;unsigned interruptions;
  int waited=plx_wait(child,&status,&interruptions);
  if(waited)return waited;
  if(error)return error;
  if(used&&used!=8){if(PLX_MUTANT!=2)return EPROTO;used=0;}
  struct plx_result result={0,0,0};
  if(used){
    uint32_t stage=get(record),code=get(record+4);
    if((stage!=1&&stage!=2)&&PLX_MUTANT!=3)return EPROTO;
    if(code==0||code>4095||!WIFEXITED(status)||WEXITSTATUS(status)!=127)return EPROTO;
    result=(struct plx_result){PLX_FAILED,(int)code,(int)stage};
  }else if(WIFEXITED(status))result=(struct plx_result){PLX_EXITED,WEXITSTATUS(status),0};
  else if(WIFSIGNALED(status))result=(struct plx_result){PLX_SIGNALED,WTERMSIG(status),0};
  else return EPROTO;
  if(PLX_MUTANT==4&&!used)result=(struct plx_result){PLX_EXITED,0,0};
  *out=result;return 0;
}
int plx_redirect(const int sources[2],const int destinations[2],unsigned fail){
  if(!sources||!destinations||fail>4||sources[0]<0||sources[1]<0||destinations[0]<0||destinations[1]<0||destinations[0]==destinations[1])return EINVAL;
  int largest=destinations[0]>destinations[1]?destinations[0]:destinations[1];
  if(largest==INT_MAX)return EINVAL;
  int copies[2]={-1,-1},error=0;
  if(PLX_MUTANT==5){for(unsigned i=0;i<2;++i)if(dup2(sources[i],destinations[i])<0)return errno;return 0;}
  for(unsigned i=0;i<2;++i){
    if(fail==i+1){error=EIO;goto done;}
    copies[i]=fcntl(sources[i],F_DUPFD_CLOEXEC,largest+1);
    if(copies[i]<0){error=errno;goto done;}
  }
  for(unsigned i=0;i<2;++i){
    if(fail==i+3){error=EIO;goto done;}
    if(PLX_MUTANT==6&&sources[i]==destinations[i])continue;
    while(dup2(copies[i],destinations[i])<0){if(errno==EINTR)continue;error=errno;goto done;}
  }
done:
  for(unsigned i=0;i<2;++i)if(copies[i]>=0)close(copies[i]);
  return error;
}
int plx_tree(int writer,int release,int ready,int repair){
  if(writer<0||release<0||ready<0||(repair!=0&&repair!=1))return EINVAL;
  pid_t child=fork();if(child<0){int e=errno;close(writer);close(release);close(ready);return e;}
  if(child==0){
    int closed=0;
    if((repair&&PLX_MUTANT!=7)||(!repair&&PLX_MUTANT==8)){close(writer);closed=1;}
    char token='R';size_t used;unsigned interruptions;
    int e=plx_write_all(ready,&token,1,&used,&interruptions);close(ready);
    if(!e)e=plx_read_all(release,&token,1,&used,&interruptions);
    close(release);if(!closed)close(writer);_exit(e?124:0);
  }
  close(writer);close(release);close(ready);int status;unsigned interruptions;
  int e=plx_wait(child,&status,&interruptions);
  return e?e:WIFEXITED(status)&&WEXITSTATUS(status)==0?0:EPROTO;
}
