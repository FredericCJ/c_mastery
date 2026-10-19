#ifndef CHECKPOINT_LINUX_H
#define CHECKPOINT_LINUX_H
#include "checkpoint_async.h"
#include <stdint.h>
typedef enum { CPL_DRAINED,CPL_BLOCKED,CPL_PROGRESS,CPL_EOF,CPL_TRUNCATED,CPL_STALE,CPL_ERROR } CPLStatus;
typedef struct {
    CPParser parser;unsigned char bytes[64];size_t cursor,end;
    unsigned slot,generation,generation_limit;int fd,epfd,live,retired,eof;
    int last_errno,cleanup_errno;
} CPLReader;
void cpl_init(CPLReader *,unsigned,unsigned);
int cpl_open(CPLReader *,int,int,int);
uint64_t cpl_token(const CPLReader *);
CPLStatus cpl_pump(CPLReader *,uint64_t,CPAQueue *);
int cpl_close(CPLReader *);
#endif
