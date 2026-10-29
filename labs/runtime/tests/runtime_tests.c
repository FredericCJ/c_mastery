#include "cvm.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                         \
    do                                                                   \
    {                                                                    \
        if (!(x))                                                        \
        {                                                                \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #x); \
            return 1;                                                    \
        }                                                                \
    } while (0)
#define STATUS(e, w)                                                         \
    do                                                                       \
    {                                                                        \
        enum cvm_status got = (e);                                           \
        if (got != (w))                                                      \
        {                                                                    \
            fprintf(stderr, "FAIL %s:%d: %s got %s expected %s\n", __FILE__, \
                    __LINE__, #e, cvm_status_name(got), cvm_status_name(w)); \
            return 1;                                                        \
        }                                                                    \
    } while (0)

static struct cvm_program simple(void)
{
    struct cvm_program p = {0};
    p.count = 1;
    p.length = 6;
    p.functions[0].length = 6;
    p.code[0] = OP_INT;
    p.code[1] = 42;
    p.code[5] = OP_RET;
    return p;
}

static enum cvm_status eval(const char *source, struct cvm_cvm *v,
                            struct cvm_value *r)
{
    struct cvm_program p;
    enum cvm_status s = cvm_compile(source, strlen(source), &p);
    return s == CVM_OK ? cvm_run(v, &p, r) : s;
}

static int wire(void)
{
    struct cvm_program p = simple(), q = {0};
    uint8_t b[1200] = {0};
    size_t n = 0;
    unsigned char before[sizeof q];
    const uint8_t expected[] = {'C', 'V', 'M',    '1', 1, 0, 0, 6,     0,
                                0,   0,   OP_INT, 42,  0, 0, 0, OP_RET};
    STATUS(cvm_encode(&p, b, sizeof b, &n), CVM_OK);
    CHECK(n == sizeof expected && !memcmp(b, expected, n));
    STATUS(cvm_decode(b, n, &q), CVM_OK);
    CHECK(q.count == 1 && q.length == 6 && q.code[1] == 42);
    memcpy(before, &q, sizeof q);
    for (size_t i = 0; i < n; i++)
    {
        STATUS(cvm_decode(b, i, &q), CVM_FORMAT);
        CHECK(!memcmp(before, &q, sizeof q));
    }
    b[0] = 'X';
    STATUS(cvm_decode(b, n, &q), CVM_FORMAT);
    b[0] = 'C';
    b[4] = 9;
    STATUS(cvm_decode(b, n, &q), CVM_FORMAT);
    b[4] = 1;
    uint8_t prior[sizeof b];
    memcpy(prior, b, sizeof b);
    size_t written_before = n;
    STATUS(cvm_encode(&p, b, 1, &n), CVM_RANGE);
    CHECK(n == written_before && !memcmp(prior, b, sizeof b));
    /* Signed immediates remain bytes here; execution belongs to dispatch. */
    p.code[1] = p.code[2] = p.code[3] = 0;
    p.code[4] = 128;
    STATUS(cvm_encode(&p, b, sizeof b, &n), CVM_OK);
    CHECK(b[12] == 0 && b[13] == 0 && b[14] == 0 && b[15] == 128);
    STATUS(cvm_decode(b, n, &q), CVM_OK);
    CHECK(!memcmp(p.code, q.code, p.length));
    /* Structurally bounded images can still contain invalid instructions. */
    p.code[0] = 255;
    STATUS(cvm_encode(&p, b, sizeof b, &n), CVM_OK);
    STATUS(cvm_decode(b, n, &q), CVM_OK);
    CHECK(q.code[0] == 255);
    p.functions[0].length = 7;
    STATUS(cvm_encode(&p, b, sizeof b, &n), CVM_FORMAT);
    return 0;
}

static int image_verifier(void)
{
    struct cvm_program p = simple(), q;
    uint8_t b[1200] = {0}, prior[1200];
    size_t n = 0;
    STATUS(cvm_save(&p, b, sizeof b, &n), CVM_OK);
    STATUS(cvm_load(b, n, &q), CVM_OK);
    CHECK(q.code[1] == 42 && q.length == 6);
    unsigned char before[sizeof q];
    memcpy(before, &q, sizeof q);
    b[11] = OP_POP;
    STATUS(cvm_load(b, n, &q), CVM_VERIFY);
    CHECK(!memcmp(before, &q, sizeof q));
    p.code[0] = OP_POP;
    memcpy(prior, b, sizeof b);
    size_t old = n;
    STATUS(cvm_save(&p, b, sizeof b, &n), CVM_VERIFY);
    CHECK(n == old && !memcmp(prior, b, sizeof b));
    return 0;
}

static int verifier(void)
{
    struct cvm_program p = simple();
    STATUS(cvm_verify(&p), CVM_OK);
    p.code[0] = OP_POP;
    STATUS(cvm_verify(&p), CVM_VERIFY);
    p = simple();
    p.code[0] = OP_JMP;
    p.code[1] = 1;
    p.code[2] = 0;
    p.code[3] = OP_TRUE;
    p.code[4] = OP_POP;
    STATUS(cvm_verify(&p), CVM_VERIFY);
    p = simple();
    p.code[5] = OP_GC;
    STATUS(cvm_verify(&p), CVM_VERIFY);
    p = simple();
    p.functions[0].arity = 1;
    STATUS(cvm_verify(&p), CVM_VERIFY);
    p = (struct cvm_program){0};
    p.count = 1;
    p.length = 10;
    p.functions[0].length = 10;
    {
        const uint8_t code[] = {OP_TRUE, OP_JFALSE, 8, 0,     OP_TRUE,
                                OP_JMP,  9,         0, OP_GC, OP_RET};
        memcpy(p.code, code, sizeof code);
    }
    STATUS(cvm_verify(&p), CVM_VERIFY);
    p = simple();
    p.functions[0].locals = 1;
    p.code[0] = OP_LOAD;
    p.code[1] = 1;
    p.code[2] = OP_GC;
    p.code[3] = OP_GC;
    p.code[4] = OP_GC;
    STATUS(cvm_verify(&p), CVM_VERIFY);
    p = simple();
    p.code[0] = OP_CALL;
    p.code[1] = 0;
    p.code[2] = 1;
    p.code[3] = OP_GC;
    p.code[4] = OP_GC;
    STATUS(cvm_verify(&p), CVM_VERIFY);
    return 0;
}

static void emit_int(struct cvm_program *p, int32_t value)
{
    uint32_t bits = (uint32_t)value;
    p->code[p->length++] = OP_INT;
    for (unsigned i = 0; i < 4; ++i)
    {
        p->code[p->length++] = (uint8_t)bits;
        bits >>= 8;
    }
}

static struct cvm_program binary(unsigned opcode, int32_t a, int32_t b)
{
    struct cvm_program p = {0};
    p.count = 1;
    emit_int(&p, a);
    emit_int(&p, b);
    p.code[p.length++] = (uint8_t)opcode;
    p.code[p.length++] = OP_RET;
    p.functions[0].length = p.length;
    return p;
}

static int arithmetic(void)
{
    struct cvm_cvm v;
    struct cvm_value r;
    struct cvm_program p = binary(OP_ADD, 20, 22);
    cvm_init(&v, 100, 0, 0);
    STATUS(cvm_run(&v, &p, &r), CVM_OK);
    CHECK(r.integer == 42);

    struct
    {
        unsigned op;
        int32_t a, b;
    } cases[] = {{OP_ADD, INT32_MAX, 1},
                 {OP_SUB, INT32_MIN, 1},
                 {OP_MUL, INT32_MAX, 2},
                 {OP_DIV, INT32_MIN, -1},
                 {OP_DIV, 1, 0}};

    for (unsigned i = 0; i < sizeof cases / sizeof cases[0]; i++)
    {
        p = binary(cases[i].op, cases[i].a, cases[i].b);
        cvm_init(&v, 100, 0, 0);
        STATUS(cvm_run(&v, &p, &r), CVM_RANGE);
    }
    p = (struct cvm_program){0};
    p.count = 1;
    p.code[p.length++] = OP_TRUE;
    emit_int(&p, 1);
    p.code[p.length++] = OP_ADD;
    p.code[p.length++] = OP_RET;
    p.functions[0].length = p.length;
    cvm_init(&v, 100, 0, 0);
    STATUS(cvm_run(&v, &p, &r), CVM_TYPE);
    p = binary(OP_DIV, -7, 2);
    cvm_init(&v, 100, 0, 0);
    STATUS(cvm_run(&v, &p, &r), CVM_OK);
    CHECK(r.integer == -3);
    return 0;
}

static int calls(void)
{
    struct cvm_cvm v;
    struct cvm_value r;
    struct cvm_program p = {0};
    p.count = 2;
    const uint8_t code[] = {
        OP_INT,  5,       0,      0,      0,     OP_CALL, 1, 1,      OP_RET,
        OP_LOAD, 0,       OP_INT, 1,      0,     0,       0, OP_LT,  OP_JFALSE,
        26,      0,       OP_INT, 0,      0,     0,       0, OP_RET, OP_LOAD,
        0,       OP_LOAD, 0,      OP_INT, 1,     0,       0, 0,      OP_SUB,
        OP_CALL, 1,       1,      OP_ADD, OP_RET};
    memcpy(p.code, code, sizeof code);
    p.length = sizeof code;
    p.functions[0] = (struct cvm_function){0, 9, 0, 0};
    p.functions[1] = (struct cvm_function){9, 32, 1, 1};
    cvm_init(&v, 2000, 0, 0);
    STATUS(cvm_run(&v, &p, &r), CVM_OK);
    CHECK(r.integer == 15);
    p = (struct cvm_program){0};
    p.count = 2;
    p.length = 8;
    const uint8_t recursive[] = {OP_CALL, 1, 0, OP_RET, OP_CALL, 1, 0, OP_RET};
    memcpy(p.code, recursive, sizeof recursive);
    p.functions[0] = (struct cvm_function){0, 4, 0, 0};
    p.functions[1] = (struct cvm_function){4, 4, 0, 0};
    cvm_init(&v, 10000, 0, 0);
    STATUS(cvm_run(&v, &p, &r), CVM_FRAME_LIMIT);
    p.code[2] = 1;
    STATUS(cvm_verify(&p), CVM_VERIFY);
    return 0;
}

static int compiler(void)
{
    struct cvm_cvm v;
    struct cvm_value r;
    struct cvm_program p;
    cvm_init(&v, 1000, 1000, 0);
    STATUS(eval("(fn main () (let x 2 (+ (let x 40 x) x)))", &v, &r), CVM_OK);
    CHECK(r.integer == 42);
    cvm_init(&v, 1000, 1000, 0);
    STATUS(eval("(fn main () (call add 40 2)) (fn add (a b) (+ a b))", &v, &r),
           CVM_OK);
    CHECK(r.integer == 42);
    const char *bad[] = {"(fn main () x)",
                         "(fn main () 2147483648)",
                         "(fn main () (+ 1))",
                         "(fn main () 1) trailing",
                         "(fn main () 1) (fn main () 2)",
                         "(fn main () (let x x x))",
                         "(fn main () (call unknown))",
                         "(fn main () (host 256 1))",
                         "(fn main () (let true 0 true))"};
    for (unsigned i = 0; i < sizeof bad / sizeof bad[0]; i++)
    {
        p = simple();
        STATUS(cvm_compile(bad[i], strlen(bad[i]), &p), CVM_SYNTAX);
        CHECK(p.code[1] == 42 && p.length == 6);
    }
    const char nul_keyword[] = "(fn main () true\0junk)";
    const char nul_identifier[] = "(fn main\0junk () 1)";
    const char non_ascii[] = "(fn main () true\xff)";

    const struct
    {
        const char *bytes;
        size_t length;
    } spans[] = {{nul_keyword, sizeof nul_keyword - 1},
                 {nul_identifier, sizeof nul_identifier - 1},
                 {non_ascii, sizeof non_ascii - 1}};

    for (unsigned i = 0; i < sizeof spans / sizeof spans[0]; i++)
    {
        unsigned char before[sizeof p];
        p = simple();
        memcpy(before, &p, sizeof p);
        STATUS(cvm_compile(spans[i].bytes, spans[i].length, &p), CVM_SYNTAX);
        CHECK(!memcmp(before, &p, sizeof p));
    }
    return 0;
}

static int control(void)
{
    struct cvm_cvm v;
    struct cvm_value r;
    struct cvm_program p = simple();
    const char *counter =
        "(fn main () (let n 0 (do (while (< n 5) (set n (+ n 1))) n)))";
    cvm_init(&v, 1000, 0, 0);
    STATUS(eval(counter, &v, &r), CVM_OK);
    CHECK(r.integer == 5);
    cvm_init(&v, 100, 0, 0);
    STATUS(eval("(fn main () (if true 42 7))", &v, &r), CVM_OK);
    CHECK(r.integer == 42);
    cvm_init(&v, 100, 0, 0);
    STATUS(eval("(fn main () (if false 42 7))", &v, &r), CVM_OK);
    CHECK(r.integer == 7);
    cvm_init(&v, 100, 0, 0);
    STATUS(eval("(fn main () (if 1 2 3))", &v, &r), CVM_TYPE);
    cvm_init(&v, 100, 0, 0);
    STATUS(eval("(fn main () (while true nil))", &v, &r), CVM_FUEL);
    const char *unknown = "(fn main () (set unknown 1))";
    STATUS(cvm_compile(unknown, strlen(unknown), &p), CVM_SYNTAX);
    CHECK(p.length == 6 && p.code[1] == 42);
    return 0;
}

static int arena(void)
{
    struct cvm_cvm v;
    struct cvm_value r;
    cvm_init(&v, 100, 1000, 0);
    v.arena = 1;
    for (unsigned i = 0; i < CVM_HEAP; i++)
        STATUS(cvm_pair(&v, cvm_int((int32_t)i), cvm_int(0), &r), CVM_OK);
    STATUS(cvm_pair(&v, cvm_int(0), cvm_int(0), &r), CVM_HEAP_LIMIT);
    CHECK(v.allocations == CVM_HEAP);
    cvm_init(&v, 100, 1000, 0);
    v.arena = 1;
    STATUS(cvm_pair(&v, cvm_int(1), cvm_int(2), &r), CVM_OK);
    CHECK(v.allocations == 1);
    struct cvm_value field;
    STATUS(cvm_field(&v, r, 0, &field), CVM_OK);
    CHECK(field.integer == 1);
    return 0;
}

static int gc(void)
{
    struct cvm_cvm v;
    struct cvm_value root, child, garbage, r;
    struct cvm_value nil = {CVM_NIL, 0, 0};
    cvm_init(&v, 1000, 10000, 0);
    STATUS(cvm_pair(&v, cvm_int(42), nil, &child), CVM_OK);
    STATUS(cvm_pair(&v, child, nil, &root), CVM_OK);
    STATUS(cvm_root_push(&v, root), CVM_OK);
    STATUS(cvm_collect(&v), CVM_OK);
    STATUS(cvm_field(&v, root, 0, &r), CVM_OK);
    STATUS(cvm_field(&v, r, 0, &r), CVM_OK);
    CHECK(r.integer == 42);
    /* A cycle remains live through its root and is reclaimed once that root
     * leaves. */
    v.heap[root.integer].right = root;
    STATUS(cvm_collect(&v), CVM_OK);
    cvm_root_pop(&v);
    STATUS(cvm_collect(&v), CVM_OK);
    STATUS(cvm_field(&v, root, 0, &r), CVM_STALE);
    /* The only reference to child is a C temporary passed into allocating pair().
     */
    STATUS(cvm_pair(&v, cvm_int(77), nil, &child), CVM_OK);
    for (unsigned i = 1; i < CVM_HEAP; i++)
        STATUS(cvm_pair(&v, cvm_int((int32_t)i), nil, &garbage), CVM_OK);
    STATUS(cvm_pair(&v, child, nil, &root), CVM_OK);
    STATUS(cvm_field(&v, root, 0, &r), CVM_OK);
    STATUS(cvm_field(&v, r, 0, &r), CVM_OK);
    CHECK(r.integer == 77);
    v.gc_budget = 0;
    STATUS(cvm_collect(&v), CVM_GC_BUDGET);
    STATUS(cvm_field(&v, root, 0, &r), CVM_OK);
    /* Frame roots via hand-encoded code; no later compiler extension needed. */
    struct cvm_program p = {0};
    p.count = 1;
    p.functions[0].locals = 1;
    emit_int(&p, 11);
    emit_int(&p, 22);
    const uint8_t tail[] = {OP_PAIR,  OP_STORE, 0, OP_GC,     OP_LOAD, 0,
                            OP_FIRST, OP_LOAD,  0, OP_SECOND, OP_ADD,  OP_RET};
    memcpy(p.code + p.length, tail, sizeof tail);
    p.length += sizeof tail;
    p.functions[0].length = p.length;
    cvm_init(&v, 1000, 10000, 0);
    STATUS(cvm_run(&v, &p, &r), CVM_OK);
    CHECK(r.integer == 33);
    cvm_init(&v, 100, 1000, 0);
    v.arena = 1;
    STATUS(cvm_collect(&v), CVM_HEAP_LIMIT);
    return 0;
}

static int fuel(void)
{
    struct cvm_cvm v;
    struct cvm_value r;
    struct cvm_program p = {0};
    p.count = 1;
    const uint8_t code[] = {OP_TRUE, OP_JFALSE, 9, 0,      OP_NIL, OP_POP,
                            OP_JMP,  0,         0, OP_NIL, OP_RET};
    memcpy(p.code, code, sizeof code);
    p.length = sizeof code;
    p.functions[0].length = p.length;
    cvm_init(&v, 50, 999, 7);
    STATUS(cvm_run(&v, &p, &r), CVM_FUEL);
    CHECK(v.fuel == 0 && v.host_budget == 7 && v.gc_budget == 999);
    cvm_init(&v, 0, 99, 0);
    p = simple();
    STATUS(cvm_run(&v, &p, &r), CVM_FUEL);
    return 0;
}

static int callback(void *ctx, uint8_t service, int32_t arg, int32_t *out)
{
    unsigned *count = ctx;
    (*count)++;
    if (service != 1)
        return -1;
    *out = arg;
    return 0;
}

static int host(void)
{
    struct cvm_cvm v;
    struct cvm_value r;
    unsigned count = 0;
    struct cvm_program p = simple();
    p.code[5] = OP_HOST;
    p.code[6] = 1;
    p.code[7] = OP_RET;
    p.length = p.functions[0].length = 8;
    cvm_init(&v, 100, 0, 1);
    v.host = callback;
    v.host_context = &count;
    STATUS(cvm_run(&v, &p, &r), CVM_OK);
    CHECK(count == 1 && r.integer == 42 && v.host_budget == 0);
    cvm_init(&v, 100, 0, 0);
    v.host = callback;
    v.host_context = &count;
    STATUS(cvm_run(&v, &p, &r), CVM_HOST_BUDGET);
    CHECK(count == 1);
    cvm_init(&v, 100, 0, 1);
    v.host = callback;
    v.host_context = &count;
    p.code[6] = 2;
    STATUS(cvm_run(&v, &p, &r), CVM_HOST_ERROR);
    CHECK(count == 2);
    return 0;
}

static int integration(void)
{
    struct cvm_cvm v;
    struct cvm_value r;
    unsigned count = 0;
    cvm_init(&v, 1000, 10000, 1);
    v.host = callback;
    v.host_context = &count;
    STATUS(eval("(fn main () (let p (pair 11 22) (do (gc) (host 1 (+ (first p) "
                "(second p))))))",
                &v, &r),
           CVM_OK);
    CHECK(r.integer == 33 && count == 1 && v.host_budget == 0);
    return 0;
}

static int portability(void)
{
    struct cvm_program p;
    struct cvm_cvm v;
    struct cvm_value r;
    uint8_t wire[1200];
    size_t n;
    const char *source = "(fn main () -2147483648)";
    STATUS(cvm_compile(source, strlen(source), &p), CVM_OK);
    STATUS(cvm_save(&p, wire, sizeof wire, &n), CVM_OK);
    CHECK(wire[12] == 0 && wire[13] == 0 && wire[14] == 0 && wire[15] == 128);
    STATUS(cvm_load(wire, n, &p), CVM_OK);
    cvm_init(&v, 10, 0, 0);
    STATUS(cvm_run(&v, &p, &r), CVM_OK);
    CHECK(r.integer == INT32_MIN);
    return 0;
}

int main(int argc, char **argv)
{
    struct
    {
        const char *name;
        int (*fn)(void);
    } cases[] = {{"wire", wire},
                 {"image_verifier", image_verifier},
                 {"verifier", verifier},
                 {"arithmetic", arithmetic},
                 {"calls", calls},
                 {"compiler", compiler},
                 {"control", control},
                 {"arena", arena},
                 {"gc", gc},
                 {"fuel", fuel},
                 {"host", host},
                 {"integration", integration},
                 {"portability", portability}};

    if (argc != 2)
        return 2;
    for (unsigned i = 0; i < sizeof cases / sizeof cases[0]; i++)
        if (!strcmp(argv[1], cases[i].name))
            return cases[i].fn();
    return 2;
}
