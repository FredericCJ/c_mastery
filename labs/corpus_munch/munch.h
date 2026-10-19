#ifndef CORPUS_MUNCH_H
#define CORPUS_MUNCH_H
#include "parsing.h"
#include <limits.h>
_Static_assert(CHAR_BIT==8,"This byte-class contract requires eight-bit bytes");
#define MM_DEAD 255u
/* Byte classes0..3; any map value4 denotes no transition. Rule bits0..7,
 * smallest set bit wins only at equal longest length. No empty tokens. */
typedef struct {
    unsigned count,start;
    unsigned char map[256],next[16][4],accept[16];
} MmDfa;
typedef struct { unsigned begin,end,rule; } MmToken;
typedef struct { MmToken token[256]; unsigned count; } MmTokens;
typedef struct { unsigned transitions,rollback_bytes,memo_stops,marked_pairs; } MmCost;
typedef struct {
    unsigned char failed[257][16],stack_state[257];
    unsigned short stack_position[257];
} MmWorkspace;
int mm_from_rx(const RxDfa *source,MmDfa *out);
/* Mode0 scans with last-accept rollback, mode1 also tabulates failed pairs.
 * Workspace is caller-owned scratch, may change on failure, required for mode1.
 * Token and cost outputs remain unchanged on error. error_offset, if nonnull,
 * receives the failing token start for NO_MATCH/RANGE only. No retained pointers. */
int mm_scan(const MmDfa *dfa,const unsigned char *input,unsigned length,
            unsigned mode,unsigned token_capacity,MmWorkspace *scratch,
            MmTokens *out,MmCost *cost,unsigned *error_offset);
#endif
