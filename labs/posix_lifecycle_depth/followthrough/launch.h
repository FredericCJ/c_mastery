#ifndef PLX_LAUNCH_H
#define PLX_LAUNCH_H
#include <stddef.h>
#include <sys/types.h>

enum
{
    PLX_EXITED,
    PLX_SIGNALED,
    PLX_FAILED
};

enum
{
    PLX_NORMAL,
    PLX_SETUP_FAILURE,
    PLX_SHORT_RECORD,
    PLX_UNKNOWN_STAGE,
    PLX_EARLY_EXIT,
    PLX_EARLY_SIGNAL
};

struct plx_result
{
    int kind, value, stage;
};

struct plx_options
{
    unsigned mode, chunk;
};

/* Same-binary teaching protocol: stage1 setup, stage2 exec, then errno;
   two little-endian uint32 fields. EOF is not an image certificate.
   Returns 0 with *result filled, EINVAL for bad arguments, EPROTO for a
   malformed record or an impossible wait status, or the errno of a failed
   system call. *result is untouched on failure. Both pipe ends are closed
   before return. The child is reaped before return unless waiting fails with
   an error other than EINTR; that error is returned and the child stays
   unreaped. Blocks until the child ends.
   options.chunk limits each transfer to that many bytes (0 = no limit). */
int plx_launch(const char *, char *const[], struct plx_options,
               struct plx_result *);
/* Makes destinations[i] a duplicate of sources[i] for both pairs, correct
   even when the pairs cross. It is not an atomic two-descriptor transaction:
   a failure after the first install may leave destination 0 changed and
   destination 1 unchanged. All descriptors are borrowed. `fail` (0..4)
   injects EIO before the numbered step. Returns 0 or an errno. */
int plx_redirect(const int sources[2], const int destinations[2],
                 unsigned fail);
/* Runs only in an already forked original helper; owns all three descriptors. */
int plx_tree(int writer, int release, int ready, int repair);
/* Transfer exactly `length` (<= 8) bytes, retrying after EINTR. *used is the
   byte count and *interrupts counts EINTR returns. EOF before `length`
   bytes is EPROTO (read) or EIO (zero-byte write). */
int plx_read_all(int, void *, size_t, size_t *, unsigned *);
int plx_write_all(int, const void *, size_t, size_t *, unsigned *);
/* Waits for exactly `child`, retrying after EINTR (counted in *interrupts). */
int plx_wait(pid_t, int *, unsigned *);
#endif
