#ifndef CHECKPOINT_LINUX_H
#define CHECKPOINT_LINUX_H
#include "checkpoint_async.h"
#include <stdint.h>

enum cpl_status
{
    CPL_DRAINED,
    CPL_BLOCKED,
    CPL_PROGRESS,
    CPL_EOF,
    CPL_TRUNCATED,
    CPL_STALE,
    CPL_ERROR
};

struct cpl_reader
{
    struct cp_parser parser;
    unsigned char bytes[64];
    size_t cursor, end;
    unsigned slot, generation, generation_limit;
    int fd, epfd, live, retired, eof;
    int last_errno, cleanup_errno;
};

void cpl_init(struct cpl_reader *, unsigned, unsigned);
int cpl_open(struct cpl_reader *, int, int, int);
uint64_t cpl_token(const struct cpl_reader *);
enum cpl_status cpl_pump(struct cpl_reader *, uint64_t, struct cpa_queue *);
int cpl_close(struct cpl_reader *);
#endif
