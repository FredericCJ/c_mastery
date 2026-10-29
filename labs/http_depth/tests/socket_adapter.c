#define _POSIX_C_SOURCE 200809L
#include "http_depth.h"
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"SOCKET_ADAPTER:%d\n",__LINE__);return 1;}}while(0)
static int flags(int fd){int f=fcntl(fd,F_GETFL),d=fcntl(fd,F_GETFD);return f>=0&&d>=0&&!fcntl(fd,F_SETFL,f|O_NONBLOCK)&&!fcntl(fd,F_SETFD,d|FD_CLOEXEC)&&(fcntl(fd,F_GETFL)&O_NONBLOCK)&&(fcntl(fd,F_GETFD)&FD_CLOEXEC);}
int main(void){
  int pair[2],fd;struct hd_pool p={0};struct hd_handle old,new;unsigned mask=123;
  CHECK(!socketpair(AF_UNIX,SOCK_STREAM,0,pair));CHECK(flags(pair[0]));
  CHECK(!hd_admit(&p,pair[0],1,&old));CHECK(!hd_close(&p,old,&fd));CHECK(fd==pair[0]);CHECK(!close(fd));
  /* dup2 only targets the descriptor this test just owned and closed. */
  CHECK(dup2(pair[1],fd)==fd);CHECK(flags(fd));CHECK(!hd_admit(&p,fd,1,&new));
  CHECK(hd_event(&p,old,fd,1,&mask)==HD_STALE&&mask==123);CHECK(!hd_event(&p,new,fd,1,&mask)&&mask==1);
  CHECK(!hd_close(&p,new,&fd));CHECK(!close(fd));CHECK(!close(pair[1]));
  puts("local socketpair: explicit flags, controlled fd reuse, stale event rejected, owned descriptors closed");return 0;
}
