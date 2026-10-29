#define _POSIX_C_SOURCE 200809L
#include "collection.h"
#include <errno.h>
#include <inttypes.h>
#include <poll.h>
#include <stdio.h>
#include <sys/timerfd.h>
#include <time.h>
#include <unistd.h>

static uint64_t now(void) {
  struct timespec t;
  if (clock_gettime(CLOCK_MONOTONIC, &t)) return 0;
  return (uint64_t)t.tv_sec * UINT64_C(1000000000) + (uint64_t)t.tv_nsec;
}

static int exercise(enum tc_kind kind) {
  struct td_timer timers[4];
  size_t index[4], position[4], next[4], previous[4], heads[4];
  struct tc_collection q;
  struct tc_token token, selected;
  struct td_dispatch dispatch;
  int fd=-1, result=1;
  uint64_t base=now(), deadline, max_lateness=0;
  if (!base || tc_init(&q,kind,timers,4,index,position,next,previous,heads,4,1000000,16)) goto done;
  if (tc_arm(&q,3,base+1000000,0,0,&token)!=1 || tc_cancel(&q,token)!=1) goto done;
  if (tc_arm(&q,2,base+3000000,0,0,&token)!=1 ||
      tc_arm(&q,1,base+1000000,0,0,&token)!=1 ||
      tc_arm(&q,0,base+1000000,0,0,&token)!=1) goto done;
  fd=timerfd_create(CLOCK_MONOTONIC,TFD_CLOEXEC|TFD_NONBLOCK);
  if (fd<0) goto done;
  for (size_t completed=0;completed<3;) {
    if (tc_peek(&q,&selected,&deadline)!=1) goto done;
    struct itimerspec arm={.it_value={.tv_sec=(time_t)(deadline/1000000000),.tv_nsec=(long)(deadline%1000000000)}};
    if (timerfd_settime(fd,TFD_TIMER_ABSTIME,&arm,NULL)) goto done;
    struct pollfd p={.fd=fd,.events=POLLIN};
    int ready;
    do { ready=poll(&p,1,2000); } while (ready<0 && errno==EINTR);
    if (ready!=1 || !(p.revents&POLLIN)) goto done;
    uint64_t notifications;
    ssize_t bytes;
    do { bytes=read(fd,&notifications,sizeof notifications); } while (bytes<0 && errno==EINTR);
    if (bytes!=(ssize_t)sizeof notifications || !notifications) goto done;
    uint64_t current=now();
    if (!current) goto done;
    while (tc_begin(&q,current,&selected,&dispatch)==1) {
      if (selected.slot!=completed || dispatch.effects!=1 || current<dispatch.scheduled) goto done;
      uint64_t late=current-dispatch.scheduled;
      if (late>max_lateness) max_lateness=late;
      if (tc_finish(&q,selected)!=1) goto done;
      ++completed;
    }
  }
  if (q.count) goto done;
  printf("{\"case\":\"timerfd-collection\",\"kind\":%d,\"dispatched\":3,\"cancelled\":1,\"max_lateness_ns\":%" PRIu64 ",\"wcet_claim\":false}\n",kind,max_lateness);
  result=0;
done:
  if(fd>=0 && close(fd)) result=1;
  if(result) fprintf(stderr,"COLLECTION_NATIVE_FAILURE:kind=%d errno=%d\n",kind,errno);
  return result;
}
int main(void) {
  for(enum tc_kind kind=TC_SCAN;kind<=TC_WHEEL;++kind) if(exercise(kind)) return 1;
  return 0;
}
