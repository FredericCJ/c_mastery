#ifndef CMASTERY_POSIX_LIFECYCLE_H
#define CMASTERY_POSIX_LIFECYCLE_H
typedef enum { PL_EXITED, PL_SIGNALED, PL_EXEC_ERROR } PLKind;
typedef struct { PLKind kind; int value; } PLLaunch;
typedef struct { unsigned created, joined, completed, sum; int create_error; } PLPool;
/* See CONTRACTS.md: caller-owned live/disjoint storage; no external child reaper. */
int pl_launch(const char *path, char *const argv[], unsigned fail_stage, PLLaunch *out);
int pl_pool(unsigned workers, unsigned fail_ordinal, PLPool *out);
#endif
