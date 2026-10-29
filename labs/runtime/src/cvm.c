#include "cvm.h"

/* Learner implementation. Stubs compile; contract tests must fail until
 * implemented. */
struct cvm_value cvm_int(int32_t n)
{
    struct cvm_value v = {CVM_INT, n, 0};
    return v;
}

void cvm_init(struct cvm_cvm *v, uint32_t f, uint32_t g, uint32_t h)
{
    (void)v;
    (void)f;
    (void)g;
    (void)h;
}

enum cvm_status cvm_decode(const uint8_t *b, size_t n, struct cvm_program *p)
{
    (void)b;
    (void)n;
    (void)p;
    return CVM_NOT_IMPLEMENTED;
}

enum cvm_status cvm_encode(const struct cvm_program *p, uint8_t *b, size_t n,
                           size_t *w)
{
    (void)p;
    (void)b;
    (void)n;
    (void)w;
    return CVM_NOT_IMPLEMENTED;
}

enum cvm_status cvm_load(const uint8_t *b, size_t n, struct cvm_program *p)
{
    (void)b;
    (void)n;
    (void)p;
    return CVM_NOT_IMPLEMENTED;
}

enum cvm_status cvm_save(const struct cvm_program *p, uint8_t *b, size_t n,
                         size_t *w)
{
    (void)p;
    (void)b;
    (void)n;
    (void)w;
    return CVM_NOT_IMPLEMENTED;
}

enum cvm_status cvm_verify(const struct cvm_program *p)
{
    (void)p;
    return CVM_NOT_IMPLEMENTED;
}

enum cvm_status cvm_run(struct cvm_cvm *v, const struct cvm_program *p,
                        struct cvm_value *r)
{
    (void)v;
    (void)p;
    (void)r;
    return CVM_NOT_IMPLEMENTED;
}

enum cvm_status cvm_collect(struct cvm_cvm *v)
{
    (void)v;
    return CVM_NOT_IMPLEMENTED;
}

enum cvm_status cvm_pair(struct cvm_cvm *v, struct cvm_value a,
                         struct cvm_value b, struct cvm_value *r)
{
    (void)v;
    (void)a;
    (void)b;
    (void)r;
    return CVM_NOT_IMPLEMENTED;
}

enum cvm_status cvm_root_push(struct cvm_cvm *v, struct cvm_value a)
{
    (void)v;
    (void)a;
    return CVM_NOT_IMPLEMENTED;
}

void cvm_root_pop(struct cvm_cvm *v)
{
    (void)v;
}

enum cvm_status cvm_field(struct cvm_cvm *v, struct cvm_value r, int right,
                          struct cvm_value *out)
{
    (void)v;
    (void)r;
    (void)right;
    (void)out;
    return CVM_NOT_IMPLEMENTED;
}

const char *cvm_status_name(enum cvm_status s)
{
    static const char *const names[] = {
        "OK",         "FORMAT",      "VERIFY",         "TYPE",
        "RANGE",      "STACK_LIMIT", "FRAME_LIMIT",    "FUEL",
        "HEAP_LIMIT", "GC_BUDGET",   "HOST_BUDGET",    "HOST_ERROR",
        "STALE",      "SYNTAX",      "NOT_IMPLEMENTED"};
    return (unsigned)s < sizeof names / sizeof names[0] ? names[s] : "UNKNOWN";
}
