#ifndef B06_MD_H
#define B06_MD_H
#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#include <math.h>
struct md_arena {uint32_t generation;unsigned live,owner;unsigned char bytes[16];};
struct md_owner {unsigned owned,releases;};
int md_write(unsigned char *arena, size_t capacity, size_t offset, const unsigned char *input, size_t n);
int md_extent(uint32_t count, uint32_t stride, uint32_t limit, uint32_t *out);
int md_read(const struct md_arena *arena, uint32_t generation, unsigned index, unsigned char *out);
int md_authorize(const struct md_arena *arena, unsigned principal, uint32_t generation, unsigned begin, unsigned length);
int md_dispatch(unsigned operation, unsigned grant, unsigned *sentinel);
int md_log_copy(const char *input, size_t n, char *output, size_t capacity);
int md_release(struct md_owner *owner);
int md_equal(const unsigned char *a, const unsigned char *b, size_t n, size_t *steps);
#endif
