#ifndef CORPUS_MUNCH_H
#define CORPUS_MUNCH_H
#include "parsing.h"
#include <limits.h>
_Static_assert(CHAR_BIT == 8,
               "This byte-class contract requires eight-bit bytes");
#define MM_DEAD 255u

/* Byte classes0..3; any map value4 denotes no transition. Rule bits0..7,
 * smallest set bit wins only at equal longest length. No empty tokens. */
struct mm_dfa
{
    unsigned count, start;
    unsigned char map[256], next[16][4], accept[16];
};

struct mm_token
{
    unsigned begin, end, rule;
};

struct mm_tokens
{
    struct mm_token token[256];
    unsigned count;
};

struct mm_cost
{
    unsigned transitions, rollback_bytes, memo_stops, marked_pairs;
};

struct mm_workspace
{
    unsigned char failed[257][16], stack_state[257];
    unsigned short stack_position[257];
};

int mm_from_rx(const struct rx_dfa *source, struct mm_dfa *out);
/* Mode0 scans with last-accept rollback, mode1 also tabulates failed pairs.
 * Workspace is caller-owned scratch, may change on failure, required for mode1.
 * Token and cost outputs remain unchanged on error. error_offset, if nonnull,
 * receives the failing token start for NO_MATCH/RANGE only. No retained pointers. */
int mm_scan(const struct mm_dfa *dfa, const unsigned char *input,
            unsigned length, unsigned mode, unsigned token_capacity,
            struct mm_workspace *scratch, struct mm_tokens *out,
            struct mm_cost *cost, unsigned *error_offset);
#endif
