#ifndef CHECKPOINT_WRITER_FOLLOWTHROUGH_H
#define CHECKPOINT_WRITER_FOLLOWTHROUGH_H
#include "integration.h"
#include "checkpoint_writer.h"
struct cwf_byte {struct in_transfer active;struct cpw_error error;unsigned failed,recoveries,calls,requeued;};
int cwf_byte_step(algo_ring *,struct cwf_byte *,f_writer_fn,void *,unsigned);
int cwf_byte_recover(struct cwf_byte *);
struct cwf_span {struct cpw_transfer transfer;unsigned char bytes[16];unsigned active;size_t count;};
int cwf_span_begin(struct cwf_span *,const unsigned char *,size_t,unsigned);
int cwf_span_take(algo_ring *,struct cwf_span *,size_t,unsigned);
enum cpw_status cwf_span_step(struct cwf_span *,cpw_write_fn,void *);
enum cpw_status cwf_span_recover(struct cwf_span *);
enum cwf_category {CWF_WAIT,CWF_COMPLETE,CWF_IO};
struct cwf_error {enum cwf_category category;unsigned operation;size_t accepted;int primary,cleanup;};
int cwf_error(const struct cpw_transfer *,struct cwf_error *);
#endif
