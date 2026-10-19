#define _POSIX_C_SOURCE 200809L
#include "flow.h"
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
/* Learner-owned Linux adapter. These named seams let the exported failure
   harness compile before implementation; they intentionally fail admission. */
uint64_t now_ms(void){return 0;}
int flags(int fd){(void)fd;return -1;}
int listener_init(unsigned short port,unsigned fault,int *out,struct sockaddr_in *published){(void)port;(void)fault;(void)out;(void)published;return -1;}
int write_socket(void *context,const void *bytes,size_t n){(void)context;(void)bytes;(void)n;return -3;}
int main(int argc,char **argv){(void)argc;(void)argv;return 2;}
