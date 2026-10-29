#ifndef B02_DEPTH_H
#define B02_DEPTH_H
#include <stddef.h>
#include <stdint.h>
enum b2_status { B2_OK, B2_INVALID, B2_RANGE, B2_FULL, B2_ABSENT, B2_STALE, B2_FAILED };
int b2_parse(const unsigned char *bytes, size_t length, int32_t *out);
struct b2_entry { unsigned state; uint16_t key; int value; };
struct b2_map { struct b2_entry slots[7]; size_t count; };
void b2_map_init(struct b2_map *map);
int b2_map_put(struct b2_map *map, uint16_t key, int value);
int b2_map_get(const struct b2_map *map, uint16_t key, int *out);
int b2_map_remove(struct b2_map *map, uint16_t key);
struct b2_item { int key; unsigned serial; };
int b2_sort(struct b2_item *items, size_t length, struct b2_item *scratch, size_t capacity);
struct b2_arena { size_t cursor, depth; unsigned next_id; size_t saved[4]; unsigned ids[4]; };
struct b2_mark { const struct b2_arena *owner; size_t depth; unsigned id; };
void b2_arena_init(struct b2_arena *arena);
int b2_reserve(struct b2_arena *arena, size_t length, size_t alignment, size_t *offset);
int b2_mark(struct b2_arena *arena, struct b2_mark *out);
int b2_rollback(struct b2_arena *arena, struct b2_mark mark);
int b2_q7(const int8_t *a, const int8_t *b, size_t length, int8_t *out);
struct b2_backend { int (*acquire)(void *, unsigned); void (*release)(void *, unsigned); void *context; };
int b2_transaction(struct b2_backend backend, unsigned *published);
struct b2_result { unsigned tag; union { int number; struct { const unsigned char *data; size_t length; } text; int error; } value; };
int b2_render(const struct b2_result *result, unsigned char *out, size_t capacity, size_t *written);
int b2_identity_int(int value);
unsigned b2_identity_unsigned(unsigned value);
#endif
