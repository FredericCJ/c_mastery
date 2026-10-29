#include "object_runtime_vm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(c)                                               \
    do                                                         \
    {                                                          \
        if (!(c))                                              \
        {                                                      \
            fprintf(stderr, "OR_CHECK:%d:%s\n", __LINE__, #c); \
            return 1;                                          \
        }                                                      \
    } while (0)

static int stop_host(void *context, uint8_t id, int32_t input, int32_t *out)
{
    (void)context;
    (void)id;
    (void)input;
    (void)out;
    return 1;
}

static unsigned registered(const struct or_vm *o)
{
    unsigned i, n = 0;
    for (i = 0; i < 16; i++)
        n += o->roots[i] != NULL;
    return n;
}

static int roots(void)
{
    const char *source =
        "(fn main () (do (pair 1 2) (let p (pair 77 88) (call middle p)))) (fn middle (q) (call leaf q)) (fn leaf (r) (host 0 1))";
    struct cvm_cvm v, before;
    struct cvm_program program;
    struct cvm_value out = cvm_int(999), old, garbage, field;
    struct or_vm o;
    struct orv_stats stats = {0}, prior;
    struct orv_derived a, b, scalar;
    uint64_t tickets[3];
    unsigned i, j, cut;
    cvm_init(&v, 1000, 1000, 100);
    v.host = stop_host;
    CHECK(cvm_compile(source, strlen(source), &program) == CVM_OK);
    CHECK(cvm_run(&v, &program, &out) == CVM_HOST_ERROR && out.integer == 999 &&
          v.depth == 3);
    old = v.frames[0].locals[0];
    garbage = (struct cvm_value){CVM_REF, 0, v.heap[0].generation};
    CHECK(old.tag == CVM_REF && old.integer == 1);
    CHECK(cvm_root_push(&v, old) == CVM_OK);
    v.stack[v.sp++] = old;
    v.stack[v.sp++] = cvm_int(old.integer);
    v.frames[2].locals[7] =
        (struct cvm_value){CVM_REF, 999, 999}; /* uninitialized: not a root */
    orv_init(&o, &v);
    a = (struct orv_derived){old, 0};
    b = (struct orv_derived){old, 2};
    scalar = (struct orv_derived){cvm_int(old.integer), 0};
    CHECK(orv_register(&o, &a, &tickets[0]) == OR_OK &&
          orv_register(&o, &b, &tickets[1]) == OR_OK &&
          orv_register(&o, &scalar, &tickets[2]) == OR_OK);
    CHECK(orv_register(&o, &a, &tickets[0]) == OR_STATE);
    before = v;
    prior = stats;
    for (cut = 1; cut <= 4; cut++)
    {
        CHECK(orv_collect(&o, 1000, cut, &stats) == OR_FULL);
        CHECK(!memcmp(&v, &before, sizeof v) &&
              !memcmp(&stats, &prior, sizeof stats) && a.base.integer == 1 &&
              b.base.integer == 1);
    }
    CHECK(orv_collect(&o, 0, 0, &stats) == OR_FULL &&
          !memcmp(&v, &before, sizeof v));
    CHECK(orv_pause(&o, 0) == OR_OK &&
          orv_collect(&o, 1000, 0, &stats) == OR_STATE &&
          orv_pause(&o, 1) == OR_OK);
    CHECK(orv_collect(&o, 1000, 0, &stats) == OR_OK && stats.live == 1 &&
          stats.moved == 1 && stats.forwarding[1] == 1);
    CHECK(a.base.integer == 0 && b.base.integer == 0 &&
          a.base.generation == 2 && b.offset == 2 && scalar.base.integer == 1);
    CHECK(v.stack[v.sp - 1].tag == CVM_INT && v.stack[v.sp - 1].integer == 1);
    CHECK(cvm_field(&v, old, 0, &field) == CVM_STALE &&
          cvm_field(&v, garbage, 0, &field) == CVM_STALE);
    CHECK(cvm_field(&v, a.base, 0, &field) == CVM_OK && field.integer == 77);
    for (i = 0; i < v.depth; i++)
        for (j = 0; j < CVM_LOCALS; j++)
            if (v.frames[i].initialized[j] &&
                v.frames[i].locals[j].tag == CVM_REF)
                CHECK(cvm_field(&v, v.frames[i].locals[j], 1, &field) ==
                          CVM_OK &&
                      field.integer == 88);
    CHECK(v.frames[2].locals[7].integer == 999);
    for (i = 0; i < 3; i++)
        CHECK(orv_unregister(&o, tickets[i]) == OR_OK);
    CHECK(orv_unregister(&o, tickets[0]) == OR_MISS && registered(&o) == 0);
    /* Full 216-slot map: all initialized copies, not one representative. */
    {
        struct orv_derived extra[16];
        uint64_t ids[16];
        struct cvm_value live = a.base;
        v.root_count = CVM_ROOTS;
        v.sp = CVM_STACK;
        v.depth = CVM_FRAMES;
        for (i = 0; i < CVM_ROOTS; i++)
            v.roots[i] = live;
        for (i = 0; i < CVM_STACK; i++)
            v.stack[i] = i & 1 ? live : cvm_int(0);
        for (i = 0; i < CVM_FRAMES; i++)
            for (j = 0; j < CVM_LOCALS; j++)
            {
                v.frames[i].locals[j] = live;
                v.frames[i].initialized[j] = 1;
            }
        for (i = 0; i < 16; i++)
        {
            extra[i] = (struct orv_derived){live, (uint8_t)(i % 3)};
            CHECK(orv_register(&o, &extra[i], &ids[i]) == OR_OK);
        }
        a = (struct orv_derived){live, 0};
        CHECK(orv_register(&o, &a, &tickets[0]) == OR_FULL);
        CHECK(orv_collect(&o, 1000, 0, &stats) == OR_OK && stats.slots == 216 &&
              stats.frames == 27 && stats.root_copies == 184);
        for (i = 0; i < 16; i++)
            CHECK(orv_unregister(&o, ids[i]) == OR_OK);
    }
    before = v;
    prior = stats;
    v.frames[0].initialized[0] = 2;
    CHECK(orv_collect(&o, 1000, 0, &stats) == OR_INVALID &&
          v.frames[0].initialized[0] == 2 &&
          !memcmp(&stats, &prior, sizeof stats));
    v = before;
    a = (struct orv_derived){v.roots[0], 3};
    CHECK(orv_register(&o, &a, &tickets[0]) == OR_INVALID);
    a.offset = 0;
    CHECK(orv_register(&o, &a, &tickets[0]) == OR_OK);
    a.base.generation++;
    CHECK(orv_collect(&o, 1000, 0, &stats) == OR_INVALID &&
          !memcmp(&v, &before, sizeof v));
    CHECK(orv_unregister(&o, tickets[0]) == OR_OK);
    /* Full-width destination generation and retired holes never wrap. */
    cvm_init(&v, 100, 100, 100);
    orv_init(&o, &v);
    v.heap[0].generation = UINT32_MAX;
    v.heap[1].generation = UINT32_MAX - 1;
    CHECK(cvm_pair(&v, cvm_int(5), cvm_int(6), &old) == CVM_OK &&
          old.integer == 1 && old.generation == UINT32_MAX);
    CHECK(cvm_root_push(&v, old) == CVM_OK &&
          orv_collect(&o, 1000, 0, &stats) == OR_OK &&
          stats.forwarding[1] == 2);
    CHECK(cvm_field(&v, v.roots[0], 0, &field) == CVM_OK && field.integer == 5);
    v.collections = UINT32_MAX;
    before = v;
    CHECK(orv_collect(&o, 1000, 0, &stats) == OR_FULL &&
          !memcmp(&v, &before, sizeof v));
    return 0;
}

struct or_foreign
{
    unsigned mode, cut, calls;
};

static enum or_status callback(struct or_vm *o, struct orv_derived *a,
                               struct orv_derived *b, void *context,
                               struct cvm_value *out)
{
    struct or_foreign *f = context;
    enum or_status s;
    f->calls++;
    if (f->mode == 3)
    {
        struct cvm_value ignored = cvm_int(0);
        if (orv_call(o, a->base, b->base, callback, context, &ignored) !=
                OR_STATE ||
            orv_pause(o, 0) != OR_STATE)
            return OR_INVALID;
    }
    s = orv_pair(o, a, b, 1, f->cut, out);
    if (s)
        return s;
    return f->mode == 2 ? OR_STATE : OR_OK;
}

static int foreign(void)
{
    struct cvm_cvm v;
    struct or_vm o;
    struct cvm_value garbage, child, out, field;
    struct or_foreign context;
    unsigned cut, i;
    struct orv_stats stats;
    for (cut = 0; cut <= 4; cut++)
    {
        cvm_init(&v, 1000, 1000, 100);
        orv_init(&o, &v);
        CHECK(cvm_pair(&v, cvm_int(0), cvm_int(0), &garbage) == CVM_OK &&
              cvm_pair(&v, cvm_int(77), cvm_int(88), &child) == CVM_OK);
        context = (struct or_foreign){0, cut, 0};
        out = cvm_int(999);
        CHECK(orv_call(&o, child, cvm_int(9), callback, &context, &out) ==
              (cut ? OR_FULL : OR_OK));
        CHECK(context.calls == 1 && registered(&o) == 0 && o.foreign == 0);
        if (cut)
            CHECK(out.tag == CVM_INT && out.integer == 999);
        else
        {
            CHECK(out.tag == CVM_REF &&
                  cvm_field(&v, out, 0, &field) == CVM_OK);
            CHECK(cvm_field(&v, field, 0, &field) == CVM_OK &&
                  field.integer == 77);
        }
    }
    /* Failed callback retains its completed allocation, but not output or roots. */
    cvm_init(&v, 1000, 1000, 100);
    orv_init(&o, &v);
    context = (struct or_foreign){2, 0, 0};
    out = cvm_int(999);
    CHECK(orv_call(&o, cvm_int(1), cvm_int(2), callback, &context, &out) ==
              OR_STATE &&
          out.integer == 999 && registered(&o) == 0 && v.allocations == 1);
    CHECK(orv_collect(&o, 1000, 0, &stats) == OR_OK && stats.live == 0);
    context = (struct or_foreign){3, 0, 0};
    CHECK(orv_call(&o, cvm_int(1), cvm_int(2), callback, &context, &out) ==
              OR_OK &&
          context.calls == 1 && registered(&o) == 0);
    /* Actual imported pool: retirement, stale ticket, second admission rollback. */
    {
        struct orv_derived extra[16];
        uint64_t ids[16], retired;
        struct cvm_value value = cvm_int(3);
        orv_init(&o, &v);
        o.registrations.generation[0] = UINT32_MAX;
        extra[0] = (struct orv_derived){value, 0};
        CHECK(orv_register(&o, &extra[0], &retired) == OR_OK &&
              orv_unregister(&o, retired) == OR_OK);
        CHECK(o.registrations.state[0] == 2 &&
              orv_unregister(&o, retired) == OR_MISS);
        for (i = 0; i < 14; i++)
        {
            extra[i] = (struct orv_derived){value, 0};
            CHECK(orv_register(&o, &extra[i], &ids[i]) == OR_OK);
        }
        context = (struct or_foreign){0, 0, 0};
        out = cvm_int(999);
        CHECK(orv_call(&o, value, value, callback, &context, &out) == OR_FULL &&
              context.calls == 0 && registered(&o) == 14 && out.integer == 999);
        for (i = 0; i < 14; i++)
            CHECK(orv_unregister(&o, ids[i]) == OR_OK);
    }
    return 0;
}

/* Python supplies a distinct reachable-set oracle; this C driver emits mapping. */
static int oracle(void)
{
    unsigned code, roots_mask;
    for (code = 0; code < 4096; code++)
        for (roots_mask = 0; roots_mask < 8; roots_mask++)
        {
            struct cvm_cvm v;
            struct or_vm o;
            struct orv_stats stats;
            struct cvm_value handles[3], ignored;
            unsigned i, j, remaining = code;
            cvm_init(&v, 1000, 1000, 100);
            orv_init(&o, &v);
            for (i = 0; i < 3; i++)
            {
                if (cvm_pair(&v, cvm_int((int32_t)(100 + i)), cvm_int(0),
                             &handles[i]) != CVM_OK)
                    return 1;
            }
            for (i = 0; i < 3; i++)
                for (j = 0; j < 2; j++)
                {
                    unsigned edge = remaining % 4;
                    struct cvm_value value;
                    remaining /= 4;
                    value =
                        edge ? handles[edge - 1] : cvm_int((int32_t)(100 + i));
                    if (j)
                        v.heap[i].right = value;
                    else
                        v.heap[i].left = value;
                }
            for (i = 0; i < 3; i++)
                if (roots_mask & (1u << i))
                    if (cvm_root_push(&v, handles[i]) != CVM_OK)
                        return 1;
            {
                struct cvm_cvm baseline = v;
                if (cvm_collect(&baseline) != CVM_OK ||
                    orv_collect(&o, 1000, 0, &stats) != OR_OK)
                    return 1;
                printf("VM %u %u %u", code, roots_mask, stats.live);
                for (i = 0; i < 3; i++)
                {
                    printf(" %u", stats.forwarding[i]);
                    if ((baseline.heap[i].used != 0) !=
                        (stats.forwarding[i] != 0))
                        return 1;
                }
                for (i = 0; i < 3; i++)
                    for (j = 0; j < 2; j++)
                    {
                        if (v.heap[i].used)
                        {
                            struct cvm_value value =
                                j ? v.heap[i].right : v.heap[i].left;
                            printf(" %d", value.tag == CVM_REF
                                              ? value.integer + 1
                                              : value.integer);
                        }
                        else
                            printf(" -1");
                    }
                for (i = 0; i < v.root_count; i++)
                {
                    if (cvm_field(&v, v.roots[i], 0, &ignored) != CVM_OK)
                        return 1;
                }
                puts("");
            }
        }
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    if (!strcmp(argv[1], "roots"))
        return roots();
    if (!strcmp(argv[1], "foreign"))
        return foreign();
    if (!strcmp(argv[1], "oracle"))
        return oracle();
    return 2;
}
