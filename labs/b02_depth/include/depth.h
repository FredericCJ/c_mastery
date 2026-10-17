#ifndef B02_DEPTH_H
#define B02_DEPTH_H
#include <stddef.h>
#include <stdint.h>
enum B2Status { B2_OK, B2_INVALID, B2_RANGE, B2_FULL, B2_ABSENT, B2_STALE, B2_FAILED };
int b2_parse(const unsigned char *bytes, size_t length, int32_t *out);
typedef struct { unsigned state; uint16_t key; int value; } B2Entry;
typedef struct { B2Entry slots[7]; size_t count; } B2Map;
void b2_map_init(B2Map *map);
int b2_map_put(B2Map *map, uint16_t key, int value);
int b2_map_get(const B2Map *map, uint16_t key, int *out);
int b2_map_remove(B2Map *map, uint16_t key);
typedef struct { int key; unsigned serial; } B2Item;
int b2_sort(B2Item *items, size_t length, B2Item *scratch, size_t capacity);
typedef struct { size_t cursor, depth; unsigned next_id; size_t saved[4]; unsigned ids[4]; } B2Arena;
typedef struct { const B2Arena *owner; size_t depth; unsigned id; } B2Mark;
void b2_arena_init(B2Arena *arena);
int b2_reserve(B2Arena *arena, size_t length, size_t alignment, size_t *offset);
int b2_mark(B2Arena *arena, B2Mark *out);
int b2_rollback(B2Arena *arena, B2Mark mark);
int b2_q7(const int8_t *a, const int8_t *b, size_t length, int8_t *out);
typedef struct { int (*acquire)(void *, unsigned); void (*release)(void *, unsigned); void *context; } B2Backend;
int b2_transaction(B2Backend backend, unsigned *published);
typedef struct { unsigned tag; union { int number; struct { const unsigned char *data; size_t length; } text; int error; } value; } B2Result;
int b2_render(const B2Result *result, unsigned char *out, size_t capacity, size_t *written);
int b2_identity_int(int value);
unsigned b2_identity_unsigned(unsigned value);
#endif
