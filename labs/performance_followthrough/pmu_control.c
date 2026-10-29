#define _POSIX_C_SOURCE 200809L
#include "pmu_control.h"
#include <errno.h>
#include <limits.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int descriptor(const char *name){
    const char *text=getenv(name);char *end;
    if(!text||!*text){errno=EINVAL;return -1;}
    errno=0;long value=strtol(text,&end,10);
    if(errno||*end||value<0||value>INT_MAX){errno=EINVAL;return -1;}
    return (int)value;
}
int pf_perf_control(int enable){
    int control=descriptor("PF_PERF_CONTROL_FD");
    if(control<0)return -1;
    int ack=descriptor("PF_PERF_ACK_FD");
    if(ack<0)return -1;
    const char *command=enable?"enable\n":"disable\n";
    size_t length=strlen(command),sent=0;
    for(unsigned attempt=0;sent<length&&attempt<8;++attempt){
        ssize_t result=write(control,command+sent,length-sent);
        if(result<0){if(errno==EINTR)continue;return -1;}
        if(!result){errno=EIO;return -1;}
        sent+=(size_t)result;
    }
    if(sent!=length){errno=EINTR;return -1;}
    /* perf's evlist__ctlfd_ack writes sizeof(EVLIST_CTL_CMD_ACK_TAG),
     * including the terminating NUL. Consume it before the next command. */
    char answer[sizeof "ack\n"];size_t used=0;
    for(unsigned attempt=0;used<sizeof answer&&attempt<8;++attempt){
        struct pollfd waiter={ack,POLLIN,0};
        int ready=poll(&waiter,1,1000);
        if(ready<0){if(errno==EINTR)continue;return -1;}
        if(!ready){errno=ETIMEDOUT;return -1;}
        ssize_t got=read(ack,answer+used,sizeof answer-used);
        if(got<0){if(errno==EINTR)continue;return -1;}
        if(!got){errno=EPIPE;return -1;}
        used+=(size_t)got;
    }
    if(used!=sizeof answer||memcmp(answer,"ack\n",sizeof answer)){
        errno=EPROTO;return -1;
    }
    return 0;
}
