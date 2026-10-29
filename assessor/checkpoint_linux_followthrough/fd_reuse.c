#define _GNU_SOURCE
#include "checkpoint_linux.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/epoll.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"P5_FD_CHECK line=%d %s\n",__LINE__,#x);exit(1);}}while(0)
int main(void){
 /* Standalone original target owns only its new descriptors. Refuse rather
    than clobber an inherited descriptor when exact fd7 is unavailable. */
 CHECK(fcntl(7,F_GETFD)==-1 && errno==EBADF);
 int ep=epoll_create1(EPOLL_CLOEXEC);CHECK(ep>=0 && ep!=7);
 struct cpa_queue q;struct cpl_reader r;CHECK(cpa_init(&q,8)==CPA_OK);cpl_init(&r,7,3);uint64_t old=0;
 for(unsigned generation=1;generation<=2;generation++){
  int p[2];CHECK(!pipe2(p,O_NONBLOCK|O_CLOEXEC));CHECK(p[0]!=7 && p[1]!=7);
  CHECK(fcntl(p[0],F_DUPFD_CLOEXEC,7)==7 && !close(p[0]));
  CHECK(!cpl_open(&r,ep,7,1) && r.fd==7 && r.generation==generation);
  struct cp_message m={.sequence=(uint16_t)generation,.length=1,.payload={42}};
  unsigned char wire[CP_WIRE];size_t bytes;CHECK(cp_encode(&m,wire,sizeof wire,&bytes)==CP_OK && write(p[1],wire,bytes)==(ssize_t)bytes);
  struct epoll_event event;CHECK(epoll_wait(ep,&event,1,1000)==1 && event.data.u64==cpl_token(&r));
  if(generation==2){CHECK(cpl_pump(&r,old,&q)==CPL_STALE && !q.count && !r.end && !r.cursor && !r.parser.ready);}
  enum cpl_status s;do{s=cpl_pump(&r,event.data.u64,&q);}while(s==CPL_PROGRESS);
  CHECK(s==CPL_DRAINED && q.count==1);struct cpa_handle h;struct cp_message got;CHECK(cpa_take(&q,&h,&got,0)==CPA_OK && got.sequence==generation && cpa_finish(&q,h,0)==CPA_OK);
  old=cpl_token(&r);CHECK(!cpl_close(&r) && !close(p[1]));CHECK(fcntl(7,F_GETFD)==-1 && errno==EBADF);
 }
 CHECK(cpa_close(&q,0)==CPA_CLOSED && cpa_destroy(&q)==CPA_OK && !close(ep));
 puts("P5_FD_PASS actual_fd=7 generations=1,2 stale_rejected_before_read");
}
