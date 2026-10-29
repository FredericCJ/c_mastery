#ifndef CMASTERY_RESILIENCE_JOURNAL_H
#define CMASTERY_RESILIENCE_JOURNAL_H
#include "resilience.h"
#define RZJ_RECORD_BYTES 64u
#define RZJ_MAX_RECORDS 128u

enum
{
    RZJ_UNCERTAIN = 17,
    RZJ_CORRUPT = 18
};

struct rzj_io
{
    void *context;
    int (*size)(void *, size_t *);
    int (*read)(void *, size_t, unsigned char *, size_t, size_t *);
    int (*write)(void *, size_t, const unsigned char *, size_t, size_t *);
    int (*truncate)(void *, size_t);
    int (*flush)(void *);
};

struct rzj_journal
{
    struct rzj_io io;
    struct rz_receiver receiver;
    uint64_t issued;
    unsigned records;
    bool poisoned;
};

struct rzj_request
{
    unsigned client, id;
    uint64_t fence;
    int delta;
};

struct rzj_replica
{
    int total;
    uint64_t fence, issued;
    unsigned records;
};

struct rzj_record
{
    unsigned sequence, kind;
    uint64_t now;
    struct rzj_request request;
    int result;
    bool duplicate;
};

int rzj_open(struct rzj_io, unsigned, struct rzj_journal *);
int rzj_issue(struct rzj_journal *, uint64_t, uint64_t *);
int rzj_rotate(struct rzj_journal *, uint64_t, uint64_t *);
int rzj_record(const struct rzj_journal *, unsigned, struct rzj_record *);
int rzj_apply(struct rzj_journal *, uint64_t, struct rzj_request, int *,
              bool *);
int rzj_mirror(const struct rzj_journal *, struct rzj_io);
int rzj_replica_read(struct rzj_io, unsigned, struct rzj_replica *);
#endif
