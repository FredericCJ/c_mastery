#ifndef PLX_LAUNCH_H
#define PLX_LAUNCH_H
#include <stddef.h>
#include <sys/types.h>
enum { PLX_EXITED, PLX_SIGNALED, PLX_FAILED };
enum { PLX_NORMAL, PLX_SETUP_FAILURE, PLX_SHORT_RECORD, PLX_UNKNOWN_STAGE,
       PLX_EARLY_EXIT, PLX_EARLY_SIGNAL };
typedef struct { int kind, value, stage; } PLXResult;
typedef struct { unsigned mode, chunk; } PLXOptions;
/* Same-binary teaching protocol: stage1 setup, stage2 exec, then errno;
   two little-endian uint32 fields. EOF is not an image certificate. */
int plx_launch(const char *, char *const [], PLXOptions, PLXResult *);
int plx_redirect(const int sources[2], const int destinations[2], unsigned fail);
/* Runs only in an already forked original helper; owns all three descriptors. */
int plx_tree(int writer, int release, int ready, int repair);
int plx_read_all(int, void *, size_t, size_t *, unsigned *);
int plx_write_all(int, const void *, size_t, size_t *, unsigned *);
int plx_wait(pid_t, int *, unsigned *);
#endif
