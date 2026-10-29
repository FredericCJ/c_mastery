#ifndef DEBUGGING_FOLLOWTHROUGH_H
#define DEBUGGING_FOLLOWTHROUGH_H
#include <stddef.h>
#include <stdint.h>
enum { DX_PASS, DX_FAIL, DX_UNRESOLVED };
struct dx_log { unsigned first, ring[3], count, dropped; };
void dx_log(struct dx_log *, unsigned);
typedef int (*dx_oracle_fn)(uint16_t, void *);
/* The ordered universe has 1..16 chunks; mask preserves their original order.
 * Oracle identity/reset/replay stability are caller obligations. Cache is local
 * to one call, so no classification crosses source/input/seed identity. */
struct dx_reduction { uint16_t mask; unsigned trials, hits, exhausted; };
int dx_reduce(unsigned, unsigned, dx_oracle_fn, void *, struct dx_reduction *);
struct dx_memory { unsigned live, generation, initialized, extent; };
enum { DX_READ, DX_WRITE, DX_FREE, DX_RESIZE_FAIL, DX_RESIZE_SUCCESS };
int dx_memory(struct dx_memory *, unsigned action, unsigned generation, unsigned index,
              unsigned new_extent);
unsigned dx_saturate(unsigned, unsigned);
int dx_checked(unsigned, unsigned, unsigned *);
/* Linux policy releases the modeled descriptor even on EIO/EINTR. The selected
 * POSIX.1-2024 EINTR policy preserves it. Never issue a native close retry here. */
struct dx_descriptor { unsigned live, generation; };
int dx_close(struct dx_descriptor *, unsigned ticket, unsigned policy, int result);
struct dx_writer { size_t offset, total; unsigned attempts; };
/* positive result accepts bytes; -1 EINTR, -2 EAGAIN, 0 no progress; >remaining
 * is an adapter violation. Error returns preserve the accepted offset. */
int dx_write_step(struct dx_writer *, int result, unsigned budget);
#endif
