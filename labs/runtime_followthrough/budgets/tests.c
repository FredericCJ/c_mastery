#include "budgets.h"
#include "fixtures.h"
#include "object_runtime_vm.h"
#ifdef VL_STANDALONE
int printf(const char *, ...);
int puts(const char *);
void *memset(void *, int, size_t);
#else
#include <stdio.h>
#include <string.h>
#endif
#define CHECK(x)                                                \
    do                                                          \
    {                                                           \
        if (!(x))                                               \
        {                                                       \
            printf("VL_CHECK %u %s\n", (unsigned)__LINE__, #x); \
            return 1;                                           \
        }                                                       \
    } while (0)
static struct cvm_cvm vm, saved;
static struct vg_vs session;
static struct vv_work scratch;
static unsigned calls;

static int callback(void *ctx, uint8_t id, int32_t n, int32_t *out)
{
    (void)ctx;
    calls++;
    if (id != 1)
        return 1;
    *out = n + 11;
    return 0;
}

static void emit(struct cvm_program *p, unsigned x)
{
    p->code[p->length++] = (uint8_t)x;
}

static void number(struct cvm_program *p, int32_t x)
{
    emit(p, OP_INT);
    uint32_t u = (uint32_t)x;
    for (unsigned i = 0; i < 4; i++)
    {
        emit(p, u & 255);
        u >>= 8;
    }
}

static void finish(struct cvm_program *p)
{
    p->count = 1;
    p->functions[0] = (struct cvm_function){0, p->length, 0, 0};
}

static void initialize(unsigned f, unsigned g, unsigned h)
{
    cvm_init(&vm, f, g, h);
    vm.host = callback;
    calls = 0;
}

static int units(void)
{
    struct cvm_program p = {0};
    struct cvm_value out = cvm_int(91);
    struct vl_report r;
    number(&p, 7);
    emit(&p, OP_RET);
    finish(&p);
    initialize(10, 50, 3);
    CHECK(vl_run(&vm, &p, &out, &r) == CVM_OK && out.integer == 7);
    CHECK(r.fuel == 2 && r.gc == 0 && r.host == 0 && r.allocations == 0 &&
          r.collections == 0);
    p = (struct cvm_program){0};
    emit(&p, OP_GC);
    number(&p, 7);
    emit(&p, OP_RET);
    finish(&p);
    initialize(10, 50, 3);
    CHECK(vl_run(&vm, &p, &out, &r) == CVM_OK && r.fuel == 3 && r.gc == 16 &&
          r.host == 0 && r.collections == 1);
    p = (struct cvm_program){0};
    number(&p, 7);
    emit(&p, OP_HOST);
    emit(&p, 1);
    emit(&p, OP_RET);
    finish(&p);
    initialize(10, 50, 3);
    CHECK(vl_run(&vm, &p, &out, &r) == CVM_OK && out.integer == 18 &&
          r.fuel == 3 && r.gc == 0 && r.host == 1 && calls == 1);
    p = vl_composed();
    initialize(9, 19, 1);
    CHECK(vl_run(&vm, &p, &out, &r) == CVM_OK && out.integer == 18);
    CHECK(r.fuel == 9 && r.gc == 19 && r.host == 1 && r.allocations == 1 &&
          r.collections == 1 && calls == 1);
    /* Paid HOST may have effects even if the following RET runs out of fuel. */
    initialize(8, 19, 1);
    out = cvm_int(91);
    CHECK(vl_run(&vm, &p, &out, &r) == CVM_FUEL);
    CHECK(r.host == 1 && calls == 1);
    CHECK(out.integer == 91);
    initialize(5, 18, 1);
    out = cvm_int(91);
    CHECK(vl_run(&vm, &p, &out, &r) == CVM_GC_BUDGET && r.fuel == 5 &&
          r.gc == 3 && r.collections == 0);
    CHECK(out.integer == 91 && calls == 0);
    puts("VL_UNITS_PASS separate_ledgers_refusal_charge_composition");
    return 0;
}

static int limits(void)
{
    struct cvm_program p = {0};
    struct cvm_value out = cvm_int(91);
    struct vl_report r;
    number(&p, 1);
    emit(&p, OP_RET);
    finish(&p);
    initialize(0, 100, 1);
    CHECK(vl_run(&vm, &p, &out, &r) == CVM_FUEL && r.fuel == 0 &&
          out.integer == 91);
    initialize(1, 100, 1);
    CHECK(vl_run(&vm, &p, &out, &r) == CVM_FUEL && r.fuel == 1 &&
          out.integer == 91);
    /* Valid self-recursion has a frame result contract, but never reaches RET. */
    p = (struct cvm_program){{OP_CALL, 0, 0, OP_RET}, 4, 1, {{0, 4, 0, 0}}};
    initialize(100, 100, 1);
    CHECK(vl_run(&vm, &p, &out, &r) == CVM_FRAME_LIMIT && vm.depth == 16 &&
          r.fuel == 16 && out.integer == 91);
    p = (struct cvm_program){0};
    for (unsigned i = 0; i < 63; i++)
        number(&p, 0);
    emit(&p, OP_CALL);
    emit(&p, 1);
    emit(&p, 0);
    for (unsigned i = 0; i < 63; i++)
        emit(&p, OP_POP);
    emit(&p, OP_RET);
    finish(&p);
    unsigned start = p.length;
    number(&p, 1);
    number(&p, 2);
    emit(&p, OP_ADD);
    emit(&p, OP_RET);
    p.count = 2;
    p.functions[1] = (struct cvm_function){(uint16_t)start,
                                           (uint16_t)(p.length - start), 0, 0};
    initialize(1000, 100, 1);
    CHECK(vl_run(&vm, &p, &out, &r) == CVM_STACK_LIMIT && vm.sp == 64 &&
          vm.depth == 2 && r.fuel == 66 && out.integer == 91);
    p = (struct cvm_program){0};
    number(&p, 0);
    number(&p, 0);
    emit(&p, OP_PAIR);
    emit(&p, OP_RET);
    finish(&p);
    initialize(100, 100, 1);
    struct cvm_value child = cvm_int(0);
    for (unsigned i = 0; i < 16; i++)
        CHECK(cvm_pair(&vm, child, cvm_int(0), &child) == CVM_OK);
    CHECK(cvm_root_push(&vm, child) == CVM_OK);
    CHECK(vl_run(&vm, &p, &out, &r) == CVM_HEAP_LIMIT && r.fuel == 3 &&
          r.gc == 51 && r.collections == 1 && out.integer == 91);
    initialize(100, 100, 1);
    vm.arena = 1;
    for (unsigned i = 0; i < 16; i++)
        CHECK(cvm_pair(&vm, cvm_int(0), cvm_int(0), &child) == CVM_OK);
    CHECK(vl_run(&vm, &p, &out, &r) == CVM_HEAP_LIMIT && r.gc == 0 &&
          out.integer == 91);
    vm.arena = 0;
    for (unsigned i = 0; i < 7; i++)
        CHECK(cvm_root_push(&vm, cvm_int(0)) == CVM_OK);
    CHECK(vl_run(&vm, &p, &out, &r) == CVM_STACK_LIMIT && vm.root_count == 7 &&
          vm.sp == 0 && r.gc == 0 && out.integer == 91);
    p = (struct cvm_program){
        {OP_GC, OP_INT, 0, 0, 0, 0, OP_RET}, 7, 1, {{0, 7, 0, 0}}};
    initialize(100, 0, 1);
    CHECK(vl_run(&vm, &p, &out, &r) == CVM_GC_BUDGET && r.fuel == 1 &&
          r.gc == 0 && out.integer == 91);
    p = (struct cvm_program){
        {OP_INT, 0, 0, 0, 0, OP_HOST, 1, OP_RET}, 8, 1, {{0, 8, 0, 0}}};
    initialize(100, 100, 0);
    CHECK(vl_run(&vm, &p, &out, &r) == CVM_HOST_BUDGET && r.fuel == 2 &&
          r.host == 0 && calls == 0 && out.integer == 91);
    /* Changed assumption: same instruction, two exhausted limits; fetch fuel
     * wins. */
    initialize(1, 100, 0);
    CHECK(vl_run(&vm, &p, &out, &r) == CVM_FUEL && r.host == 0 && calls == 0);
    initialize(2, 100, 0);
    CHECK(vl_run(&vm, &p, &out, &r) == CVM_HOST_BUDGET && r.host == 0 &&
          calls == 0);
    /* Wrong tag precedes host-budget check after paid instruction fetch. */
    p = (struct cvm_program){
        {OP_TRUE, OP_HOST, 1, OP_RET}, 4, 1, {{0, 4, 0, 0}}};
    initialize(2, 100, 0);
    CHECK(vl_run(&vm, &p, &out, &r) == CVM_TYPE && r.fuel == 2 && r.host == 0 &&
          calls == 0);
    puts("VL_LIMITS_PASS seven_limits_independent_reinit_two_near_limits");
    return 0;
}

static int changed_collector(void)
{
    initialize(100, 100, 1);
    struct cvm_value values[3];
    for (unsigned i = 0; i < 3; i++)
        CHECK(cvm_pair(&vm, cvm_int((int32_t)i), cvm_int(0), &values[i]) ==
              CVM_OK);
    vm.heap[2].left = values[1];
    CHECK(cvm_root_push(&vm, values[2]) == CVM_OK);
    saved = vm;
    CHECK(cvm_collect(&vm) == CVM_OK && vm.gc_budget == 79); /* 1+2*2+16 */
    vm = saved;
    struct or_vm adapter;
    orv_init(&adapter, &vm);
    struct orv_stats stats = {0};
    stats.work = 91;
    CHECK(orv_collect(&adapter, 24, 0, &stats) == OR_FULL && stats.work == 91 &&
          vm.heap[0].used);
    CHECK(orv_collect(&adapter, 25, 0, &stats) == OR_OK); /* 1+2*3+16+2 */
    CHECK(stats.work == 25 && stats.slots == 1 && stats.live == 2 &&
          stats.moved == 2 && vm.gc_budget == 100);
    CHECK(vm.roots[0].integer == 1 && vm.heap[1].left.integer == 0);
    puts("VL_COLLECTOR_PASS original21_moving25_distinct_units");
    return 0;
}

static int pending(void)
{
    struct cvm_program p = {
        {OP_INT, 7, 0, 0, 0, OP_HOST, 1, OP_RET}, 8, 1, {{0, 8, 0, 0}}};
    struct cvm_value out = cvm_int(91);
    struct vl_report r;
    memset(&session, 0, sizeof session);
    CHECK(vs_start(&session, &p, 0, 3, 0, 1, 2, &scratch) == VS_RUNNING);
    CHECK(vl_step(&session, &out, &r) == VS_RUNNING && r.fuel == 1 &&
          out.integer == 91);
    CHECK(vl_step(&session, &out, &r) == VS_PENDING && r.fuel == 1 &&
          r.host == 1 && out.integer == 91);
    CHECK(vl_step(&session, &out, &r) == VS_STATE && r.fuel == 0 &&
          r.host == 0 && out.integer == 91);
    struct vs_request request;
    CHECK(vs_take(&session, &request) == VS_PENDING);
    CHECK(vs_start(&session, &p, 0, 3, 0, 1, 2, &scratch) == VS_STATE);
    CHECK(vs_complete(&session, request.epoch, request.serial, 0, 18) ==
          VS_RUNNING);
    CHECK(vl_step(&session, &out, &r) == VS_DONE && r.fuel == 1 &&
          r.host == 0 && out.integer == 18);
    puts("VL_PENDING_PASS retained_state_no_step_charge_while_pending");
    return 0;
}

int main(void)
{
    if (units() || limits() || changed_collector() || pending())
        return 1;
    puts("VL_PASS independent_limits_distinct_work_pending");
    return 0;
}
