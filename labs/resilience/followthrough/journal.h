#ifndef CMASTERY_RESILIENCE_JOURNAL_H
#define CMASTERY_RESILIENCE_JOURNAL_H
#include "resilience.h"
#define RZJ_RECORD_BYTES 64u
#define RZJ_MAX_RECORDS 128u
enum {RZJ_UNCERTAIN=17,RZJ_CORRUPT=18};
typedef struct {
    void *context;
    int (*size)(void *,size_t *);
    int (*read)(void *,size_t,unsigned char *,size_t,size_t *);
    int (*write)(void *,size_t,const unsigned char *,size_t,size_t *);
    int (*truncate)(void *,size_t);
    int (*flush)(void *);
} RzjIO;
typedef struct {
    RzjIO io;
    RZReceiver receiver;
    uint64_t issued;
    unsigned records;
    bool poisoned;
} RzjJournal;
typedef struct {unsigned client,id;uint64_t fence;int delta;} RzjRequest;
typedef struct {int total;uint64_t fence,issued;unsigned records;} RzjReplica;
typedef struct {unsigned sequence,kind;uint64_t now;RzjRequest request;int result;bool duplicate;} RzjRecord;
int rzj_open(RzjIO,unsigned,RzjJournal *);
int rzj_issue(RzjJournal *,uint64_t,uint64_t *);
int rzj_rotate(RzjJournal *,uint64_t,uint64_t *);
int rzj_record(const RzjJournal *,unsigned,RzjRecord *);
int rzj_apply(RzjJournal *,uint64_t,RzjRequest,int *,bool *);
int rzj_mirror(const RzjJournal *,RzjIO);
int rzj_replica_read(RzjIO,unsigned,RzjReplica *);
#endif
