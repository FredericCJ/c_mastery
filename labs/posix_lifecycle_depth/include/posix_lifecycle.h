#ifndef CMASTERY_POSIX_LIFECYCLE_H
#define CMASTERY_POSIX_LIFECYCLE_H
enum pl_kind { PL_EXITED, PL_SIGNALED, PL_EXEC_ERROR };
struct pl_launch { enum pl_kind kind; int value; };
struct pl_pool { unsigned created, joined, completed, sum; int create_error; };
/* See CONTRACTS.md: caller-owned live/disjoint storage; no external child reaper. */
int pl_launch(const char *path, char *const argv[], unsigned fail_stage, struct pl_launch *out);
int pl_pool(unsigned workers, unsigned fail_ordinal, struct pl_pool *out);
#endif
