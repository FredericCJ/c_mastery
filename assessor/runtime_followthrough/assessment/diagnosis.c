#include "cvm.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                \
    do                                                          \
    {                                                           \
        if (!(x))                                               \
        {                                                       \
            printf("VP_CHECK %u %s\n", (unsigned)__LINE__, #x); \
            return 1;                                           \
        }                                                       \
    } while (0)

static struct cvm_program p(const uint8_t *b, size_t n)
{
    struct cvm_program x = {0};
    memcpy(x.code, b, n);
    x.count = 1;
    x.length = (uint16_t)n;
    x.functions[0] = (struct cvm_function){0, (uint16_t)n, 0, 0};
    return x;
}

static int verifier(void)
{
    const uint8_t good[] = {OP_INT, 0, 0, 0, 0, OP_RET}, bad[] = {OP_RET};
    struct cvm_program a = p(good, sizeof good), b = p(bad, sizeof bad);
    CHECK(cvm_verify(&a) == CVM_OK);
    CHECK(cvm_verify(&b) == CVM_VERIFY);
    puts("VP_VERIFY_PAIR valid_INT_RET invalid_empty_RET");
    return 0;
}

static int roots(void)
{
    for (unsigned count = CVM_HEAP - 1; count <= CVM_HEAP; count++)
    {
        struct cvm_cvm vm;
        cvm_init(&vm, 0, 1000, 0);
        struct cvm_value a, b, o;
        CHECK(cvm_pair(&vm, cvm_int(11), cvm_int(12), &a) == CVM_OK);
        CHECK(cvm_pair(&vm, cvm_int(21), cvm_int(22), &b) == CVM_OK);
        for (unsigned i = 2; i < count; i++)
            CHECK(cvm_pair(&vm, cvm_int(0), cvm_int(0), &o) == CVM_OK);
        CHECK(cvm_pair(&vm, a, b, &o) == CVM_OK);
        struct cvm_value x, y;
        CHECK(cvm_field(&vm, a, 0, &x) == CVM_OK && x.integer == 11);
        CHECK(cvm_field(&vm, b, 0, &y) == CVM_OK && y.integer == 21);
        CHECK(vm.collections == (count == CVM_HEAP));
    }
    puts("VP_ROOT_PAIR capacity15_no_collection capacity16_two_temporaries");
    return 0;
}

static int budget(void)
{
    const uint8_t b[] = {OP_INT, 7, 0, 0, 0, OP_RET};
    struct cvm_program x = p(b, sizeof b);
    for (unsigned fuel = 1; fuel <= 2; fuel++)
    {
        struct cvm_cvm vm;
        cvm_init(&vm, fuel, 0, 0);
        struct cvm_value out = cvm_int(91);
        enum cvm_status status = cvm_run(&vm, &x, &out);
        CHECK(status == (fuel == 1 ? CVM_FUEL : CVM_OK));
        CHECK(out.integer == (fuel == 1 ? 91 : 7) && vm.fuel == 0);
    }
    puts("VP_FUEL_PAIR one_refuses_two_completes");
    return 0;
}

struct vg_slot
{
    unsigned epoch, live;
    int32_t synthetic;
};

struct vg_loan
{
    struct vg_slot *slot;
    unsigned epoch;
    int checked;
};

static int callback(void *context, uint8_t id, int32_t input, int32_t *out)
{
    struct vg_loan *l = context;
    if (id != 1 || input != 0)
        return -1;
    if (l->checked && (!l->slot->live || l->slot->epoch != l->epoch))
        return -1;
    *out = l->slot->synthetic;
    return 0;
}

static int lifetime(void)
{
    const uint8_t b[] = {OP_INT, 0, 0, 0, 0, OP_HOST, 1, OP_RET};
    struct cvm_program x = p(b, sizeof b);
    CHECK(cvm_verify(&x) == CVM_OK);
    struct vg_slot slot = {1, 1, 42};
    struct vg_loan loan = {&slot, 1, 1};
    struct cvm_cvm vm;
    struct cvm_value out;
    cvm_init(&vm, 3, 0, 1);
    vm.host = callback;
    vm.host_context = &loan;
    CHECK(cvm_run(&vm, &x, &out) == CVM_OK && out.integer == 42);
    /* Keep the C object alive; its logical owner lifetime ended and slot reused.
     */
    slot.live = 0;
    slot.epoch = 2;
    slot.synthetic = 77;
    slot.live = 1;
    cvm_init(&vm, 3, 0, 1);
    vm.host = callback;
    vm.host_context = &loan;
    out = cvm_int(91);
    CHECK(cvm_run(&vm, &x, &out) == CVM_HOST_ERROR && out.integer == 91);
    loan.checked = 0;
    cvm_init(&vm, 3, 0, 1);
    vm.host = callback;
    vm.host_context = &loan;
    CHECK(cvm_run(&vm, &x, &out) == CVM_OK && out.integer == 77);
    puts("VP_LIFETIME accepted_image_stale_loan_wrong_owner77 "
         "repaired_HOST_ERROR");
    return 0;
}

int main(void)
{
    if (verifier() || roots() || budget() || lifetime())
        return 1;
    puts("VP_DIAG_PASS three_minimal_pairs_host_lifetime_boundary");
    return 0;
}
