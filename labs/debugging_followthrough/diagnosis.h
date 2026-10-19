#ifndef DEBUGGING_FOLLOWTHROUGH_H
#define DEBUGGING_FOLLOWTHROUGH_H
#include <stddef.h>
#include <stdint.h>
enum { DX_PASS, DX_FAIL, DX_UNRESOLVED };
typedef struct { unsigned first, ring[3], count, dropped; } DxLog;
void dx_log(DxLog *, unsigned);
typedef int (*DxOracle)(uint16_t, void *);
/* The ordered universe has 1..16 chunks; mask preserves their original order.
 * Oracle identity/reset/replay stability are caller obligations. Cache is local
 * to one call, so no classification crosses source/input/seed identity. */
typedef struct { uint16_t mask; unsigned trials, hits, exhausted; } DxReduction;
int dx_reduce(unsigned, unsigned, DxOracle, void *, DxReduction *);
typedef struct { unsigned live, generation, initialized, extent; } DxMemory;
enum { DX_READ, DX_WRITE, DX_FREE, DX_RESIZE_FAIL, DX_RESIZE_SUCCESS };
int dx_memory(DxMemory *, unsigned action, unsigned generation, unsigned index,
              unsigned new_extent);
unsigned dx_saturate(unsigned, unsigned);
int dx_checked(unsigned, unsigned, unsigned *);
/* Linux policy releases the modeled descriptor even on EIO/EINTR. The selected
 * POSIX.1-2024 EINTR policy preserves it. Never issue a native close retry here. */
typedef struct { unsigned live, generation; } DxDescriptor;
int dx_close(DxDescriptor *, unsigned ticket, unsigned policy, int result);
typedef struct { size_t offset, total; unsigned attempts; } DxWriter;
/* positive result accepts bytes; -1 EINTR, -2 EAGAIN, 0 no progress; >remaining
 * is an adapter violation. Error returns preserve the accepted offset. */
int dx_write_step(DxWriter *, int result, unsigned budget);
#endif
