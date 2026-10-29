#include "core.h"
int cc_token(const char *s, size_t n, size_t *offset, unsigned *kind, uint32_t *value) { return 0; }
int cc_parse(const char *source, size_t n, struct cc_ins *ir, size_t capacity, size_t *used) { return 0; }
int cc_binary(unsigned op, uint32_t a, uint32_t b, uint32_t *out) { return 0; }
int cc_run(const struct cc_ins *ir, size_t n, uint32_t x, uint32_t *out) { return 0; }
int cc_validate(const struct cc_ins *ir, size_t n, unsigned *peak) { return 0; }
int cc_x86(const struct cc_ins *ir, size_t n, char *out, size_t capacity) { return 0; }
int cc_thumb(const struct cc_ins *ir, size_t n, char *out, size_t capacity) { return 0; }
int cc_compile_eval(const char *s, size_t n, uint32_t x, uint32_t *out) { return 0; }
