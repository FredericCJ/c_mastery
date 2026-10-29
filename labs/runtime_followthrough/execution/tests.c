#include "execution.h"
#ifdef VS_STANDALONE
int printf(const char *, ...);
int puts(const char *);
void *memcpy(void *, const void *, size_t);
void *memset(void *, int, size_t);
int memcmp(const void *, const void *, size_t);
#else
#include <stdio.h>
#include <string.h>
#endif
#define CHECK(x)                                                \
    do                                                          \
    {                                                           \
        if (!(x))                                               \
        {                                                       \
            printf("VS_CHECK %u %s\n", (unsigned)__LINE__, #x); \
            return 1;                                           \
        }                                                       \
    } while (0)
static struct vv_work scratch;
static struct vg_vs machine;

static void emit(struct cvm_program *p, unsigned op)
{
    p->code[p->length++] = (uint8_t)op;
}

static void number(struct cvm_program *p, int32_t n)
{
    emit(p, OP_INT);
    uint32_t u = (uint32_t)n;
    for (unsigned i = 0; i < 4; i++)
    {
        emit(p, u & 255);
        u >>= 8;
    }
}

static struct cvm_program binary(unsigned op, int32_t a, int32_t b)
{
    struct cvm_program p = {0};
    p.count = 1;
    number(&p, a);
    number(&p, b);
    emit(&p, op);
    emit(&p, OP_RET);
    p.functions[0].length = p.length;
    return p;
}

static struct cvm_program bytes(const uint8_t *b, unsigned n, unsigned locals)
{
    struct cvm_program p = {0};
    p.count = 1;
    p.length = (uint16_t)n;
    p.functions[0] = (struct cvm_function){0, (uint16_t)n, 0, (uint8_t)locals};
    memcpy(p.code, b, n);
    return p;
}

static int finish(struct vg_vs *s, struct cvm_value *out, unsigned bound)
{
    int status = VS_RUNNING;
    while (status == VS_RUNNING && bound--)
    {
        status = vs_step(s, out);
    }
    return status;
}

static int arithmetic(void)
{
    const struct
    {
        unsigned op;
        int32_t a, b, want;
        int status;
    } cases[] = {{OP_ADD, INT32_MAX, 1, 0, CVM_RANGE},
                 {OP_ADD, INT32_MAX, -1, INT32_MAX - 1, CVM_OK},
                 {OP_SUB, INT32_MIN, 1, 0, CVM_RANGE},
                 {OP_SUB, INT32_MIN, -1, INT32_MIN + 1, CVM_OK},
                 {OP_MUL, 46341, 46341, 0, CVM_RANGE},
                 {OP_MUL, 46340, 46340, 2147395600, CVM_OK},
                 {OP_DIV, INT32_MIN, -1, 0, CVM_RANGE},
                 {OP_DIV, INT32_MIN, 1, INT32_MIN, CVM_OK},
                 {OP_DIV, 1, 0, 0, CVM_RANGE},
                 {OP_DIV, -7, 2, -3, CVM_OK},
                 {OP_DIV, 7, -2, -3, CVM_OK},
                 {OP_DIV, -7, -2, 3, CVM_OK}};

    for (unsigned i = 0; i < sizeof cases / sizeof cases[0]; i++)
    {
        struct cvm_program p = binary(cases[i].op, cases[i].a, cases[i].b);
        struct cvm_cvm original;
        struct cvm_value a = cvm_int(91), b = a;
        cvm_init(&original, 10, 0, 0);
        int old = cvm_run(&original, &p, &a);
        CHECK(old == cases[i].status);
        memset(&machine, 0, sizeof machine);
        CHECK(vs_start(&machine, &p, 0, 10, 0, 0, 0, &scratch) == VS_RUNNING);
        CHECK(vs_step(&machine, &b) == VS_RUNNING && b.integer == 91 &&
              machine.vm.sp == 1 && machine.vm.stack[0].integer == cases[i].a);
        CHECK(vs_step(&machine, &b) == VS_RUNNING && b.integer == 91 &&
              machine.vm.sp == 2 && machine.vm.stack[1].integer == cases[i].b);
        int s = vs_step(&machine, &b);
        CHECK(b.integer == 91);
        if (old == CVM_OK)
        {
            CHECK(s == VS_RUNNING && machine.vm.sp == 1 &&
                  machine.vm.stack[0].integer == cases[i].want);
            CHECK(vs_step(&machine, &b) == VS_DONE &&
                  b.integer == cases[i].want && a.integer == b.integer);
        }
        else
        {
            CHECK(s == old && machine.vm.sp == 0 && a.integer == 91);
            CHECK(vs_step(&machine, &b) == old && b.integer == 91);
        }
    }
    int32_t n = 91;
    CHECK(vs_arithmetic(VS_WRAP_V2, OP_ADD, INT32_MAX, 1, &n) == CVM_OK &&
          n == INT32_MIN);
    CHECK(vs_arithmetic(VS_WRAP_V2, OP_SUB, INT32_MIN, 1, &n) == CVM_OK &&
          n == INT32_MAX);
    CHECK(vs_arithmetic(VS_WRAP_V2, OP_MUL, INT32_MAX, 2, &n) == CVM_OK &&
          n == -2);
    CHECK(vs_arithmetic(VS_WRAP_V2, OP_DIV, INT32_MIN, -1, &n) == CVM_OK &&
          n == INT32_MIN);
    CHECK(vs_arithmetic(VS_WRAP_V2, OP_DIV, 1, 0, &n) == CVM_RANGE &&
          n == INT32_MIN);
    struct cvm_program wrapping = binary(OP_ADD, INT32_MAX, 1);
    struct cvm_value result = cvm_int(91);
    memset(&machine, 0, sizeof machine);
    CHECK(vs_start(&machine, &wrapping, VS_WRAP_V2, 4, 0, 0, 0, &scratch) ==
          VS_RUNNING);
    CHECK(finish(&machine, &result, 4) == VS_DONE &&
          result.integer == INT32_MIN);
    puts("VS_ARITHMETIC_PASS "
         "checked_admission_neighbors_wrapping_version_no_C_overflow");
    return 0;
}

static int failures(void)
{
    const uint8_t wrong[] = {OP_TRUE, OP_FALSE, OP_ADD, OP_RET};
    struct cvm_program p = bytes(wrong, 4, 0);
    struct cvm_value out = cvm_int(91);
    memset(&machine, 0, sizeof machine);
    CHECK(vs_start(&machine, &p, 0, 10, 0, 0, 0, &scratch) == VS_RUNNING);
    CHECK(finish(&machine, &out, 10) == CVM_TYPE && out.integer == 91);
    p.code[0] = OP_POP;
    struct vg_vs before;
    memcpy(&before, &machine, sizeof before);
    CHECK(vs_start(&machine, &p, 0, 10, 0, 0, 0, &scratch) == CVM_VERIFY &&
          !memcmp(&machine, &before, sizeof machine));
    p = binary(OP_ADD, 20, 22);
    for (unsigned fuel = 0; fuel <= 5; fuel++)
    {
        memset(&machine, 0, sizeof machine);
        out = cvm_int(91);
        CHECK(vs_start(&machine, &p, 0, fuel, 0, 0, 0, &scratch) == VS_RUNNING);
        unsigned done = 0;
        int status = VS_RUNNING;
        while (status == VS_RUNNING)
        {
            status = vs_step(&machine, &out);
            if (status == VS_RUNNING || status == VS_DONE)
            {
                done++;
            }
            if (status != VS_DONE)
            {
                CHECK(out.integer == 91);
            }
        }
        CHECK(status == (fuel < 4 ? CVM_FUEL : VS_DONE) &&
              done == (fuel < 4 ? fuel : 4));
        CHECK(machine.vm.fuel == (fuel < 4 ? 0 : fuel - 4));
    }
    p = (struct cvm_program){0};
    p.count = 2;
    for (unsigned i = 0; i < 63; i++)
    {
        emit(&p, OP_NIL);
    }
    emit(&p, OP_CALL);
    emit(&p, 1);
    emit(&p, 0);
    for (unsigned i = 0; i < 63; i++)
    {
        emit(&p, OP_POP);
    }
    emit(&p, OP_RET);
    p.functions[0].length = p.length;
    unsigned at = p.length;
    emit(&p, OP_NIL);
    emit(&p, OP_NIL);
    emit(&p, OP_POP);
    emit(&p, OP_RET);
    p.functions[1] = (struct cvm_function){(uint16_t)at, 4, 0, 0};
    memset(&machine, 0, sizeof machine);
    out = cvm_int(91);
    CHECK(vs_start(&machine, &p, 0, 200, 0, 0, 0, &scratch) == VS_RUNNING);
    CHECK(finish(&machine, &out, 200) == CVM_STACK_LIMIT &&
          machine.vm.sp == 64 && out.integer == 91);
    puts("VS_FAILURE_PASS "
         "TYPE_RANGE_STACK_VERIFY_fuel_each_dispatch_sticky_publication");
    return 0;
}

static int pending(void)
{
    struct cvm_program p = binary(OP_ADD, 20, 22);
    p.code[11] = OP_HOST;
    p.code[12] = 3;
    p.code[13] = OP_RET;
    p.length = p.functions[0].length = 14;
    memset(&machine, 0, sizeof machine);
    struct cvm_value out = cvm_int(91);
    CHECK(vs_start(&machine, &p, 0, 8, 0, 1, 8, &scratch) == VS_RUNNING);
    /* The live program is owned; mutation of the caller's input is irrelevant. */
    p.code[1] = 1;
    CHECK(finish(&machine, &out, 8) == VS_PENDING && out.integer == 91 &&
          machine.vm.fuel == 4 && machine.vm.host_budget == 0);
    struct vs_request r;
    CHECK(vs_complete(&machine, machine.epoch, machine.serial, 0, 90) ==
          VS_STATE);
    CHECK(vs_take(&machine, &r) == VS_PENDING && r.service == 3 &&
          r.argument == 42);
    CHECK(vs_take(&machine, &r) == VS_STATE &&
          vs_step(&machine, &out) == VS_STATE);
    struct vg_vs before;
    memcpy(&before, &machine, sizeof before);
    CHECK(vs_start(&machine, &p, 0, 8, 0, 1, 8, &scratch) == VS_STATE &&
          !memcmp(&machine, &before, sizeof machine));
    CHECK(vs_complete(&machine, r.epoch + 1, r.serial, 0, 99) == VS_STATE &&
          !memcmp(&machine, &before, sizeof machine));
    CHECK(vs_complete(&machine, r.epoch, r.serial + 1, 0, 99) == VS_STATE &&
          !memcmp(&machine, &before, sizeof machine));
    CHECK(vs_complete(&machine, r.epoch, r.serial, 0, 84) == VS_RUNNING &&
          machine.vm.fuel == 4 && machine.vm.host_budget == 0);
    CHECK(vs_complete(&machine, r.epoch, r.serial, 0, 99) == VS_STATE);
    CHECK(vs_step(&machine, &out) == VS_DONE && out.integer == 84 &&
          machine.vm.fuel == 3);
    uint32_t old_epoch = r.epoch;
    p.code[1] = 20;
    out = cvm_int(91);
    CHECK(vs_start(&machine, &p, 0, 8, 0, 1, 8, &scratch) == VS_RUNNING &&
          machine.epoch == old_epoch + 1);
    CHECK(finish(&machine, &out, 8) == VS_PENDING &&
          vs_take(&machine, &r) == VS_PENDING);
    CHECK(vs_complete(&machine, old_epoch, r.serial, 0, 99) == VS_STATE);
    CHECK(vs_complete(&machine, r.epoch, r.serial, 1, 0) == CVM_HOST_ERROR &&
          out.integer == 91 && vs_step(&machine, &out) == CVM_HOST_ERROR);
    CHECK(vs_start(&machine, &p, 0, 8, 0, 1, 0, &scratch) == VS_RUNNING);
    CHECK(finish(&machine, &out, 8) == CVM_HOST_ERROR &&
          machine.vm.host_budget == 0 && vs_take(&machine, &r) == VS_STATE);
    CHECK(vs_start(&machine, &p, 0, 8, 0, 0, 8, &scratch) == VS_RUNNING);
    CHECK(finish(&machine, &out, 8) == CVM_HOST_BUDGET && machine.serial == 0);
    machine.epoch = UINT32_MAX;
    memcpy(&before, &machine, sizeof before);
    CHECK(vs_start(&machine, &p, 0, 8, 0, 1, 8, &scratch) == VS_STATE &&
          !memcmp(&machine, &before, sizeof machine));
    puts("VS_PENDING_PASS "
         "owned_image_once_ticket_epoch_no_reissue_error_after_arithmetic");
    return 0;
}

static struct cvm_program sum(void)
{
    const uint8_t b[] = {
        OP_INT,  100,    0,       0, 0,      OP_INT, 5,      0,
        0,       0,      OP_CALL, 1, 1,      OP_ADD, OP_RET, OP_LOAD,
        0,       OP_INT, 1,       0, 0,      0,      OP_LT,  OP_JFALSE,
        32,      0,      OP_INT,  0, 0,      0,      0,      OP_RET,
        OP_LOAD, 0,      OP_LOAD, 0, OP_INT, 1,      0,      0,
        0,       OP_SUB, OP_CALL, 1, 1,      OP_ADD, OP_RET};
    struct cvm_program p = bytes(b, sizeof b, 0);
    p.count = 2;
    p.functions[0].length = 15;
    p.functions[1] = (struct cvm_function){15, 32, 1, 1};
    return p;
}

static struct cvm_program countdown(unsigned n, unsigned tail, unsigned mutual)
{
    struct cvm_program p = {0};
    p.count = mutual ? 3 : 2;
    number(&p, (int32_t)n);
    number(&p, 0);
    emit(&p, OP_CALL);
    emit(&p, 1);
    emit(&p, 2);
    emit(&p, OP_RET);
    p.functions[0].length = p.length;
    for (unsigned fn = 1; fn < p.count; fn++)
    {
        unsigned start = p.length;
        emit(&p, OP_LOAD);
        emit(&p, 0);
        number(&p, 1);
        emit(&p, OP_LT);
        emit(&p, OP_JFALSE);
        unsigned fix = p.length;
        emit(&p, 0);
        emit(&p, 0);
        emit(&p, OP_LOAD);
        emit(&p, 1);
        emit(&p, OP_RET);
        unsigned target = p.length;
        p.code[fix] = (uint8_t)target;
        p.code[fix + 1] = (uint8_t)(target >> 8);
        emit(&p, OP_LOAD);
        emit(&p, 0);
        number(&p, 1);
        emit(&p, OP_SUB);
        emit(&p, OP_LOAD);
        emit(&p, 1);
        emit(&p, OP_LOAD);
        emit(&p, 0);
        emit(&p, OP_ADD);
        emit(&p, tail ? VS_TAIL : OP_CALL);
        emit(&p, mutual ? (fn == 1 ? 2 : 1) : 1);
        emit(&p, 2);
        emit(&p, OP_RET);
        p.functions[fn] = (struct cvm_function){
            (uint16_t)start, (uint16_t)(p.length - start), 2, 2};
    }
    return p;
}

static int calls(void)
{
    struct cvm_program p = sum();
    struct cvm_value out = cvm_int(91);
    memset(&machine, 0, sizeof machine);
    CHECK(vs_start(&machine, &p, 0, 2000, 0, 0, 0, &scratch) == VS_RUNNING);
    unsigned calls = 0, returns = 0, maxdepth = 1, steps = 0;
    while (machine.state == VS_RUNNING)
    {
        struct cvm_frame *f = &machine.vm.frames[machine.vm.depth - 1];
        unsigned op = machine.image.code[f->ip];
        if (op == OP_CALL)
        {
            calls++;
            if (calls > 1)
            {
                CHECK(f->function == 1 && f->initialized[0] &&
                      f->locals[0].integer == (int32_t)(7 - calls) &&
                      f->base == calls - 1 &&
                      machine.invocation[machine.vm.depth - 1] == calls);
            }
        }
        if (op == OP_RET)
        {
            returns++;
            if (f->function == 1)
            {
                CHECK(f->locals[0].integer == (int32_t)(returns - 1) &&
                      f->base == 7 - returns);
            }
        }
        if (op == OP_CALL || op == OP_RET)
        {
            printf(
                "VS_TRACE step=%u op=%u fn=%u invocation=%u ip=%u base=%u sp=%u "
                "initialized0=%u local0=%u\n",
                steps, op, (unsigned)f->function,
                (unsigned)machine.invocation[machine.vm.depth - 1],
                (unsigned)f->ip, (unsigned)f->base, (unsigned)machine.vm.sp,
                (unsigned)f->initialized[0],
                f->initialized[0] ? (unsigned)f->locals[0].integer : 0u);
        }
        int status = vs_step(&machine, &out);
        CHECK(status == VS_RUNNING || status == VS_DONE);
        if (machine.vm.depth > maxdepth)
        {
            maxdepth = machine.vm.depth;
        }
        steps++;
    }
    CHECK(out.integer == 115 && calls == 6 && returns == 7 && maxdepth == 7 &&
          machine.next_invocation == 7 && steps == 66);
    struct cvm_cvm v;
    cvm_init(&v, 2000, 0, 0);
    struct cvm_value original = cvm_int(91);
    CHECK(cvm_run(&v, &p, &original) == CVM_OK &&
          original.integer == out.integer && 2000 - v.fuel == steps);
    for (unsigned mutual = 0; mutual < 2; mutual++)
    {
        p = countdown(30, 1, mutual);
        memset(&machine, 0, sizeof machine);
        out = cvm_int(91);
        CHECK(vs_start(&machine, &p, 0, 2000, 0, 0, 0, &scratch) == VS_RUNNING);
        maxdepth = 1;
        while (machine.state == VS_RUNNING)
        {
            if (machine.vm.depth > 1)
            {
                unsigned invocation = machine.invocation[machine.vm.depth - 1];
                struct cvm_frame *active =
                    &machine.vm.frames[machine.vm.depth - 1];
                CHECK(active->function ==
                      (mutual ? (invocation % 2 ? 2 : 1) : 1));
                CHECK(active->initialized[0] && active->initialized[1] &&
                      active->locals[0].integer == (int32_t)(32 - invocation));
            }
            CHECK(vs_step(&machine, &out) >= VS_RUNNING);
            if (machine.vm.depth > maxdepth)
            {
                maxdepth = machine.vm.depth;
            }
        }
        CHECK(machine.state == VS_DONE && out.integer == 465 && maxdepth == 2 &&
              machine.next_invocation == 32);
        p = countdown(30, 0, mutual);
        memset(&machine, 0, sizeof machine);
        out = cvm_int(91);
        CHECK(vs_start(&machine, &p, 0, 2000, 0, 0, 0, &scratch) == VS_RUNNING);
        CHECK(finish(&machine, &out, 2000) == CVM_FRAME_LIMIT &&
              machine.vm.depth == 16 && machine.next_invocation == 16 &&
              out.integer == 91);
        p = countdown(5, 0, mutual);
        memset(&machine, 0, sizeof machine);
        CHECK(vs_start(&machine, &p, 0, 2000, 0, 0, 0, &scratch) == VS_RUNNING);
        CHECK(finish(&machine, &out, 2000) == VS_DONE && out.integer == 15);
    }
    p = countdown(1, 0, 0);
    p.code[12] = 1;
    struct vg_vs before;
    memcpy(&before, &machine, sizeof before);
    CHECK(vs_start(&machine, &p, 0, 100, 0, 0, 0, &scratch) == CVM_VERIFY &&
          !memcmp(&machine, &before, sizeof machine));
    puts("VS_CALLS_PASS "
         "recursive_sum_live_caller_trace_mutual_tail_depth2_ordinary16");
    return 0;
}

static int interleave(void)
{
    struct vg_vs automatic = {0};
    struct cvm_program a = binary(OP_ADD, 20, 22), b = binary(OP_MUL, 7, 8);
    struct cvm_value x = cvm_int(91), y = cvm_int(92);
    memset(&machine, 0, sizeof machine);
    CHECK(vs_start(&machine, &a, 0, 20, 0, 0, 0, &scratch) == VS_RUNNING);
    CHECK(vs_start(&automatic, &b, 0, 20, 0, 0, 0, &scratch) == VS_RUNNING);
    for (unsigned i = 0; i < 4; i++)
    {
        CHECK(vs_step(&machine, &x) == (i == 3 ? VS_DONE : VS_RUNNING));
        CHECK(vs_step(&automatic, &y) == (i == 3 ? VS_DONE : VS_RUNNING));
        CHECK(x.integer == (i == 3 ? 42 : 91) &&
              y.integer == (i == 3 ? 56 : 92));
    }
    puts("VS_STORAGE_PASS "
         "automatic_static_disjoint_interleaving_no_scratch_borrow");
    return 0;
}

int main(void)
{
    CHECK(!arithmetic());
    CHECK(!failures());
    CHECK(!pending());
    CHECK(!calls());
    CHECK(!interleave());
    puts("VS_PASS checked_dispatch_owned_continuation_explicit_frames");
    return 0;
}
