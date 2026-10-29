#define _POSIX_C_SOURCE 200809L
#include "timing.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
int main(int argc,char **argv){
 if(argc!=3)return 2;
 unsigned char bytes[41];struct ct_stamp stamp={7,3,1000000,4,8};
 if(!strcmp(argv[1],"write")){if(ct_encode(stamp,bytes)!=CT_OK)return 2;int fd=open(argv[2],O_WRONLY|O_CREAT|O_EXCL,0600);if(fd<0)return 2;size_t done=0;while(done<40){ssize_t n=write(fd,bytes+done,40-done);if(n<0&&errno==EINTR)continue;if(n<=0)return 2;done+=(size_t)n;}if(fsync(fd)||close(fd))return 2;puts("CT_PERSIST_WRITE:40 canonical bytes boot=7");return 0;}
 if(strcmp(argv[1],"read"))return 2;
 int fd=open(argv[2],O_RDONLY);if(fd<0)return 2;size_t done=0;while(done<sizeof bytes){ssize_t n=read(fd,bytes+done,sizeof bytes-done);if(n<0&&errno==EINTR)continue;if(n<0)return 2;if(!n)break;done+=(size_t)n;}if(close(fd)||ct_decode(bytes,done,&stamp)!=CT_OK)return 2;struct ct_stamp reset=stamp;reset.boot=8;reset.low=10;uint64_t duration=99;if(ct_duration(stamp,reset,&duration)!=CT_DIFFERENT||duration!=99){puts("CT_PERSIST_CHECK:reset-did-not-invalidate-chronology");return 1;}puts("CT_PERSIST_PASS:saved-boot=7 new-boot=8 duration=DIFFERENT output=99");return 0;
}
