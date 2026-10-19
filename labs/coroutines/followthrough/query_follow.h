#ifndef CMASTERY_QUERY_FOLLOW_H
#define CMASTERY_QUERY_FOLLOW_H
#include "coroutines.h"
typedef struct {const CORow *rows;size_t count,index;unsigned closed;} CFScan;
typedef struct {CFScan scan;size_t scan_calls,predicate_calls;} CFVolcano;
int cf_volcano_open(CFVolcano *,const CORow *,size_t);
int cf_volcano_next(CFVolcano *,unsigned *,uint32_t *);
int cf_volcano_close(CFVolcano *);
typedef struct {
 const CORow *rows;size_t capacity,available,committed,scan;
 unsigned key,active,kept,complete,closed,eof;uint32_t sum;
} CFBatch;
int cf_batch_open(CFBatch *,const CORow *,size_t);
int cf_batch_poll(CFBatch *,size_t available,unsigned eof,unsigned sink_ready,unsigned *,uint32_t *);
int cf_batch_cancel(CFBatch *);
int cf_order(const CORow *,size_t,unsigned eof,CORow *,size_t,size_t *);
#endif
