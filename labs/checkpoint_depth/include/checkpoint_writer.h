#ifndef CHECKPOINT_WRITER_H
#define CHECKPOINT_WRITER_H
#include <stddef.h>
#include <stdint.h>
typedef enum { CPW_PROGRESS,CPW_RETRY,CPW_ERROR } CPWKind;
typedef struct { CPWKind kind;size_t accepted;int cause; } CPWResult;
typedef enum { CPW_PENDING,CPW_DONE,CPW_FAILED,CPW_INVALID } CPWStatus;
typedef struct { unsigned operation;size_t accepted;int primary,cleanup; } CPWError;
typedef struct { const unsigned char *data;size_t length,accepted;CPWStatus status;CPWError error; } CPWTransfer;
typedef CPWResult (*CPWWrite)(void *,const unsigned char *,size_t);
CPWResult cpw_raw(long);
CPWResult cpw_libc(long,int);
CPWResult cpw_script(long);
CPWStatus cpw_begin(CPWTransfer *,const unsigned char *,size_t,unsigned);
CPWStatus cpw_step(CPWTransfer *,CPWWrite,void *);
CPWStatus cpw_recover(CPWTransfer *);
void cpw_cleanup(CPWTransfer *,int);
#endif
