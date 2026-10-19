#ifndef CHECKPOINT_WRITER_FOLLOWTHROUGH_H
#define CHECKPOINT_WRITER_FOLLOWTHROUGH_H
#include "integration.h"
#include "checkpoint_writer.h"
typedef struct {InTransfer active;CPWError error;unsigned failed,recoveries,calls,requeued;} CWFByte;
int cwf_byte_step(algo_ring *,CWFByte *,FWriter,void *,unsigned);
int cwf_byte_recover(CWFByte *);
typedef struct {CPWTransfer transfer;unsigned char bytes[16];unsigned active;size_t count;} CWFSpan;
int cwf_span_begin(CWFSpan *,const unsigned char *,size_t,unsigned);
int cwf_span_take(algo_ring *,CWFSpan *,size_t,unsigned);
CPWStatus cwf_span_step(CWFSpan *,CPWWrite,void *);
CPWStatus cwf_span_recover(CWFSpan *);
typedef enum {CWF_WAIT,CWF_COMPLETE,CWF_IO} CWFCategory;
typedef struct {CWFCategory category;unsigned operation;size_t accepted;int primary,cleanup;} CWFError;
int cwf_error(const CPWTransfer *,CWFError *);
#endif
