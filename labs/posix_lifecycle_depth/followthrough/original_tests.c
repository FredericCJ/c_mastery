#define _GNU_SOURCE
#include "posix_lifecycle.h"
#include <dirent.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"PLX_CHECK:%d:%s\n",__LINE__,#x);return 1;}}while(0)
extern int plx_original_mode;
static int descriptors(void){DIR *d=opendir("/proc/self/fd");if(!d)return -1;int n=0;while(readdir(d))++n;closedir(d);return n;}
int main(int argc,char **argv){
 if(argc!=3)return 2;
 char *args[]={argv[2],"127",NULL};struct pl_launch result={PL_SIGNALED,999};int initial=descriptors();
 CHECK(pl_launch(argv[2],args,0,&result)==0);CHECK(result.kind==PL_EXITED&&result.value==127);
 char missing[4096];int n=snprintf(missing,sizeof missing,"%s.absent",argv[2]);CHECK(n>0&&(size_t)n<sizeof missing);CHECK(access(missing,F_OK)==-1&&errno==ENOENT);
 CHECK(pl_launch(missing,args,0,&result)==0);CHECK(result.kind==PL_EXEC_ERROR&&result.value==ENOENT);
 plx_original_mode=3;result=(struct pl_launch){PL_SIGNALED,999};CHECK(pl_launch(missing,args,0,&result)==EPROTO);CHECK(result.kind==PL_SIGNALED&&result.value==999);
 plx_original_mode=1;CHECK(pl_launch(argv[2],args,0,&result)==0);CHECK(result.kind==PL_EXITED&&result.value==126);
 plx_original_mode=2;CHECK(pl_launch(argv[2],args,0,&result)==0);CHECK(result.kind==PL_SIGNALED&&result.value==SIGTERM);
 CHECK(descriptors()==initial);CHECK(waitpid(-1,NULL,WNOHANG)==-1&&errno==ECHILD);
 puts("PLX_PASS original");return 0;
}
