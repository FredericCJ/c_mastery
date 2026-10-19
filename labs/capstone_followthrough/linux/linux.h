#ifndef CAPSTONE_LINUX_H
#define CAPSTONE_LINUX_H
#include "service.h"
#include <stdint.h>
typedef struct { int fd,live,ready,eof;unsigned generation; } CsSession;
typedef struct { CsService *core;int epoll;CsSession session[2]; } CsLinux;
typedef struct { size_t bytes;CsStatus status;int error; } CsTurn;
void csl_init(CsLinux *,CsService *,int);
uint64_t csl_token(const CsLinux *,unsigned);
int csl_open(CsLinux *,unsigned,int,int);
int csl_close(CsLinux *,unsigned);
CsStatus csl_event(CsLinux *,uint64_t);
CsTurn csl_turn(CsLinux *,unsigned,size_t,int);
CsIO csl_write(void *,const unsigned char *,size_t);
#endif
