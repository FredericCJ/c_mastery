#ifndef B06_CC_H
#define B06_CC_H
#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#include <math.h>

struct cc_ins
{
    unsigned op;
    uint32_t value;
};

int cc_token(const char *s, size_t n, size_t *offset, unsigned *kind,
             uint32_t *value);
int cc_parse(const char *source, size_t n, struct cc_ins *ir, size_t capacity,
             size_t *used);
int cc_binary(unsigned op, uint32_t a, uint32_t b, uint32_t *out);
int cc_run(const struct cc_ins *ir, size_t n, uint32_t x, uint32_t *out);
int cc_validate(const struct cc_ins *ir, size_t n, unsigned *peak);
int cc_x86(const struct cc_ins *ir, size_t n, char *out, size_t capacity);
int cc_thumb(const struct cc_ins *ir, size_t n, char *out, size_t capacity);
int cc_compile_eval(const char *s, size_t n, uint32_t x, uint32_t *out);
#endif
